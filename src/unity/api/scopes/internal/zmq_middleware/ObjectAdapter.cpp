/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/zmq_middleware/ObjectAdapter.h>

#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>

#include <cassert>
#include <sstream>
#include <iostream> // TODO: remove this

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

ObjectAdapter::ObjectAdapter(ZmqMiddleware& mw, string const& name, string const& endpoint, int pool_size, Type t) :
    mw_(mw),
    name_(name),
    endpoint_(endpoint),
    type_(t),
    pool_size_(pool_size),
    state_(Inactive)
{
    assert(!name.empty());
    assert(!endpoint.empty());
    assert(pool_size >= 1);
}

ObjectAdapter::~ObjectAdapter() noexcept
{
    // No catch handler here because these are noexcept
    shutdown();
    wait_for_shutdown();
}

string ObjectAdapter::name() const
{
    return name_;
}

string ObjectAdapter::endpoint() const
{
    return endpoint_;
}

ZmqProxy ObjectAdapter::add(std::string const& id, std::shared_ptr<AbstractObject> const& obj)
{
    if (id.empty())
    {
        throw InvalidArgumentException("ObjectAdapter::add(): invalid empty id (adapter: " + name_ + ")");
    }
    if (!obj)
    {
        throw InvalidArgumentException("ObjectAdapter::add(): invalid nullptr object (adapter: " + name_ + ")");
    }

    lock_guard<mutex> lock(mutex_);
    auto pair = servants_.insert(make_pair(id, obj));
    if (!pair.second)
    {
        ostringstream s;
        s << "ObjectAdapter::add(): " << "cannot add id \"" << id << "\": id already used (adapter: " << name_  << ")";
        throw MiddlewareException(s.str());
    }
}

void ObjectAdapter::remove(std::string const& id)
{
    lock_guard<mutex> lock(mutex_);
    auto it = servants_.find(id);
    if (it == servants_.end())
    {
        ostringstream s;
        s << "ObjectAdapter::remove(): " << "cannot remove id \"" << id << "\": id not present (adapter: "
          << name_ << ")";
        throw MiddlewareException(s.str());
    }
    servants_.erase(it);
}

shared_ptr<AbstractObject> ObjectAdapter::find(std::string const& id) const
{
    lock_guard<mutex> lock(mutex_);
    auto it = servants_.find(id);
    if (it == servants_.end())
    {
        ostringstream s;
        s << "ObjectAdapter::find(): " << "cannot find id \"" << id << "\": id not present (adapter: "
          << name_ << ")";
        throw MiddlewareException(s.str());
    }
    return it->second;
}

