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

#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>

#include <unity/scopes/internal/zmq_middleware/ServantBase.h>
#include <unity/scopes/internal/zmq_middleware/StopPublisher.h>
#include <unity/scopes/internal/zmq_middleware/Util.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReceiver.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSender.h>
#include <unity/UnityExceptions.h>
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

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

namespace
{

char const* pump_suffix = "-pump";

}  // namespace

ObjectAdapter::ObjectAdapter(ZmqMiddleware& mw, string const& name, string const& endpoint, RequestMode m,
                             int pool_size, int64_t idle_timeout) :
    mw_(mw),
    name_(name),
    endpoint_(endpoint),
    mode_(m),
    pool_size_(pool_size),
    idle_timeout_(idle_timeout != -1 ? idle_timeout : zmqpp::poller::wait_forever),
    state_(Inactive)
{
    assert(!name.empty());
    assert(!endpoint.empty());
    assert(pool_size >= 1);
    throw_if_bad_endpoint(endpoint);
}

ObjectAdapter::~ObjectAdapter()
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
            throw_bad_state("add()", state_);
        }
    }

    auto pair = servants_.insert(make_pair(id, obj));
    if (!pair.second)
    {
        ostringstream s;
        s << "ObjectAdapter::add(): " << "cannot add id \"" << id << "\": id already in use (adapter: " << name_  << ")";
        throw MiddlewareException(s.str());
    }
    return ZmqProxy(new ZmqObjectProxy(&mw_, endpoint_, id, "", mode_));
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
                throw_bad_state("remove()", state_);
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
            throw_bad_state("find()", state_);
        }
    }

    auto it = servants_.find(id);
    if (it != servants_.end())
    {
        return it->second;
    }
    return shared_ptr<ServantBase>();
}

void ObjectAdapter::add_dflt_servant(std::string const& category, std::shared_ptr<ServantBase> const& obj)
{
    if (!obj)
    {
        throw InvalidArgumentException("ObjectAdapter::add_dflt_servant(): invalid nullptr object (adapter: " + name_ + ")");
    }

    lock(map_mutex_, state_mutex_);
    lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
    {
        lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
        if (state_ == Destroyed || state_ == Failed)
        {
            throw_bad_state("add_dflt_servant()", state_);
        }
    }

    auto pair = dflt_servants_.insert(make_pair(category, obj));
    if (!pair.second)
    {
        ostringstream s;
        s << "ObjectAdapter::add_dflt_servant(): " << "cannot add category \"" << category
          << "\": category already in use (adapter: " << name_  << ")";
        throw MiddlewareException(s.str());
    }
}

void ObjectAdapter::remove_dflt_servant(std::string const& category)
{
    shared_ptr<ServantBase> servant;
    {
        lock(map_mutex_, state_mutex_);
        lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
        {
            lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
            if (state_ == Destroyed || state_ == Failed)
            {
                throw_bad_state("remove_dflt_servant()", state_);
            }
        }

        auto it = dflt_servants_.find(category);
        if (it == dflt_servants_.end())
        {
            ostringstream s;
            s << "ObjectAdapter::remove_dflt_servant(): " << "cannot remove category \"" << category
              << "\": category not present (adapter: " << name_ << ")";
            throw MiddlewareException(s.str());
        }
        servant = it->second;
        dflt_servants_.erase(it);
    }
    // Lock released here, so we don't call servant destructor while holding a lock

    servant = nullptr;  // This may trigger destructor call on the servant
}

shared_ptr<ServantBase> ObjectAdapter::find_dflt_servant(std::string const& category) const
{
    lock(map_mutex_, state_mutex_);
    lock_guard<mutex> map_lock(map_mutex_, adopt_lock);
    {
        lock_guard<mutex> state_lock(state_mutex_, adopt_lock);
        if (state_ == Destroyed || state_ == Failed)
        {
            throw_bad_state("find_dflt_servant()", state_);
        }
    }

    auto it = dflt_servants_.find(category);
    if (it != dflt_servants_.end())
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
            throw_bad_state("activate()", state_);
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
            throw_bad_state("shutdown() [state_ == Failed]", state_);
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
                throw_bad_state("shutdown() [state == Activating]", state_);
            }
            // LCOV_EXCL_STOP

            // FALLTHROUGH
        }
        case Active:
        {
            state_ = Deactivating;
            stopper_->stop();
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

    {
        unique_lock<mutex> lock(state_mutex_);
        if (stopper_)
        {
            stopper_->wait_until_stopped();
        }
    }

    if (state == Failed)
    {
        throw_bad_state("wait_for_shutdown()", state);
    }
}

