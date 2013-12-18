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

#include <scopes/internal/zmq_middleware/ObjectAdapter.h>

#include <scopes/internal/zmq_middleware/ServantBase.h>
#include <scopes/internal/zmq_middleware/ZmqException.h>
#include <scopes/internal/zmq_middleware/ZmqReceiver.h>
#include <scopes/internal/zmq_middleware/ZmqSender.h>
#include <scopes/ScopeExceptions.h>
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

ObjectAdapter::ObjectAdapter(ZmqMiddleware& mw, string const& name, string const& endpoint, RequestType t, int pool_size) :
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

ZmqMiddleware* ObjectAdapter::mw() const
{
    return &mw_;
}

string ObjectAdapter::name() const
{
    return name_;
}

string ObjectAdapter::endpoint() const
{
    return endpoint_;
}

ZmqProxy ObjectAdapter::add(std::string const& id, std::shared_ptr<ServantBase> const& obj)
{
    if (id.empty())
    {
        throw InvalidArgumentException("ObjectAdapter::add(): invalid empty id (adapter: " + name_ + ")");
    }
    if (!obj)
    {
        throw InvalidArgumentException("ObjectAdapter::add(): invalid nullptr object (adapter: " + name_ + ")");
    }

    lock_guard<mutex> lock(map_mutex_);
    auto pair = servants_.insert(make_pair(id, obj));
    if (!pair.second)
    {
        ostringstream s;
        s << "ObjectAdapter::add(): " << "cannot add id \"" << id << "\": id already used (adapter: " << name_  << ")";
        throw MiddlewareException(s.str());
    }
    return ZmqProxy(new ZmqObjectProxy(&mw_, endpoint_, id, type_));
}

void ObjectAdapter::remove(std::string const& id)
{
    shared_ptr<ServantBase> servant;
    {
        lock_guard<mutex> lock(map_mutex_);
        auto it = servants_.find(id);
        if (it == servants_.end())
        {
            ostringstream s;
            s << "ObjectAdapter::remove(): " << "cannot remove id \"" << id << "\": id not present (adapter: " << name_ << ")";
            throw MiddlewareException(s.str());
        }
        servant = it->second;
        servants_.erase(it);
    }
    // Servant goes out of scope here, outside the synchronization. This prevents deadlock if the servant
    // destructor tries to manipulate the servant map.
}

shared_ptr<ServantBase> ObjectAdapter::find(std::string const& id) const noexcept
{
    lock_guard<mutex> lock(map_mutex_);
    auto it = servants_.find(id);
    if (it != servants_.end())
    {
        return it->second;
    }
    return shared_ptr<ServantBase>();
}

