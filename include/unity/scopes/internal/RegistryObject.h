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

#ifndef UNITY_SCOPES_INTERNAL_REGISTRYOBJECT_H
#define UNITY_SCOPES_INTERNAL_REGISTRYOBJECT_H

#include <core/posix/child_process.h>
#include <unity/scopes/internal/MWRegistryProxyFwd.h>
#include <unity/scopes/internal/RegistryObjectBase.h>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

class RegistryObject : public RegistryObjectBase
{
public:
    struct ScopeExecData
    {
        ScopeExecData() = default;
        ScopeExecData(std::initializer_list<std::string>) = delete;
        std::string scoperunner_path;
        std::string config_file;
        std::string scope_name;
    };

public:
    UNITY_DEFINES_PTRS(RegistryObject);

    RegistryObject();
    virtual ~RegistryObject();

    // Remote operation implementations
    virtual ScopeMetadata get_metadata(std::string const& scope_name) const override;
    virtual MetadataMap list() const override;
    virtual ScopeProxy locate(std::string const& scope_name) override;

    // Local methods
    bool add_local_scope(ScopeMetadata const& scope, ScopeExecData const& scope_exec_data);
    bool remove_local_scope(std::string const& scope_name);
    void set_remote_registry(MWRegistryProxy const& remote_registry);

private:
    void on_process_death(core::posix::Process const& process);

private:
    class ScopeProcess
    {
    public:
        enum ProcessState
        {
            Stopped, Starting, Running, Stopping
        };

        ScopeProcess(ScopeExecData exec_data);
        ScopeProcess(ScopeProcess const& other);

        void exec();
        void kill();
        void on_process_death(core::posix::Process const& process);

    private:
        ProcessState state();
        bool wait_for_state(ProcessState state, int timeout_ms);
        void update_state(ProcessState state);

    private:
        ScopeExecData exec_data_;
        core::posix::ChildProcess process_ = core::posix::ChildProcess::invalid();
        ProcessState state_ = Stopped;
        std::mutex process_mutex_;
        std::mutex state_mutex_;
        std::condition_variable state_change_cond_;
    };

private:
    mutable std::mutex mutex_;
    MetadataMap scopes_;
    std::map<std::string, ScopeProcess> scope_processes_;
    MWRegistryProxy remote_registry_;

    core::posix::ChildProcess::DeathObserver& death_observer_;
    std::thread death_observer_thread_;
    std::error_code death_observer_error_;
    core::ScopedConnection process_death_conn_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