void ObjectAdapter::throw_bad_state(string const& label, AdapterState state) const
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
    MiddlewareException e(label + ": Object adapter in " + bad_state + " state (adapter: " + name_ + ")");
    e.remember(exception_);  // Remember any exception encountered by the broker thread or a worker thread.
    throw e;
}

void ObjectAdapter::run_workers()
{
    {
        lock_guard<mutex> state_lock(state_mutex_);
        try
        {
            // Start the publisher for stop messages.
            stopper_.reset(new StopPublisher(mw_.context(), name_ + "-stopper"));
        }
        catch (...)
        {
            state_ = Failed;
            state_changed_.notify_all();
            throw MiddlewareException("ObjectAdapter::run_workers(): stop thread failure (adapter: " + name_ + ")"); // LCOV_EXCL_LINE
        }
    }

    // Start pump.
    auto ready = promise<void>();
    auto f = ready.get_future();
    pump_ = thread(&ObjectAdapter::pump, this, move(ready));
    f.get();
}

shared_ptr<ServantBase> ObjectAdapter::find_servant(string const& id, string const& category)
{
    shared_ptr<ServantBase> servant = find(id);
    if (!servant)
    {
        servant = find_dflt_servant(category);
    }
    return servant;
}

// Load-balancing message pump using router-router sockets (for twoway) or
// pull-router (for oneway). Loosely follows the "Load Balancing Broker" in the Zmq Guide.
// (Workers use a REQ socket to pull work from the pump.)

