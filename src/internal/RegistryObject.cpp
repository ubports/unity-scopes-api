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

#include <scopes/internal/RegistryObject.h>

#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <signal.h>
#include <cassert>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

static bool is_dead(pid_t pid)
{
    return kill(pid, 0) < 0 && errno == ESRCH;
}
/**
 * This does all the hard work. It is not thread safe. All calls to this
 * functionality come via RegistyObject, which takes care of locking.
 */

class RegistryObjectPrivate final {
public:
    RegistryObjectPrivate(RegistryObjectPrivate const&) = delete;
    RegistryObjectPrivate& operator=(RegistryObjectPrivate const&) = delete;

    RegistryObjectPrivate() {};
    ~RegistryObjectPrivate();
    ScopeMetadata get_metadata(std::string const& scope_name);
    ScopeProxy get_scope(std::string const& scope_name);
    MetadataMap list();
    bool add(std::string const& scope_name, ScopeMetadata const& metadata,
            std::vector<std::string> const& spawn_command);
    bool remove(std::string const& scope_name);

private:

    MetadataMap scopes;
    std::map<std::string, pid_t> scope_processes;
    std::map<std::string, std::vector<std::string>> commands;

    void spawn_scope(std::string const& scope_name);
    void shutdown();

};

RegistryObjectPrivate::~RegistryObjectPrivate()
{
    try {
        shutdown();
    } catch(...) {
        // FIXME, write error log.
    }
}

void RegistryObjectPrivate::shutdown()
{
    for(const auto &i : scope_processes) {
        // Currently just shoot children dead.
        // If we want to get fancy and give them a graceful
        // warning, this is the place to do it.
        kill(i.second, SIGKILL);
        waitpid(i.second, nullptr, 0);
    }
    scope_processes.clear();
}

ScopeMetadata RegistryObjectPrivate::get_metadata(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    auto const& it = scopes.find(scope_name);
    if (it == scopes.end())
    {
        throw NotFoundException("Registry::get_metadata(): no such scope",  scope_name);
    }
    return it->second;

}

ScopeProxy RegistryObjectPrivate::get_scope(std::string const& scope_name)
{
    auto metadata = scopes.find(scope_name);
    if(metadata == scopes.end()) {
        throw NotFoundException("Tried to obtain unknown scope", scope_name);
    }
    auto search = scope_processes.find(scope_name);
    if(search == scope_processes.end() || is_dead(search->second)) {
        spawn_scope(scope_name);
    }
    return metadata->second.proxy();
}

void RegistryObjectPrivate::spawn_scope(std::string const& scope_name)
{
    if(scopes.find(scope_name) == scopes.end())
        throw NotFoundException("Tried to spawn an unknown scope.", scope_name);
    auto process = scope_processes.find(scope_name);
    if(process != scope_processes.end()) {
        assert(is_dead(process->second));
        int status;
        waitpid(process->second, &status, 0);
        if(status != 0) {
            // FIXME, print log message.
        }
        scope_processes.erase(scope_name);
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
            const vector<string> &cmd = commands[scope_name];
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
    // FIXME print spawn info to log.
    scope_processes[scope_name] = pid;
}

MetadataMap RegistryObjectPrivate::list()
{
    return scopes;
}

bool RegistryObjectPrivate::add(std::string const& scope_name, ScopeMetadata const& metadata,
        std::vector<std::string> const& spawn_command)
{
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }
    // TODO: check for names containing a slash, because that won't work if we use
    //       the name for a socket in the file system.

    if(scopes.find(scope_name) != scopes.end()) {
        scopes.erase(scope_name);
        commands.erase(scope_name);
    }
    scopes[scope_name] = metadata;
    commands[scope_name] = spawn_command;
    return true;
}

bool RegistryObjectPrivate::remove(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot remove scope with empty name");
    }

    commands.erase(scope_name);
    return scopes.erase(scope_name) == 1;
}


RegistryObject::RegistryObject() : p(new RegistryObjectPrivate())
{
}

RegistryObject::~RegistryObject() noexcept
{
    delete p;
}

ScopeMetadata RegistryObject::get_metadata(std::string const& scope_name)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->get_metadata(scope_name);
}

ScopeProxy RegistryObject::get_scope(std::string const& scope_name) {
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->get_scope(scope_name);
}

MetadataMap RegistryObject::list()
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->list();
}

bool RegistryObject::add(std::string const& scope_name, ScopeMetadata const& metadata,
        std::vector<std::string> const& spawn_command)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->add(scope_name, metadata, spawn_command);
}

bool RegistryObject::remove(std::string const& scope_name)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->remove(scope_name);
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
