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

#include <unity/scopes/internal/RegistryObject.h>

#include <core/posix/exec.h>
#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

///! TODO: get from config
static const int c_process_wait_timeout = 1000;

RegistryObject::RegistryObject()
    : death_observer_(
          core::posix::ChildProcess::DeathObserver::instance()),
      death_observer_thread_(
          [this]() { death_observer_.instance().run(death_observer_error_); }),
      process_death_conn_(
          death_observer_.instance().child_died().connect(
          [this](const core::posix::ChildProcess& child) { on_process_death(child); }))
{
}

RegistryObject::~RegistryObject()
{
    // destroy all scopes and processes
    scopes_.clear();
    scope_processes_.clear();

    // stop the death oberver
    try
    {
        death_observer_.quit();
        if (death_observer_thread_.joinable())
        {
            death_observer_thread_.join();
        }
    }
    catch(std::exception const& e)
    {
        cerr << "RegistryObject::~RegistryObject(): " << e.what() << endl;
        death_observer_thread_.detach();
    }
}

ScopeMetadata RegistryObject::get_metadata(std::string const& scope_id) const
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the id is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("RegistryObject::get_metadata(): Cannot search for scope with empty id");
    }

    // Look for the scope in both the local and the remote map.
    // Local scopes take precedence over remote ones of the same
    // id. (Ideally, this should never happen.)
    auto const& scope_it = scopes_.find(scope_id);
    if (scope_it != scopes_.end())
    {
        return scope_it->second;
    }

    if (remote_registry_)
    {
        return remote_registry_->get_metadata(scope_id);
    }

    throw NotFoundException("RegistryObject::get_metadata(): no such scope: ",  scope_id);
}

MetadataMap RegistryObject::list() const
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    MetadataMap all_scopes(scopes_);  // Local scopes

    // If a remote scope has the same id as a local one,
    // this will not overwrite a local scope with a remote
    // one if they have the same id.
    if (remote_registry_)
    {
        MetadataMap remote_scopes = remote_registry_->list();
        all_scopes.insert(remote_scopes.begin(), remote_scopes.end());
    }

    return all_scopes;
}

ScopeProxy RegistryObject::locate(std::string const& scope_id)
{
    decltype(scopes_.cbegin()) scope_it;
    decltype(scope_processes_.begin()) proc_it;

    {
        lock_guard<decltype(mutex_)> lock(mutex_);
        // If the id is empty, it was sent as empty by the remote client.
        if (scope_id.empty())
        {
            throw unity::InvalidArgumentException("Registry::locate(): Cannot locate scope with empty id");
        }

        scope_it = scopes_.find(scope_id);
        if (scope_it == scopes_.end())
        {
            throw NotFoundException("Registry::locate(): Tried to locate unknown local scope", scope_id);
        }

        proc_it = scope_processes_.find(scope_id);
        if (proc_it == scope_processes_.end())
        {
            throw NotFoundException("Registry::locate(): Tried to exec unknown local scope", scope_id);
        }
    }

    proc_it->second.exec();
    return scope_it->second.proxy();
}

bool RegistryObject::add_local_scope(std::string const& scope_id, ScopeMetadata const& metadata,
                                     ScopeExecData const& exec_data)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    bool return_value = true;
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("RegistryObject::add_local_scope(): Cannot add scope with empty id");
    }
    if (scope_id.find('/') != std::string::npos)
    {
        throw unity::InvalidArgumentException("RegistryObject::add_local_scope(): Cannot create a scope with '/' in its id");
    }

    if (scopes_.find(scope_id) != scopes_.end())
    {
        scopes_.erase(scope_id);
        scope_processes_.erase(scope_id);
        return_value = false;
    }
    scopes_.insert(make_pair(scope_id, metadata));
    scope_processes_.insert(make_pair(scope_id, ScopeProcess(exec_data)));
    return return_value;
}

bool RegistryObject::remove_local_scope(std::string const& scope_id)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the id is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("RegistryObject::remove_local_scope(): Cannot remove scope "
                                              "with empty id");
    }

    scope_processes_.erase(scope_id);
    return scopes_.erase(scope_id) == 1;
}

void RegistryObject::set_remote_registry(MWRegistryProxy const& remote_registry)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    remote_registry_ = remote_registry;
}

bool RegistryObject::is_scope_running( std::string const& scope_id )
{
    auto it = scope_processes_.find( scope_id );
    if (it != scope_processes_.end())
    {
        return it->second.state() != ScopeProcess::ProcessState::Stopped;
    }

    throw NotFoundException("RegistryObject::is_scope_process_running(): no such scope: ",  scope_id);
}

void RegistryObject::on_process_death(core::posix::Process const& process)
{
    // the death observer has signaled that a child has died.
    // broadcast this message to each scope process until we have found the process in question.
    // (this is slightly more efficient than just connecting the signal to every scope process)
    pid_t pid = process.pid();
    for (auto& scope_process : scope_processes_)
    {
        if (scope_process.second.on_process_death(pid))
            break;
    }
}

RegistryObject::ScopeProcess::ScopeProcess(ScopeExecData exec_data)
    : exec_data_(exec_data)
{
}

RegistryObject::ScopeProcess::ScopeProcess(ScopeProcess const& other)
    : exec_data_(other.exec_data_)
{
}

RegistryObject::ScopeProcess::~ScopeProcess()
{
    try
    {
        kill();
    }
    catch(std::exception const& e)
    {
        cerr << "RegistryObject::ScopeProcess::~ScopeProcess(): " << e.what() << endl;
    }
}

RegistryObject::ScopeProcess::ProcessState RegistryObject::ScopeProcess::state()
{
    std::lock_guard<std::mutex> lock(process_mutex_);
    return state_;
}

