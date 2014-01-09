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
#include <unity/util/ResourcePtr.h>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>

#include <cassert>
#include <sstream>
#include <iostream>  // TODO: remove this once logging is added

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

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
    try
    {
        shutdown();
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "~ObjectAdapter(): exception from shutdown(): " << e.what() << endl;
    }
    // LCOV_EXCL_START
    catch (...)
    {
        // TODO: log error
        cerr << "~ObjectAdapter(): unknown exception from shutdown()" << endl;
    }
    // LCOV_EXCL_STOP

    // We need to make sure that wait_for_shutdown() is always called because it joins
    // with any threads that may still be running.
    try
    {
        wait_for_shutdown();
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "~ObjectAdapter(): exception from wait_for_shutdown(): " << e.what() << endl;
    }
    // LCOV_EXCL_START
    catch (...)
    {
        // TODO: log error
        cerr << "~ObjectAdapter(): unknown exception from wait_for_shutdown()" << endl;
    }
    // LCOV_EXCL_STOP
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

    lock(map_mutex_, state_mutex_);
    lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
    {
        lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
        if (state_ == Destroyed || state_ == Failed)
        {
            throw_bad_state(state_);
        }
    }

    auto pair = servants_.insert(make_pair(id, obj));
    if (!pair.second)
    {
        ostringstream s;
        s << "ObjectAdapter::add(): " << "cannot add id \"" << id << "\": id already in use (adapter: " << name_  << ")";
        throw MiddlewareException(s.str());
    }
    return ZmqProxy(new ZmqObjectProxy(&mw_, endpoint_, id, type_));
}

