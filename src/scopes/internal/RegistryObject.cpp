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

#include <unity/scopes/internal/RegistryObject.h>

#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <signal.h>
#include <cassert>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

RegistryObject::RegistryObject()
{
}

RegistryObject::~RegistryObject()
{
    try
    {
        shutdown();
    }
    catch (std::exception const& e)
    {
        fprintf(stderr, "scoperegistry: shutdown error: %s\n", e.what());
    }
    catch (...)
    {
        fprintf(stderr, "scoperegistry: unknown exception during shutdown\n");
    }
}

ScopeMetadata RegistryObject::get_metadata(std::string const& scope_id)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    // Look for the scope in both the local and the remote map.
    // Local scopes take precedence over remote ones of the same
    // name. (Ideally, this will never happen, except maybe
    // during development.)
    auto const& it = scopes_.find(scope_id);
    if (it != scopes_.end())
    {
        return it->second;
    }

    if (remote_registry_)
    {
        return remote_registry_->get_metadata(scope_id);
    }

    throw NotFoundException("Registry::get_metadata(): no such scope",  scope_id);
}

MetadataMap RegistryObject::list()
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    MetadataMap all_scopes(scopes_);  // Local scopes

    // If a remote scope has the same name as a local one,
    // this will not overwrite a local scope with a remote
    // one if they have the same name.
    if (remote_registry_)
    {
        MetadataMap remote_scopes = remote_registry_->list();
        all_scopes.insert(remote_scopes.begin(), remote_scopes.end());
    }

    return all_scopes;
}

bool RegistryObject::add_local_scope(std::string const& scope_id, ScopeMetadata const& metadata,
                                     std::vector<std::string> const& spawn_command)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    bool return_value = true;
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }
    if(scope_id.find('/') != std::string::npos) {
        throw unity::InvalidArgumentException("Registry: Cannot create a scope with a slash in its name");
    }

    if (scopes_.find(scope_id) != scopes_.end())
    {
        auto proc = scope_processes_.find(scope_id);
        if (proc != scope_processes_.end())
        {
            kill_process(proc->second);
            scope_processes_.erase(scope_id);
        }
        scopes_.erase(scope_id);
        commands_.erase(scope_id);
        return_value = false;
    }
    scopes_.insert(make_pair(scope_id, metadata));
    commands_[scope_id] = spawn_command;
    return return_value;
}

bool RegistryObject::remove_local_scope(std::string const& scope_id)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot remove scope with empty name");
    }

    commands_.erase(scope_id);
    return scopes_.erase(scope_id) == 1;
}

void RegistryObject::set_remote_registry(MWRegistryProxy const& remote_registry)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    remote_registry_ = remote_registry;
}

ScopeProxy RegistryObject::locate(std::string const& scope_id)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
        throw unity::InvalidArgumentException("Registry: Cannot locate scope with empty name");
    auto metadata = scopes_.find(scope_id);
    if (metadata == scopes_.end())
    {
        throw NotFoundException("Tried to obtain unknown scope", scope_id);
    }
    auto search = scope_processes_.find(scope_id);
    if (search == scope_processes_.end() || is_dead(search->second))
    {
        spawn_scope(scope_id);
    }
    return metadata->second.proxy();
}

void RegistryObject::spawn_scope(std::string const& scope_id)
{
    if (scopes_.find(scope_id) == scopes_.end())
    {
        throw NotFoundException("Tried to spawn an unknown scope.", scope_id);
    }
    auto process = scope_processes_.find(scope_id);
    if (process != scope_processes_.end())
    {
        assert(is_dead(process->second));
        int status;
        waitpid(process->second, &status, 0);
        if (status != 0)
        {
            printf("scope %s has exited with nonzero error status %d.\n", scope_id.c_str(), status);
        }
        scope_processes_.erase(scope_id);
    }

    pid_t pid;
    switch (pid = fork())
    {
        case -1:
        {
            throw SyscallException("cannot fork", errno);
        }
        case 0: // child
        {
            const vector<string>& cmd = commands_[scope_id];
            assert(cmd.size() == 3);
            // Includes room for final NULL element.
            unique_ptr<char const* []> argv(new char const*[4]);
            argv[0] = cmd[0].c_str();
            argv[1] = cmd[1].c_str();
            argv[2] = cmd[2].c_str();
            argv[3] = nullptr;
            execv(argv[0], const_cast<char* const*>(argv.get()));
            throw SyscallException("cannot exec scoperunner", errno);
        }
    }
    const vector<string>& cmd = commands_[scope_id];
    printf("spawning scope %s to process number %d with command line %s %s %s.\n",
           scope_id.c_str(), (int)pid, cmd[0].c_str(), cmd[1].c_str(), cmd[2].c_str());
    scope_processes_[scope_id] = pid;
}

void RegistryObject::shutdown()
{
    for (const auto &i : scope_processes_)
    {
        kill_process(i.second);
        // If and when we move to graceful shutdown, check that exit status
        // was zero and print error message here.
    }
    scope_processes_.clear();
    commands_.clear();
}

int RegistryObject::kill_process(pid_t pid) {
    int exitcode;
    // Currently just shoot children dead.
    // If we want to get fancy and give them a graceful
    // warning, this is the place to do it.
    kill(pid, SIGKILL);
    waitpid(pid, &exitcode, 0);
    return exitcode;
}

bool RegistryObject::is_dead(pid_t pid)
{
    return kill(pid, 0) < 0 && errno == ESRCH;
}

} // namespace internal

} // namespace scopes

} // namespace unity
