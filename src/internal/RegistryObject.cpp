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

class RegistryObjectPrivate {
private:

    MetadataMap scopes;
    std::map<std::string, pid_t> scope_processes;

    int spawn_scope(std::string const& scope_name);
    void shutdown();

public:
    RegistryObjectPrivate() {};
    ~RegistryObjectPrivate();
    ScopeMetadata get_metadata(std::string const& scope_name);
    int get_scope(std::string const& scope_name);
    MetadataMap list();
    bool add(std::string const& scope_name, ScopeMetadata const& metadata);
    bool remove(std::string const& scope_name);
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

int RegistryObjectPrivate::get_scope(std::string const& scope_name)
{
    auto search = scope_processes.find(scope_name);
    if(search != scope_processes.end()) {
        pid_t pid = search->second;
        if(!is_dead(pid)) {
            return 1;
        }
    }
    return spawn_scope(scope_name);
}

int RegistryObjectPrivate::spawn_scope(std::string const& scope_name)
{
    if(scopes.find(scope_name) == scopes.end())
        throw "FixmeLaterException";
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
    pid_t childpid = 52; // FIXME: Fork & exec here.
    scope_processes[scope_name] = childpid;
    return 3;
}

MetadataMap RegistryObjectPrivate::list()
{
    return scopes;
}

bool RegistryObjectPrivate::add(std::string const& scope_name, ScopeMetadata const& metadata)
{
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }
    // TODO: check for names containing a slash, because that won't work if we use
    //       the name for a socket in the file system.

    auto const& pair = scopes.insert(make_pair(scope_name, metadata));
    if (!pair.second)
    {
        // Replace already existing entry with this one
        scopes.erase(pair.first);
        scopes.insert(make_pair(scope_name, metadata));
        return false;
    }
    return true;
}

bool RegistryObjectPrivate::remove(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot remove scope with empty name");
    }

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

int RegistryObject::get_scope(std::string const& scope_name) {
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->get_scope(scope_name);
}

MetadataMap RegistryObject::list()
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->list();
}

bool RegistryObject::add(std::string const& scope_name, ScopeMetadata const& metadata)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return p->add(scope_name, metadata);
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