void ObjectAdapter::remove(std::string const& id)
{
    shared_ptr<ServantBase> servant;
    {
        lock(map_mutex_, state_mutex_);
        lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
        {
            lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
            if (state_ == Destroyed || state_ == Failed)
            {
                throw_bad_state(state_);
            }
        }

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
    // Lock released here, so we don't call servant destructor while holding a lock

    servant = nullptr;  // This may trigger destructor call on the servant
}

shared_ptr<ServantBase> ObjectAdapter::find(std::string const& id) const
{
    lock(map_mutex_, state_mutex_);
    lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
    {
        lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
        if (state_ == Destroyed || state_ == Failed)
        {
            throw_bad_state(state_);
        }
    }

    auto it = servants_.find(id);
    if (it != servants_.end())
    {
        return it->second;
    }
    return shared_ptr<ServantBase>();
}

void ObjectAdapter::activate()
{
    unique_lock<mutex> lock(state_mutex_);
    switch (state_)
    {
        case Inactive:
        {
            state_ = Activating;  // No notify_all() here because no-one waits for this
            try
            {
                lock.unlock();
                run_workers();
                lock.lock();
            }
            catch (...)
            {
                lock.lock();
                state_ = Failed;
                state_changed_.notify_all();
                throw;
            }
            state_ = Active;
            state_changed_.notify_all();
            break;
        }
        case Activating:
        case Active:
        {
            break;  // Already active, or about to become active, no-op
        }
        case Deactivating:
        case Destroyed:
        case Failed:
        {
            throw_bad_state(state_);
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ObjectAdapter::shutdown()
{
    unique_lock<mutex> lock(state_mutex_);
    switch (state_)
    {
        case Deactivating:
        case Destroyed:
        {
            break;  // Nothing to do
        }
        case Failed:
        {
            throw_bad_state(state_);
        }
        case Inactive:
        {
            state_ = Destroyed;
            state_changed_.notify_all();
            break;
        }
        case Activating:
        {
            // LCOV_EXCL_START  Too hard to hit with a test, the window is too small.

            // Wait until activation is complete or we reach a final state
            state_changed_.wait(lock, [this]{ return state_ != Activating; });
            if (state_ == Deactivating || state_ == Destroyed)
            {
                return;  // Another thread has finished the job in the mean time.
            }
            if (state_ == Failed)
            {
                throw_bad_state(state_);
            }
            // LCOV_EXCL_STOP

            // FALLTHROUGH
        }
        case Active:
        {
            state_ = Deactivating;
            stop_workers();
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ObjectAdapter::wait_for_shutdown()
{
    AdapterState state;
    {
        unique_lock<mutex> lock(state_mutex_);
        state_changed_.wait(lock, [this]{ return state_ == Deactivating || state_ == Destroyed || state_ == Failed; });
        if (state_ == Deactivating)
        {
            state_ = Destroyed;
        }
        state_changed_.notify_all();  // Wake up everyone else asleep in here
        state = state_;
    }

    // We join with threads and call servant destructors outside synchronization.
    call_once(once_, [this](){ this->cleanup(); });

    if (state == Failed)
    {
        throw_bad_state(state_);
    }
}

void ObjectAdapter::throw_bad_state(AdapterState state) const
{
    string bad_state;
    switch (state)
    {
        // LCOV_EXCL_START Too hard to hit with a test, the window is too short.
        case Deactivating:
        {
            bad_state = "Deactivating";
            break;
        }
        // LCOV_EXCL_STOP
        case Destroyed:
        {
            bad_state = "Destroyed";
            break;
        }
        case Failed:
        {
            bad_state = "Failed";
            break;
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
            break;
        }
    }
    MiddlewareException e("Object adapter in " + bad_state + " state (adapter: " + name_ + ")");
    e.remember(exception_);  // Remember any exception encountered by the broker thread or a worker thread.
    throw e;
}

void ObjectAdapter::run_workers()
{
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
        auto f = ready_.get_future();
        num_workers_.store(pool_size_);
        for (auto i = 0; i < pool_size_; ++i)
        {
            workers_.push_back(thread(&ObjectAdapter::worker_thread, this));
        }
        try
        {
            f.get();
        }
        // LCOV_EXCL_START
        catch (...) //
        {
            stop_workers();
            throw MiddlewareException("ObjectAdapter::run_workers(): worker thread failure (adapter: " + name_ + ")");
        }
        // LCOV_EXCL_STOP
    }
}

void ObjectAdapter::init_ctrl_socket()
{
    lock_guard<mutex> lock(ctrl_mutex_);

    // PUB socket to let the broker and workers know when it is time to shut down.
    // Sending anything means the socket becomes ready for reading, which causes
    // the broker and worker threads to finish.
    ctrl_.reset(new zmqpp::socket(*mw_.context(), zmqpp::socket_type::publish));
    ctrl_->set(zmqpp::socket_option::linger, 0);
    ctrl_->bind("inproc://" + name_ + "_adapter_ctrl");
}

zmqpp::socket ObjectAdapter::subscribe_to_ctrl_socket()
{
    zmqpp::socket ctrl(*mw_.context(), zmqpp::socket_type::subscribe);
    ctrl.set(zmqpp::socket_option::linger, 0);
    ctrl.connect("inproc://" + name_ + "_adapter_ctrl"); // Once a thread can read from here, that's the command to stop.
    ctrl.subscribe("");
    return move(ctrl);
}

void ObjectAdapter::stop_workers() noexcept
{
    lock_guard<mutex> lock(ctrl_mutex_);

    try
    {
        ctrl_->send("stop");
    }
    // LCOV_EXCL_START
    catch (std::exception const& e)
    {
        // TODO: log this instead
        cerr << "ObjectAdapter::stop_workers(): " << e.what() << endl;
    }
    catch (...)
    {
        // TODO: log this instead
        cerr << "ObjectAdapter::stop_workers(): unknown exception" << endl;
    }
    // LCOV_EXCL_STOP
}

// For the ipc transport, zmq permits more than one server to bind to the same endpoint.
// If a server binds to an endpoint while another server is using that endpoint, the
// second server silently "steals" the endpoint from the previous server, so all
// connects after that point go to the new server, while connects that happened earlier
// go to the old server. This is meant as a fail-over feature, and cannot be disabled.
//
// We don't want this and need an error if two servers try to use the same endpoint.
// Hacky solution: we check whether it's possible to successfully connect to the
// endpoint. If so, a server is still running there, and we throw. This has a
// small race because a second server may connect after the check, but before
// the bind. But, in practice, that's good enough for our purposes.

void ObjectAdapter::safe_bind(zmqpp::socket& s, string const& endpoint)
{
    const std::string transport_prefix = "ipc://";
    if (endpoint.substr(0, transport_prefix.size()) == transport_prefix)
    {
        string path = endpoint.substr(transport_prefix.size());
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), path.size());
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == -1)
        {
            // LCOV_EXCL_START
            throw MiddlewareException("ObjectAdapter: broker thread failure (adapter: " + name_ + "): " +
                                      "cannot create socket: " + strerror(errno));
            // LCOV_EXCL_STOP
        }
        util::ResourcePtr<int, decltype(&::close)> close_guard(fd, ::close);
        if (::connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        {
            // Connect succeeded, so another server is using the socket already.
            throw MiddlewareException("ObjectAdapter: broker thread failure (adapter: " + name_ + "): " +
                                      "address in use: " + endpoint);
        }
    }
    s.bind(endpoint);
}

void ObjectAdapter::broker_thread()
{
    try
    {
        // Create the writing end of the ctrl socket. We need to do this before subscribing because, for inproc
        // pub-sub sockets, the bind must happen before the connect.
        init_ctrl_socket();

        // Subscribe to ctrl socket. Once this socket becomes readable, that's the command to finish.
        auto ctrl = subscribe_to_ctrl_socket();

        // Set up message pump. Router-dealer for twoway adapter, pull-push for oneway adapter.
        auto socket_type = type_ == RequestType::Twoway ? zmqpp::socket_type::router : zmqpp::socket_type::pull;
        zmqpp::socket frontend(*mw_.context(), socket_type);

        socket_type = type_ == RequestType::Twoway ? zmqpp::socket_type::dealer : zmqpp::socket_type::push;
        zmqpp::socket backend(*mw_.context(), socket_type);

        zmqpp::poller poller;

        try
        {
            poller.add(ctrl);

            frontend.set(zmqpp::socket_option::linger, 0);
            // "Safe" bind: prevents two servers from binding to the same endpoint.
            safe_bind(frontend, endpoint_);
            poller.add(frontend);

            backend.set(zmqpp::socket_option::linger, 0);
            backend.bind("inproc://" + name_ + "-worker");
            poller.add(backend);

            // Tell parent that we are ready
            {
                lock_guard<mutex> lock(ready_mutex_);
                ready_.set_value();
            }
        }
        // LCOV_EXCL_START
        catch (...) // LCOV_EXCL_LINE
        {
            // TODO: log error
            lock_guard<mutex> lock(ready_mutex_);
            ready_.set_exception(current_exception());
            return;
        }
        // LCOV_EXCL_STOP

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
            if (!shutting_down && poller.has_input(frontend)) // Once shutting down, we no longer read incoming messages.
            {
                int flag;
                do
                {
                    // This is the actual message pump. We read an incoming request and pass it to one of the workers
                    // for processing. The dealer socket ensures fair sharing.
                    frontend.receive(message);
                    flag = frontend.has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    backend.send(message, flag);
                }
                while (flag == zmqpp::socket::send_more);
                if (type_ == RequestType::Twoway)
                {
                    ++pending_requests; // Only twoway requests require us to wait for replies
                }
            }
            if (pending_requests != 0 && poller.has_input(backend))
            {
                // We need to read a reply for an earlier request.
                int flag;
                do
                {
                    // Message pump in the opposite direction, for replies from server to client.
                    backend.receive(message);
                    flag = backend.has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    frontend.send(message, flag);
                }
                while (flag == zmqpp::socket::send_more);
                assert(pending_requests > 0);
                --pending_requests;
            }
            if (shutting_down && pending_requests == 0)
            {
                frontend.close();
                backend.close();
                return;
            }
        }
    }
    catch (...)
    {
        MiddlewareException e("ObjectAdapter: broker thread failure (adapter: " + name_ + ")");
        store_exception(e);
        // We may not have signaled the parent yet, depending on where things went wrong.
        try
        {
            lock_guard<mutex> lock(ready_mutex_);
            ready_.set_exception(make_exception_ptr(e));
        }
        catch (future_error)  // LCOV_EXCL_LINE
        {
        }
    }
}

void ObjectAdapter::worker_thread()
{
    try
    {
        zmqpp::poller poller;

        // Subscribe to ctrl socket. Once this socket becomes readable, that's the command to finish.
        auto ctrl = subscribe_to_ctrl_socket();
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
                capnproto::Request::Reader req;
                unique_ptr<capnp::SegmentArrayMessageReader> message;
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
                            // TODO: log error
                            cerr << "ObjectAdapter: invalid oneway message header "
                                 << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << ")" << endl;
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
                            cerr << "ObjectAdapter: twoway invocation sent to oneway adapter "
                                 << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << ")" << endl;
                        }
                        continue;
                    }
                }
                catch (std::exception const& e)
                {
                    // We get here if header unmarshaling failed.
                    ostringstream s;
                    s << "ObjectAdapter: error unmarshaling request header "
                      << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << "): " << e.what();
                    if (type_ == RequestType::Twoway)
                    {
                        capnp::MallocMessageBuilder b;
                        auto exr = create_unknown_response(b, s.str());
                        sender.send(exr);
                    }
                    else
                    {
                        // TODO: log error
                        cerr << s.str() << endl;
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
    // LCOV_EXCL_START
    catch (...)
    {
        stop_workers();  // Fatal error, we need to stop all other workers and the broker.
        MiddlewareException e("ObjectAdapter: worker thread failure (adapter: " + name_ + ")");
        store_exception(e);
        // We may not have signaled the parent yet, depending on where things went wrong.
        try
        {
            lock_guard<mutex> lock(ready_mutex_);
            ready_.set_exception(make_exception_ptr(e));
        }
        catch (future_error)
        {
        }
    }
    // LCOV_EXCL_STOP
}

void ObjectAdapter::cleanup()
{
    join_with_all_threads();
    servants_.clear();
}

void ObjectAdapter::join_with_all_threads()
{
    for (auto& w : workers_)
    {
        if (w.joinable())
        {
            w.join();
        }
    }
    if (broker_.joinable())
    {
        broker_.join();
    }
}

void ObjectAdapter::store_exception(MiddlewareException& ex)
{
    lock_guard<mutex> lock(state_mutex_);
    exception_ = ex.remember(exception_);
    state_ = Failed;
    state_changed_.notify_all();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