void ObjectAdapter::pump(std::promise<void> ready)
{
    try
    {
        zmqpp::poller poller;

        zmqpp::socket_type stype = mode_ == RequestMode::Twoway ? zmqpp::socket_type::router : zmqpp::socket_type::pull;

        // Front-end receives incoming requests from clients. (Frontend is in the server role.)
        zmqpp::socket frontend(*mw_.context(), stype);
        frontend.set(zmqpp::socket_option::linger, 50);
        // "Safe" bind: prevents two servers from binding to the same endpoint.
        safe_bind(frontend, endpoint_);
        // We do not add the frontend to the poller just yet because we poll the frontend
        // only when workers are ready to process a request.

        // The backend replies to requests from workers for more work. (Backend is in the server role.)
        // This means that our *reply* to the request from the worker contains the incoming
        // request from the client, and the next *request* from the worker contains the result that
        // is to go back to the client via the frontend (or the ready message for a oneway worker).
        zmqpp::socket backend(*mw_.context(), zmqpp::socket_type::router);
        backend.set(zmqpp::socket_option::linger, 50);
        backend.bind("inproc://" + name_ + pump_suffix);
        poller.add(backend);

        // Subscribe to stop socket. Once this socket becomes readable, that's the command to finish.
        auto stop = stopper_->subscribe();
        poller.add(stop);

        // Fire up the worker threads.
        int num_workers = 0;
        for (int i = 0; i < pool_size_; ++i)
        {
            workers_.push_back(thread(&ObjectAdapter::worker, this));
            ++num_workers;
        }

        // Tell parent that we are ready
        ready.set_value();

        // Start the pump.
        bool shutting_down = false;
        queue<string> ready_workers;

        for (;;)
        {
            if (!poller.poll(idle_timeout_))
            {
                // Shut down, no activity for the idle timeout period.
                mw_.stop();
            }
            if (!shutting_down && poller.has_input(stop))
            {
                // When the stop socket becomes ready, we need to get out of here.
                // We stop reading more requests from the router, but continue processing
                // while there are still outstanding replies for twoway requests.
                poller.remove(stop);
                stop.close();
                if (poller.has(frontend))
                {
                    poller.remove(frontend);  // Don't accept any more request from clients.
                }
                shutting_down = true;
            }
            if (poller.has_input(backend))
            {
                // A worker is asking for more work to do.
                string worker_id;
                backend.receive(worker_id);          // First frame: worker ID for LRU routing
                ready_workers.push(worker_id);       // Thread will be ready again in a sec
                if (!shutting_down && ready_workers.size() == 1)
                {
                    // We poll the front end when there is at least one worker
                    poller.add(frontend);
                }
                string buf;
                backend.receive(buf);                // Second frame: empty delimiter frame
                assert(buf.empty());
                string client_address;
                backend.receive(client_address);     // Third frame: "ready" or client reply address
                if (client_address != "ready")
                {
                    backend.receive(buf);            // Fourth frame: empty delimiter frame
                    assert(buf.empty());
                    if (mode_ == RequestMode::Twoway)
                    {
                        frontend.send(client_address, zmqpp::socket::send_more);   // Client address tells router where to send the reply to
                        frontend.send("", zmqpp::socket::send_more);               // Empty delimiter frame
                        // Read reply contents and return them to client via frontend
                        int flag;
                        do
                        {
                            backend.receive(buf);
                            flag = backend.has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                            frontend.send(buf, flag);
                        } while (flag == zmqpp::socket::send_more);
                    }
                }
            }
            if (!shutting_down && poller.has(frontend) && poller.has_input(frontend))
            {
                // Incoming request from client.
                string client_address;
                if (mode_ == RequestMode::Twoway)
                {
                    frontend.receive(client_address);  // First frame: client address
                    string buf;
                    frontend.receive(buf);             // Second frame: empty delimiter frame
                    assert(buf.empty());
                }
                string worker_id = ready_workers.front();
                ready_workers.pop();
                if (ready_workers.size() == 0)  // Stop reading from frontend once all workers are busy.
                {
                    poller.remove(frontend);
                }

                // Give incoming request to worker.
                backend.send(worker_id, zmqpp::socket::send_more);
                backend.send("", zmqpp::socket::send_more);
                backend.send(client_address, zmqpp::socket::send_more);
                backend.send("", zmqpp::socket::send_more);
                int flag;
                do
                {
                    string buf;
                    frontend.receive(buf);
                    flag = frontend.has_more_parts() ? zmqpp::socket::send_more : zmqpp::socket::normal;
                    backend.send(buf, flag);
                } while (flag == zmqpp::socket::send_more);
            }
            if (shutting_down)
            {
                // Tell each worker that is ready to stop. This automatically
                // "waits" for executing twoway requests to complete because
                // a worker that's doing work isn't ready. Once all workers
                // have been told to stop, we are done.
                while (ready_workers.size() > 0)
                {
                    string worker_id = ready_workers.front();
                    ready_workers.pop();
                    backend.send(worker_id, zmqpp::socket::send_more);
                    backend.send("", zmqpp::socket::send_more);
                    backend.send("stop");
                    if (--num_workers == 0)
                    {
                        return;
                    }
                }
            }
        }
    }
    catch (...)
    {
        MiddlewareException e("ObjectAdapter: pump thread failure (adapter: " + name_ + ")");
        store_exception(e);
        // We may not have signaled the parent yet, depending on where things went wrong.
        try
        {
            stopper_->stop();
            ready.set_exception(make_exception_ptr(e));
        }
        catch (future_error)  // LCOV_EXCL_LINE
        {
        }
    }
}

void ObjectAdapter::worker()
{
    try
    {
        zmqpp::socket pump(*mw_.context(), zmqpp::socket_type::req);
        pump.set(zmqpp::socket_option::linger, 50);
        pump.connect("inproc://" + name_ + pump_suffix);
        pump.send("ready");                             // First message tells pump that we are ready.

        for (;;)
        {
            string client_address;
            pump.receive(client_address);
            if (client_address == "stop")  // pump has decided to stop
            {
                return;
            }
            string buf;
            pump.receive(buf);
            assert(buf.empty());

            // Any bytes remaining in the input are the marshaled request payload.
            dispatch(pump, client_address);

            if (mode_ == RequestMode::Oneway)
            {
                // Oneway requests don't have a reply, so we send a dummy "reply" to tell
                // the pump that we are ready for another request.
                pump.send("ready");
            }
        }
    }
    // LCOV_EXCL_START
    catch (...)
    {
        stopper_->stop();  // Fatal error, we need to stop all other workers and the pump.
        MiddlewareException e("ObjectAdapter: worker thread failure (adapter: " + name_ + ")");
        store_exception(e);
    }
    // LCOV_EXCL_STOP
}

