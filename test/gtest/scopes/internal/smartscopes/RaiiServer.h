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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_API_TEST_SCOPES_INTERNAL_SMARTSCOPES_RAIISERVER_H
#define UNITY_API_TEST_SCOPES_INTERNAL_SMARTSCOPES_RAIISERVER_H

#include <signal.h>
#include <unistd.h>

namespace unity
{

namespace api
{

namespace test
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class RaiiServer
{
public:
    RaiiServer(std::string const& server_path)
    {
        int pipefd[2];
        if (pipe(pipefd) < 0)
        {
            throw unity::ResourceException("Pipe creation failed");
        }

        switch (pid_ = fork())
        {
            case -1:
                throw unity::ResourceException("Failed to fork process");
            case 0: // child
                close(STDOUT_FILENO);   // close stdout
                close(pipefd[0]);       // close read
                if (dup(pipefd[1]) < 0) // open write
                {
                    throw unity::ResourceException("Write pipe duplication failed");
                }

                execl(server_path.c_str(), "", NULL);
                throw unity::ResourceException("Failed to execute fake server script");
            default: // parent
                close(STDIN_FILENO);    // close stdin
                close(pipefd[1]);       // close write
                if (dup(pipefd[0]) < 0) // open read
                {
                    throw unity::ResourceException("Read pipe duplication failed");
                }

                char port_str[10];
                read(pipefd[0], port_str, sizeof(port_str));
                port_ = std::atoi(port_str);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    ~RaiiServer()
    {
        kill(pid_, SIGABRT);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    pid_t pid_ = -1;
    int port_ = 0;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace test

} // namespace api

} // namespace unity

#endif // UNITY_API_TEST_SCOPES_INTERNAL_SMARTSCOPES_RAIISERVER_H
