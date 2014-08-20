/*
 * Copyright (C) 2014 Canonical Ltd
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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_STOPPUBLISHER_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_STOPPUBLISHER_H

#include <unity/util/NonCopyable.h>

#include <zmqpp/context.hpp>
#include <zmqpp/socket.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// StopPublisher provides a way to use Zmq pub/sub without falling prey
// to the slow joiner syndrome. The provided name is used to create an inproc
// endpoint by prefixing the name with "inproc://".
// Subscribers create a socket by calling subscribe(). The returned
// socket becomes ready for reading (for a zero-length message) once
// some thread calls stop(). Multiple calls to stop() are benign.

class StopPublisher final
{
public:
    NONCOPYABLE(StopPublisher);

    StopPublisher(zmqpp::context* context, std::string const& inproc_name);
    ~StopPublisher();

    std::string endpoint() const;
    zmqpp::socket subscribe();
    void stop() noexcept;
    void wait_until_stopped() noexcept;

private:
    zmqpp::context* context_;
    std::string endpoint_;
    std::thread thread_;
    std::exception_ptr ex_;

    enum State { Starting, Started, Stopping, Stopped, Failed };
    State state_;
    bool stop_sent_;

    mutable std::mutex m_;
    std::condition_variable cond_;

    void stopper_thread() noexcept;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