void ObjectAdapter::activate() noexcept
{
    unique_lock<mutex> lock(state_mutex_);
    switch (state_)
    {
        case Active:
        {
            break;  // Already active, or about to become active, no-op
        }
        case ShuttingDown:
        {
            // Wait until shutdown in progress has completed before re-activating.
            // Coverage excluded here because the window for which we are in this state is too
            // small to hit with a test.
            state_changed_.wait(lock, [this] { return state_ == Inactive; }); // LCOV_EXCL_LINE
            // FALLTHROUGH
        }
        case Inactive:
        {
            run_workers();
            state_ = Active;
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
    unique_lock<mutex> lock(state_mutex_);
    switch (state_)
    {
        case Inactive:
        case ShuttingDown:
        {
            break;  // Already shut down, or about to shut down, no-op
        }
        case Active:
        {
            state_ = ShuttingDown;      // No notify_all() for this because no-one waits for that state.
            try
            {
                ctrl_->send("stop");    // Sending anything will cause the broker and workers to stop.
            }
            catch (zmqpp::exception)    // Ignore errors from destroyed context
            {
            }
            lock.unlock();              // Unlock here because workers may still be processing requests.
            for (auto& w : workers_)    // Join with all the workers.
            {
                w.join();
            }
            broker_.join();
            try
            {
                ctrl_->close();
            }
            catch (...)                 // Ignore errors from destroyed context
            {
            }
            workers_.clear();
            {
                lock_guard<mutex> map_lock(map_mutex_);
                ServantMap().swap(servants_);   // No need for a try block. The ServantBase destructor is noexcept.
            }
            lock.lock();
            state_ = Inactive;
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
    unique_lock<mutex> lock(state_mutex_);
    state_changed_.wait(lock, [this]{ return state_ == Inactive; });
}

void ObjectAdapter::run_workers()
{
    // PUB socket to let the broker and workers know when it is time to shut down.
    ctrl_.reset(new zmqpp::socket(*mw_.context(), zmqpp::socket_type::publish));
    ctrl_->set(zmqpp::socket_option::linger, 0);
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
            // TODO: Throwing here causes a segfault in _Unwind_Resume().
            //       This appears to be a glibc problem during clean-up?
            //       Reported here: https://bugs.launchpad.net/unity-scopes-api/+bug/1252870
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
        auto f = ready_.get_future();
        num_workers_.store(pool_size_);
        for (auto i = 0; i < pool_size_; ++i)
        {
            workers_.push_back(thread(&ObjectAdapter::worker_thread, this));
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
        zmqpp::socket ctrl(*mw_.context(), zmqpp::socket_type::subscribe);
        ctrl.set(zmqpp::socket_option::linger, 0);
        zmqpp::poller poller;
        try
        {
            ctrl.connect("inproc://" + name_ + "_adapter_ctrl"); // Once we can read from here, that's the command to stop.
            ctrl.subscribe("");
            poller.add(ctrl);

            // Set up message pump. Router-dealer for twoway adapter, pull-push for oneway oneway adapter.

            assert(!frontend_);
            auto socket_type = type_ == RequestType::Twoway ? zmqpp::socket_type::router : zmqpp::socket_type::pull;
            frontend_.reset(new zmqpp::socket(*mw_.context(), socket_type));
            frontend_->set(zmqpp::socket_option::linger, 0);
            frontend_->bind(endpoint_);
            poller.add(*frontend_);

            assert(!backend_);
            socket_type = type_ == RequestType::Twoway ? zmqpp::socket_type::dealer : zmqpp::socket_type::push;
            backend_.reset(new zmqpp::socket(*mw_.context(), socket_type));
            backend_->set(zmqpp::socket_option::linger, 0);
            backend_->bind("inproc://" + name_ + "-worker");
            poller.add(*backend_);

            // Tell parent that we are ready
            {
                lock_guard<mutex> lock(ready_mutex_);
                ready_.set_value();
            }
        }
        catch (...) // LCOV_EXCL_LINE
        {
            // TODO: returning here, which happens, for example, if the directory for an endpoint does
            //       not exist causes a crash in run_workers when the exception is caught and re-thrown;
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
                // When the ctrl socket becomes ready, we need to get out of here.
                // We stop reading more requests from the router, but continue processing
                // while there are still replies outstanding.
                ctrl.receive(message);
                ctrl.close();
                shutting_down = true;
            }
            if (!shutting_down && poller.has_input(*frontend_)) // Once shutting down, we no longer read incoming messages.
            {
                int flag;
                do
                {
                    // This is the actual message pump. We read an incoming request and pass it to one of the workers
                    // for processing. The dealer socket ensures fair sharing.
                    frontend_->receive(message);
                    flag = frontend_->has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    backend_->send(message, flag);
                }
                while (flag == zmqpp::socket::send_more);
                if (type_ == RequestType::Twoway)
                {
                    ++pending_requests; // Only twoway requests require us to wait for replies
                }
            }
            if (pending_requests != 0 && poller.has_input(*backend_))
            {
                // We need to read a reply for an earlier request.
                int flag;
                do
                {
                    // Message pump in the opposite direction, for replies from server to client.
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
                return;
            }
        }
    }
    catch (std::exception const& e)
    {
        // TODO: log this
        cerr << "OA broker thread exception: " << e.what() << endl;
        return;
    }
    catch (...)
    {
        // TODO: log this
        cerr << "OA broker unknown exception" << endl;
        throw;
    }
}

void ObjectAdapter::worker_thread()
{
    try
    {
        zmqpp::poller poller;

        zmqpp::socket ctrl(*mw_.context(), zmqpp::socket_type::subscribe); // ctrl becomes ready for reading when we are told to stop.
        ctrl.set(zmqpp::socket_option::linger, 0);
        ctrl.connect("inproc://" + name_ + "_adapter_ctrl");
        ctrl.subscribe("");
        poller.add(ctrl);

        auto socket_type = type_ == RequestType::Twoway ? zmqpp::socket_type::reply : zmqpp::socket_type::pull;
        zmqpp::socket s(*mw_.context(), socket_type);
        s.set(zmqpp::socket_option::linger, 0);
        s.connect("inproc://" + name_ + "-worker");
        poller.add(s);

        ZmqReceiver receiver(s);
        ZmqSender sender(s);        // Sender is used only by twoway adapters (zero-cost, the class just remembers the passed socket).

        if (--num_workers_ == 0)    // Last worker to reach this point notifies the parent that all workers are ready.
        {
            lock_guard<mutex> lock(ready_mutex_);
            ready_.set_value();
        }

        struct Current current;
        current.adapter = this;     // Stays the same for all invocations, so we set this once only.

        bool finish = false;
        for (;;)
        {
            if (finish)
            {
                s.close();
                return;
            }

            poller.poll();
            if (poller.has_input(ctrl)) // Parent sent a stop message, so we are supposed to go away.
            {
                zmqpp::message message;
                ctrl.receive(message);
                ctrl.close();
                finish = true;
            }

            if (!finish && poller.has_input(s)) // We stop reading new incoming messages once told to finish.
            {
                // Unmarshal the type-independent part of the message (id, operation name, mode).
                unique_ptr<capnp::SegmentArrayMessageReader> message;
                capnproto::Request::Reader req;
                try
                {
                    // Unmarshal generic part of the message.
                    auto segments = receiver.receive();
                    message.reset(new capnp::SegmentArrayMessageReader(segments));
                    req = message->getRoot<capnproto::Request>();
                    current.id = req.getId().cStr();
                    current.op_name = req.getOpName().cStr();
                    if (current.id.empty() || current.op_name.empty() || !req.hasMode())
                    {
                        if (type_ == RequestType::Twoway)
                        {
                            capnp::MallocMessageBuilder b;
                            auto exr = create_unknown_response(b, "Invalid message header");
                            sender.send(exr);
                        }
                        else
                        {
                            // TODO: log error for invalid oneway request header
                        }
                        continue;
                    }
                    auto mode = req.getMode();
                    auto expected_mode = type_ == RequestType::Twoway ? capnproto::RequestMode::TWOWAY : capnproto::RequestMode::ONEWAY;
                    if (mode != expected_mode) // Can't do oneway on a twoway adapter and vice-versa.
                    {
                        if (type_ == RequestType::Twoway)
                        {
                            ostringstream s;
                            s << "ObjectAdapter: oneway invocation sent to twoway adapter "
                              << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << ")";
                            capnp::MallocMessageBuilder b;
                            auto exr = create_unknown_response(b, s.str());
                            sender.send(exr);
                        }
                        else
                        {
                            // TODO: log error
                        }
                        continue;
                    }
                }
                catch (std::exception const& e)
                {
                    // We get here if receive() throws. It will do that if the isn't an integral number
                    // of words in the received message.
                    if (type_ == RequestType::Twoway)
                    {
                        capnp::MallocMessageBuilder b;
                        ostringstream s;
                        s << "ObjectAdapter: error unmarshaling request header "
                          << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << "): " << e.what();
                        auto exr = create_unknown_response(b, s.str());
                        sender.send(exr);
                    }
                    else
                    {
                        // TODO: log error
                    }
                    continue;
                }

                // Look for a servant with matching id.
                auto servant = find(current.id);
                if (!servant)
                {
                    if (type_ == RequestType::Twoway)
                    {
                        capnp::MallocMessageBuilder b;
                        auto exr = create_object_not_exist_response(b, current.id, endpoint_, name_);
                        sender.send(exr);
                    }
                    continue;
                }

                // We have a target object, so we can ask it to unmarshal the in-params, forward
                // the invocation to the application-provided method, and to marshal the results.
                auto in_params = req.getInParams();
                capnp::MallocMessageBuilder b;
                auto r = b.initRoot<capnproto::Response>();
                servant->safe_dispatch_(current, in_params, r); // noexcept
                if (type_ == RequestType::Twoway)
                {
                    sender.send(b.getSegmentsForOutput());
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        // TODO: log this
        cerr << "OA worker thread exception: " << e.what() << endl;
        return;
    }
    catch (...)
    {
        // TODO: log this
        cerr << "OA worker unknown exception" << endl;
        throw;
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