bool RegistryObject::ScopeProcess::wait_for_state(ProcessState state, int timeout_ms) const
{
    std::unique_lock<std::mutex> lock(process_mutex_);
    return wait_for_state_unlocked(lock, state, timeout_ms);
}

void RegistryObject::ScopeProcess::exec()
{
    std::unique_lock<std::mutex> lock(process_mutex_);

    // 1. check if the scope is running.
    //  1.1. if scope already running, return.
    if (state_ == ScopeProcess::Running)
    {
        return;
    }
    //  1.2. if scope running but is “stopping”, wait for it to stop / kill it.
    else if (state_ == ScopeProcess::Stopping)
    {
        if (!wait_for_state_unlocked(lock, ScopeProcess::Stopped, c_process_wait_timeout))
        {
            cerr << "RegistryObject::ScopeProcess::exec(): Force killing process. Scope: \""
                 << exec_data_.scope_id << "\" took too long to stop." << endl;
            try
            {
                kill_unlocked(lock);
            }
            catch(std::exception const& e)
            {
                cerr << "RegistryObject::ScopeProcess::exec(): " << e.what() << endl;
            }
        }
    }

    // 2. exec the scope.
    update_state_unlocked(Starting);

    const std::string program{exec_data_.scoperunner_path};
    const std::vector<std::string> argv = {exec_data_.runtime_config, exec_data_.scope_config};

    std::map<std::string, std::string> env;
    core::posix::this_process::env::for_each([&env](const std::string& key, const std::string& value)
    {
        env.insert(std::make_pair(key, value));
    });

    {
        process_ = core::posix::exec(program, argv, env,
                                     core::posix::StandardStream::stdin | core::posix::StandardStream::stdout);
        if (process_.pid() <= 0)
        {
            clear_handle_unlocked();
            throw unity::ResourceException("RegistryObject::ScopeProcess::exec(): Failed to exec scope via command: \""
                                           + exec_data_.scoperunner_path + " " + exec_data_.runtime_config + " "
                                           + exec_data_.scope_config + "\"");
        }
    }

    ///! TODO: This should not be here. A ready signal from the scope should trigger "running".
    update_state_unlocked(Running);

    // 3. wait for scope to be "running".
    //  3.1. when ready, return.
    //  3.2. OR if timeout, kill process and throw.
    if (!wait_for_state_unlocked(lock, ScopeProcess::Running, c_process_wait_timeout))
    {
        try
        {
            kill_unlocked(lock);
        }
        catch(std::exception const& e)
        {
            cerr << "RegistryObject::ScopeProcess::exec(): " << e.what() << endl;
        }
        throw unity::ResourceException("RegistryObject::ScopeProcess::exec(): exec aborted. Scope: \""
                                       + exec_data_.scope_id + "\" took too long to start.");
    }

    cout << "RegistryObject::ScopeProcess::exec(): Process for scope: \"" << exec_data_.scope_id << "\" started" << endl;

    // 4. add the scope process to the death observer
    core::posix::ChildProcess::DeathObserver::instance().add(process_);
}

void RegistryObject::ScopeProcess::kill()
{
    std::unique_lock<std::mutex> lock(process_mutex_);
    kill_unlocked(lock);
}

bool RegistryObject::ScopeProcess::on_process_death(pid_t pid)
{
    std::lock_guard<std::mutex> lock(process_mutex_);

    // check if this is the process reported to have died
    if (pid == process_.pid())
    {
        cout << "RegistryObject::ScopeProcess::on_process_death(): Process for scope: \"" << exec_data_.scope_id
             << "\" terminated" << endl;
        clear_handle_unlocked();
        return true;
    }

    return false;
}

void RegistryObject::ScopeProcess::clear_handle_unlocked()
{
    process_ = core::posix::ChildProcess::invalid();
    update_state_unlocked(Stopped);
}

void RegistryObject::ScopeProcess::update_state_unlocked(ProcessState state)
{
    state_ = state;
    state_change_cond_.notify_all();
}

bool RegistryObject::ScopeProcess::wait_for_state_unlocked(std::unique_lock<std::mutex>& lock,
                                                           ProcessState state, int timeout_ms) const
{
    // keep track of time left as process can undergo multiple state changes
    // before reaching the state we want
    int time_left = timeout_ms;
    while (state_ != state && time_left > 0)
    {
        auto start = std::chrono::high_resolution_clock::now();
        state_change_cond_.wait_for(lock, std::chrono::milliseconds(time_left));

        // update time left
        time_left -= std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - start).count();
    }

    return state_ == state;
}

void RegistryObject::ScopeProcess::kill_unlocked(std::unique_lock<std::mutex>& lock)
{
    if (state_ == Stopped)
    {
        return;
    }

    try
    {
        // first try to close the scope process gracefully
        process_.send_signal_or_throw(core::posix::Signal::sig_term);

        if (!wait_for_state_unlocked(lock, ScopeProcess::Stopped, c_process_wait_timeout))
        {
            // scope is taking too long to close, send kill signal
            std::error_code ec;
            process_.send_signal(core::posix::Signal::sig_kill, ec);

            throw unity::ResourceException("Scope: \"" + exec_data_.scope_id
                 + "\" is taking longer than expected to terminate (This process is "
                 + "likely to close upon termination of the parent application).");
        }
    }
    catch (std::exception const&)
    {
        // invalidate the process handle and move on, as the previous handle is clearly
        // unrecoverable at this point.
        clear_handle_unlocked();

        cerr << "RegistryObject::ScopeProcess::in_lock_kill(): Failed to kill scope: \""
             << exec_data_.scope_id << "\"" << endl;
        throw;
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
