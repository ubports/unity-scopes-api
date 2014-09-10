/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/Reaper.h>

#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>

using namespace std;
using namespace unity::scopes::internal::reaper_private;

namespace unity
{

namespace scopes
{

namespace internal
{

ReapItem::ReapItem(weak_ptr<Reaper> const& reaper, reaper_private::Reaplist::iterator it) :
    reaper_(reaper),
    it_(it),
    cancelled_(false)
{
}

ReapItem::~ReapItem()
{
    // If we go out of scope, we remove ourselves from the reaper list. This
    // ensures that no callback will be made.
    cancel();
}

void ReapItem::refresh() noexcept
{
    auto const reaper = reaper_.lock();  // Reaper may no longer be around
    if (reaper)
    {
        std::lock(mutex_, reaper->mutex_);
        lock_guard<mutex> reaper_lock(reaper->mutex_, adopt_lock);
        lock_guard<mutex> item_lock(mutex_, adopt_lock);

        if (cancelled_)
        {
            return;  // A reaping pass may have cancelled this ReapItem already.
        }

        // Move our Item to the head of the list after updating the time stamp.
        assert(it_ != reaper->list_.end());
        reaper_private::Item item(*it_);
        item.timestamp = chrono::steady_clock::now();
        reaper->list_.erase(it_);
        reaper->list_.push_front(item);
        it_ = reaper->list_.begin();
    }
    else
    {
        // The reaper has gone away, so we disable ourself.
        lock_guard<mutex> item_lock(mutex_);
        cancelled_ = true;
    }
}

void ReapItem::cancel() noexcept
{
    auto const reaper = reaper_.lock();  // Reaper may no longer be around
    if (reaper)
    {
        {
            // Wait for a concurrent reaping pass to complete only
            // if cancel() is not called by the reaper. This ensures
            // that cancel() does not return until after the callback
            // function for this reap item has completed, and avoids
            // deadlock if the callback function tries to cancel its
            // own reap item.
            unique_lock<mutex> reap_lock(reaper->reap_mutex_);
            if (reaper->reap_thread_id_ != this_thread::get_id())
            {
                reaper->reap_done_.wait(reap_lock, [&reaper]{ return !reaper->reap_in_progress_; });
            }

            lock_guard<mutex> item_lock(mutex_);
            if (cancelled_)
            {
                return;  // A reaping pass may have cancelled this ReapItem already.
            }
            cancelled_ = true;
        }

        // Remove our Item from the reaper's list.
        lock_guard<mutex> lock(reaper->mutex_);
        assert(it_ != reaper->list_.end());
        reaper->list_.erase(it_);
        it_ = reaper->list_.end();
    }
    else
    {
        // The reaper has gone away, so we disable ourself.
        lock_guard<mutex> item_lock(mutex_);
        cancelled_ = true;
    }
}

Reaper::Reaper(int reap_interval, int expiry_interval, DestroyPolicy p) :
    reap_interval_(chrono::seconds(reap_interval)),
    expiry_interval_(chrono::seconds(expiry_interval)),
    policy_(p),
    finish_(false),
    reap_in_progress_(false)
{
    if (reap_interval != -1 && expiry_interval != -1)
    {
        if (reap_interval < 1)
        {
            ostringstream s;
            s << "Reaper: invalid reap_interval (" << reap_interval << "). Interval must be > 0.";
            throw unity::InvalidArgumentException(s.str());
        }
        if (reap_interval > expiry_interval)
        {
            ostringstream s;
            s << "Reaper: reap_interval (" << reap_interval << ") must be <= expiry_interval (" << expiry_interval << ").";
            throw unity::LogicException(s.str());
        }
    }
}

Reaper::~Reaper()
{
    destroy();  // noexcept
}

// Instantiate a new reaper. We call set_self() after instantiation so the reaper
// can keep a weak_ptr to itself. That weak_ptr in turn is passed to each ReapItem,
// so the ReapItem can manipulate the reap list. If the reaper goes out of scope
// before a ReapItem, the ReapItem will notice this and disable itself.

Reaper::SPtr Reaper::create(int reap_interval, int expiry_interval, DestroyPolicy p)
{
    SPtr reaper(new Reaper(reap_interval, expiry_interval, p));
    reaper->set_self();
    if (reap_interval != -1 && expiry_interval != -1)
    {
        reaper->start();
    }
    return reaper;
}

void Reaper::set_self() noexcept
{
    self_ = shared_from_this();
}

void Reaper::start()
{
    reap_thread_ = thread(&Reaper::reap_func, this);
    lock_guard<mutex> reap_lock(reap_mutex_);
    reap_thread_id_ = reap_thread_.get_id();
}

void Reaper::destroy()
{
    // Let the reaper thread know that it needs to stop doing things
    {
        lock_guard<mutex> lock(mutex_);
        if (finish_)
        {
            return;
        }
        // If the reaper thread was never started, but there
        // are entries to be reaped, start the thread, so it
        // will invoke the callbacks for any remaining entries.
        if (reap_interval_.count() == -1 && list_.size() != 0 && policy_ == CallbackOnDestroy)
        {
            start();
        }
        finish_ = true;
        do_work_.notify_one();
    }
    if (reap_thread_.joinable())
    {
        reap_thread_.join();
    }
}

// Add a new entry to the reaper. If the entry is not refreshed within the expiry interval,
// the reaper removes the item from the list and calls cb to let the caller know about the expiry.

ReapItem::SPtr Reaper::add(ReaperCallback const& cb)
{
    if (!cb)
    {
        throw unity::InvalidArgumentException("Reaper: invalid null callback passed to add().");
    }

    unique_lock<mutex> lock(mutex_);

    if (finish_)
    {
        throw unity::LogicException("Reaper: cannot add item to destroyed reaper.");
    }

    // Put new Item at the head of the list.
    reaper_private::Reaplist::iterator li;
    Item item(cb);
    list_.push_front(item); // LRU order
    li = list_.begin();
    if (list_.size() == 1)
    {
        do_work_.notify_one();  // Wake up reaper thread
    }

    // Make a new ReapItem.
    assert(self_.lock());
    ReapItem::SPtr reap_item(new ReapItem(self_, li));
    // Now that the ReapItem is created, we can set the back-pointer.
    li->reap_item = reap_item;
    return reap_item;
}

size_t Reaper::size() const noexcept
{
    lock_guard<mutex> lock(mutex_);
    return list_.size();
}

// Reaper thread

void Reaper::reap_func()
{
    unique_lock<mutex> lock(mutex_);
    for (;;)
    {
        if (list_.empty())
        {
            // If no items are in the list, we wait until there is at least one item
            // in the list or we are told to finish. (While there is nothing
            // to reap, there is no point in waking up periodically only to find the list empty.)
            do_work_.wait(lock, [this] { return !list_.empty() || finish_; });
        }
        else
        {
            // There is at least one item on the list, we wait with a timeout.
            // The first-to-expire item is at the tail of the list. We sleep at least long enough
            // for that item to get a chance to expire. (There is no point in waking up earlier.)
            // But, if we have just done a scan, we sleep for at least reap_interval_, so there is
            // at most one pass every reap_interval_.
            auto const now = chrono::steady_clock::now();
            auto const oldest_item_age = chrono::duration_cast<chrono::milliseconds>(now - list_.back().timestamp);
            auto const reap_interval = chrono::duration_cast<chrono::milliseconds>(reap_interval_);
            auto const sleep_interval = max(expiry_interval_ - oldest_item_age, reap_interval);
            do_work_.wait_for(lock, sleep_interval, [this]{ return finish_; });
        }

        if (finish_ && policy_ == NoCallbackOnDestroy)
        {
            // If we are told to finish (which happens when the Reaper instance is destroyed),
            // if NoCallbackOnDestroy is set, we are done.
            return;
        }

        // We run along the list from the tail towards the head. For any entry on the list
        // that is too old, we copy it to a zombie list. We use a strictly less comparison for
        // the timestamp, so if the entry is exactly expiry_interval_ old, it will be reaped.
        reaper_private::Reaplist zombies;
        if (finish_ && policy_ == CallbackOnDestroy)
        {
            // Final pass for CallbackOnDestroy. We simply call back on everything.
            zombies.assign(list_.begin(), list_.end());
        }
        else if (reap_interval_.count() != -1)  // Look only if we have non-infinite expiry time.
        {
            // Find any entries that have expired.
            for (auto it = list_.rbegin(); it != list_.rend(); ++it)
            {
                if (chrono::steady_clock::now() < it->timestamp + expiry_interval_)
                {
                    break;  // LRU order. Once we find an entry that's not expired, we can stop looking.
                }
                zombies.push_back(*it);
            }
        }

        // Callbacks are made outside the synchronization, so we can't deadlock if a
        // a callback invokes a method on the reaper or a ReapItem.
        lock.unlock();
        remove_zombies(zombies);    // noexcept
        lock.lock();

        if (finish_)
        {
            return;
        }
    }
}

void Reaper::remove_zombies(reaper_private::Reaplist const& zombies) noexcept
{
    // reap_in_progress prevents ReapItem::cancel() from returning
    // before its callback has completed.
    {
        lock_guard<mutex> reap_lock(reap_mutex_);
        reap_in_progress_ = true;
    }

    for (auto& item : zombies)
    {
        auto ri = item.reap_item.lock();
        if (!ri)
        {
            // ReapItem was deallocated after this reaping pass started,
            // but before we got around to dealing with this ReapItem.
            continue;
        }

        {
            lock_guard<mutex> item_lock(ri->mutex_);
            if (ri->cancelled_)
            {
                // ReapItem::cancel() was called during this pass, it has already completed cancellation.
                continue;
            }
            ri->cancelled_ = true;
        }

        {
            lock_guard<mutex> lock(mutex_);
            assert(ri->it_ != list_.end());
            list_.erase(ri->it_);
            ri->it_ = list_.end();
        }

        try
        {
            assert(item.cb);
            item.cb();                      // Informs the caller that the item timed out.
        }
        catch (...)
        {
            // Ignore exceptions raised by the application's callback function.
        }
    }

    {
        lock_guard<mutex> reap_lock(reap_mutex_);
        reap_in_progress_ = false;
        reap_done_.notify_all();
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