void ObjectAdapter::activate() noexcept
{
    unique_lock<mutex> lock(mutex_);
    switch (state_)
    {
        case Active:
        case Activating:
        {
            break;  // Already active, or about to become active, no-op
        }
        case ShuttingDown:
        {
            // Wait until shutdown in progress has completed before re-activating.
            // Coverage excluded here because the window in which we are in this state is too
            // small to hit with a test.
            state_changed_.wait(lock, [this]{ return state_ == Inactive; }); // LCOV_EXCL_LINE
            // FALLTHROUGH
        }
        case Inactive:
        {
            state_ = Activating;            // No notify_all() here because no-one waits for this
            run_workers();
            state_ = Active;
            lock.unlock();
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ObjectAdapter::shutdown() noexcept
{
    unique_lock<mutex> lock(mutex_);
    switch (state_)
    {
        case Inactive:
        case ShuttingDown:
        {
            break;  // Already shut down, or about to shut down, no-op
        }
        case Activating:
        {
            // Wait until activation in progress has completed before shutting down.
            // Coverage excluded here because the window in which we are in this state is too
            // small to hit with a test.
            state_changed_.wait(lock, [this]{ return state_ == Active; }); // LCOV_EXCL_LINE
            // FALLTHROUGH
        }
        case Active:
        {
            state_ = ShuttingDown;      // No notify_all() for this because no-one waits for that state.
            ctrl_->send("stop");        // Sending anything will cause the broker and workers to stop.
            for (auto& w : workers_)    // Join with all the workers.
            {
                w.join();
            }
            broker_.join();
            workers_.clear();
            //backend_.reset(nullptr);

            // We clear the servant map outside the synchronization, so it is not possible for the destructor
            // of a servant to cause deadlock by calling back into this adapter.
            lock.unlock();
            ServantMap().swap(servants_);   // Not need for a try block. The AbstractObject destructor is noexcept.
            lock.lock();
            state_ = Inactive;
            lock.unlock();
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ObjectAdapter::wait_for_shutdown() noexcept
{
    unique_lock<mutex> lock(mutex_);
    state_changed_.wait(lock, [this]{ return state_ == Inactive; });
}

void ObjectAdapter::run_workers()
{
    // PUB socket to let the broker and workers know when it is time to shut down.
    ctrl_.reset(new zmqpp::socket(mw_.context(), zmqpp::socket_type::publish));
    ctrl_->bind("inproc://" + name_ + "_adapter_ctrl");

    // Start a broker thread to forward incoming messages to backend workers and
    // wait for the broker thread to finish its initialization. The broker
    // signals after it has connected to the ctrl socket.
    {
        lock_guard<mutex> lock(ready_mutex_);
        ready_ = promise<void>();
    }
    broker_ = thread(&ObjectAdapter::broker_thread, this);
    {
        auto f = ready_.get_future();
        f.wait();
        try
        {
            f.get();
        }
        catch (...) // LCOV_EXCL_LINE
        {
            throw MiddlewareException("ObjectAdapter::run_workers(): broker thread failure (adapter: " + name_ + ")"); // LCOV_EXCL_LINE
        }
    }

    // Create the appropriate number of worker threads.
    // The last worker thread to subscribe signals that the workers are ready.
    // This ensures that we won't send a stop from deactivate() until after
    // the workers can actually receive it.
    {
        lock_guard<mutex> lock(ready_mutex_);
        ready_ = promise<void>();
    }
    {
        auto start_func = type_ == Type::Oneway ? &ObjectAdapter::oneway_thread : &ObjectAdapter::twoway_thread;
        auto f = ready_.get_future();
        num_workers_.store(pool_size_);
        for (auto i = 0; i < pool_size_; ++i)
        {
            workers_.push_back(thread(start_func, this));
        }
        f.wait();
        try
        {
            f.get();
        }
        catch (...) // LCOV_EXCL_LINE
        {
            throw MiddlewareException("ObjectAdapter::run_workers(): worker thread failure (adapter: " + name_ + ")"); // LCOV_EXCL_LINE
        }
    }
}

void ObjectAdapter::broker_thread()
{
    try
    {
        zmqpp::socket ctrl(mw_.context(), zmqpp::socket_type::subscribe);
        zmqpp::poller poller;
        try
        {
            ctrl.connect("inproc://" + name_ + "_adapter_ctrl");
            ctrl.subscribe("");

            assert(!frontend_);
            frontend_.reset(new zmqpp::socket(mw_.context(), zmqpp::socket_type::router));
            frontend_->bind(endpoint_);

            assert(!backend_);
            backend_.reset(new zmqpp::socket(mw_.context(), zmqpp::socket_type::dealer));
            backend_->bind("inproc://" + name_);

            poller.add(ctrl);
            poller.add(*frontend_);
            poller.add(*backend_);

            {
                lock_guard<mutex> lock(ready_mutex_);
                ready_.set_value(); // Tell parent that we are ready
            }
        }
        catch (...) // LCOV_EXCL_LINE
        {
            // TODO: log error
            lock_guard<mutex> lock(ready_mutex_);       // LCOV_EXCL_LINE
            ready_.set_exception(current_exception());  // LCOV_EXCL_LINE
            return;                                     // LCOV_EXCL_LINE
        }

        int pending_requests = 0;
        bool shutting_down = false;
        for (;;)
        {
            zmqpp::message message;
            poller.poll();
            if (poller.has_input(ctrl))
            {
                // When the ctrl socket becomes ready, wee need to get out of here.
                // We stop reading more requests from the router, but continue processing
                // while there are still replies outstanding.
                shutting_down = true;
            }
            if (!shutting_down && poller.has_input(*frontend_))
            {
                int flag;
                do
                {
                    cerr << "broker: calling frontend receive" << endl;
                    frontend_->receive(message);
                    cerr << "broker: frontend receive returned" << endl;
                    flag = frontend_->has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    backend_->send(message, flag);
                }
                while (flag == zmqpp::socket::send_more);
                ++pending_requests;
            }
            if (poller.has_input(*backend_))
            {
                int flag;
                do
                {
                    backend_->receive(message);
                    flag = backend_->has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    frontend_->send(message, flag);
                }
                while (flag == zmqpp::socket::send_more);
                assert(pending_requests > 0);
                --pending_requests;
            }
            if (shutting_down && pending_requests == 0)
            {
                frontend_->close();
                backend_->close();
                ctrl.close();
                return;
            }
        }
    }
    catch (...) // LCOV_EXCL_LINE
    {
        lock_guard<mutex> lock(ready_mutex_);       // LCOV_EXCL_LINE
        ready_.set_exception(current_exception());  // LCOV_EXCL_LINE
    }
}

void ObjectAdapter::oneway_thread()
{
    if (--num_workers_ == 0)
    {
        lock_guard<mutex> lock(ready_mutex_);
        ready_.set_value();
    }

#if 0
    zmqpp::socket s(mw_.context(), zmqpp::socket_type::dealer);
    s.connect("inproc://" + name_);
    try
    {
    }
    catch (...)
    {
        // Sockets are not thread-safe, so we need to close here rather than
        // relying on the destructor. (If we do that, the destructor will be
        // invoked by the calling thread instead of this thread).
        // http://zeromq.org/whitepapers:0mq-termination
        s.close();
    }
#endif
}

void ObjectAdapter::twoway_thread()
{
    try
    {
        zmqpp::socket ctrl(mw_.context(), zmqpp::socket_type::subscribe);
        ctrl.connect("inproc://" + name_ + "_adapter_ctrl");
        ctrl.subscribe("");

        zmqpp::socket s(mw_.context(), zmqpp::socket_type::request);
        s.connect("inproc://" + name_);

        zmqpp::poller poller;
        poller.add(ctrl);
        poller.add(s);

        if (--num_workers_ == 0)
        {
            {
                lock_guard<mutex> lock(ready_mutex_);
                ready_.set_value();
            }
        }

        for (;;)
        {
            cerr << "worker: waiting for input" << endl;
            poller.poll();
            if (poller.has_input(ctrl))
            {
                ctrl.close();
                s.close();
                return;
            }
            if (poller.has_input(s))
            {
                zmqpp::message request;
                cerr << "worker: calling receive" << endl;
                s.receive(request);

                // wake up follower

                string payload;
                request >> payload;
                cerr << "received message: " << payload << endl;
                zmqpp::message response;
                response << "response";
                s.send(response);
                cerr << "sent response" << endl;
            }
        }
    }
    catch (...) // LCOV_EXCL_LINE
    {
        lock_guard<mutex> lock(ready_mutex_);       // LCOV_EXCL_LINE
        ready_.set_exception(current_exception());  // LCOV_EXCL_LINE
    }
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