// Unmarshal input parameters, dispatch to servant and, if this is a twoway request,
// marshal the results (or exception).

void ObjectAdapter::dispatch(zmqpp::socket& pump, string const& client_address)
{
    ZmqSender sender(pump);    // Unused for oneway requests
    capnproto::Request::Reader req;
    Current current;
    ZmqReceiver receiver(pump);
    unique_ptr<capnp::SegmentArrayMessageReader> message;

    try
    {
        // Unmarshal the type-independent part of the message (id, category, operation name, mode).
        auto segments = receiver.receive();
        message.reset(new capnp::SegmentArrayMessageReader(segments));
        req = message->getRoot<capnproto::Request>();

        current.adapter = this;
        current.id = req.getId().cStr();
        current.category = req.getCat().cStr();
        current.op_name = req.getOpName().cStr();
        auto mode = req.getMode();
        if (current.id.empty() || current.op_name.empty() ||
            (mode != capnproto::RequestMode::TWOWAY && mode != capnproto::RequestMode::ONEWAY))
        {
            // Something is wrong with the request header.
            if (mode_ == RequestMode::Twoway)
            {
                pump.send(client_address, zmqpp::socket::send_more);
                pump.send("", zmqpp::socket::send_more);
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
            return;
        }
        auto expected_mode = mode_ == RequestMode::Twoway ? capnproto::RequestMode::TWOWAY : capnproto::RequestMode::ONEWAY;
        if (mode != expected_mode) // Can't do oneway on a twoway adapter and vice-versa.
        {
            if (mode_ == RequestMode::Twoway)
            {
                pump.send(client_address, zmqpp::socket::send_more);
                pump.send("", zmqpp::socket::send_more);
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
            return;
        }
    }
    catch (std::exception const& e)
    {
        // We get here if header unmarshaling failed.
        ostringstream s;
        s << "ObjectAdapter: error unmarshaling request header "
          << "(id: " << current.id << ", adapter: " << name_ << ", op: " << current.op_name << "): " << e.what();
        if (mode_ == RequestMode::Twoway)
        {
            pump.send(client_address, zmqpp::socket::send_more);
            pump.send("", zmqpp::socket::send_more);
            capnp::MallocMessageBuilder b;
            auto exr = create_unknown_response(b, s.str());
            sender.send(exr);
        }
        else
        {
            // TODO: log error
            cerr << s.str() << endl;
        }
        return;
    }

    // Look for a servant with matching id.
    shared_ptr<ServantBase> servant;
    try
    {
        servant = find_servant(current.id, current.category);
    }
    catch (std::exception const&)
    {
        // Ignore failure to find servant during destruction phase.
    }

    if (!servant)
    {
        if (mode_ == RequestMode::Twoway)
        {
            pump.send(client_address, zmqpp::socket::send_more);
            pump.send("", zmqpp::socket::send_more);
            capnp::MallocMessageBuilder b;
            auto exr = create_object_not_exist_response(b, current);
            sender.send(exr);
        }
        return;
    }

    // We have a target object, so we can ask it to unmarshal the in-params, forward
    // the invocation to the application-provided method, and to marshal the results.
    auto in_params = req.getInParams();
    capnp::MallocMessageBuilder b;
    auto r = b.initRoot<capnproto::Response>();
    servant->safe_dispatch_(current, in_params, r); // noexcept
    if (mode_ == RequestMode::Twoway)
    {
        pump.send(client_address, zmqpp::socket::send_more);
        pump.send("", zmqpp::socket::send_more);
        sender.send(b.getSegmentsForOutput());
    }
}

void ObjectAdapter::cleanup()
{
    join_with_all_threads();
    {
        // Need a full fence here to make sure this thread sees up-to-date
        // memory for servants_ and dflt_servants_.
        lock_guard<mutex> lock(map_mutex_);
    }
    // Don't hold a lock while the servant destructors run.
    servants_.clear();
    dflt_servants_.clear();
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
    if (pump_.joinable())
    {
        pump_.join();
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

} // namespace unity
