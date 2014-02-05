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

#include <unity/scopes/internal/ScopeLoader.h>

#include <unity/scopes/Version.h>
#include <unity/UnityExceptions.h>

#include <cassert>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeLoader::ScopeLoader(string const& name, string const& libpath, RegistryProxy const& registry) :
    scope_name_(name),
    dyn_loader_(DynamicLoader::create(libpath, DynamicLoader::Binding::now, DynamicLoader::Unload::noclose)),
    registry_(registry),
    scope_base_(nullptr, reinterpret_cast<DestroyFunction>(dyn_loader_->find_function(UNITY_SCOPE_DESTROY_SYMSTR))),
    scope_state_(ScopeState::Stopped)
{
    // Look for the scope create function in the plug-in and call it.
    // instance. If anything goes wrong below, scope_base_ takes care of destroying it again.
    CreateFunction create_func = reinterpret_cast<CreateFunction>(
                                     dyn_loader_->find_function(UNITY_SCOPE_CREATE_SYMSTR)
                                 );
    scope_base_.reset(create_func());
    if (!scope_base_)
    {
        throw unity::ResourceException("Scope " + scope_name_ + " returned nullptr from " + UNITY_SCOPE_CREATE_SYMSTR);
    }
}

ScopeLoader::~ScopeLoader()
{
    try
    {
        unload();
    }
    catch (...) // LCOV_EXCL_LINE
    {
    }
}

ScopeLoader::UPtr ScopeLoader::load(string const& name, string const& libpath, RegistryProxy const& registry)
{
    return UPtr(new ScopeLoader(name, libpath, registry));
}

void ScopeLoader::unload()
{
    if (scope_state_ == ScopeState::Failed)
    {
        scope_state_ = ScopeState::Finished;
    }

    if (scope_state_ == ScopeState::Finished)
    {
        return;
    }

    stop();
    scope_base_.reset(nullptr);
    scope_state_ = ScopeState::Finished;
}


// Call the scope's start method.

void ScopeLoader::start()
{
    if (scope_state_ == ScopeState::Started)
    {
        return;
    }

    if (scope_state_ == ScopeState::Finished)
    {
        throw unity::LogicException("Cannot start scope " + scope_name_ + " in Finished state");
    }

    if (scope_state_ == ScopeState::Failed)
    {
        throw unity::LogicException("Cannot start scope " + scope_name_ + " in Failed state");
    }

    try
    {
        const int s_version = scope_base_->start(scope_name_, registry_);
        const int maj_version = major_version();
        if (s_version != maj_version)
        {

            try
            {
                scope_base_->stop(); // Make sure to tell the scope to stop again before reporting version mismatch.
            }
            catch (...)
            {
            }
            throw unity::ResourceException("Scope " + scope_name_ + " was compiled with major version " +
                                           std::to_string(s_version) + " of the Unity scopes run time. This " +
                                           "version is incompatible with the current major " +
                                           "version (" + std::to_string(maj_version) + ").");
        }
    }
    catch (...)
    {
        scope_state_ = ScopeState::Failed;
        throw unity::ResourceException("Scope " + scope_name_ +": terminated due to exception in start()");
    }
    scope_state_ = ScopeState::Started;
}

// Call the scope's stop method.

void ScopeLoader::stop()
{
    if (scope_state_ == ScopeState::Stopped)
    {
        return;
    }

    if (scope_state_ == ScopeState::Finished)
    {
        throw unity::LogicException("Cannot stop scope " + scope_name_ + " in Finished state");
    }

    if (scope_state_ == ScopeState::Failed)
    {
        throw unity::LogicException("Cannot stop scope " + scope_name_ + " in Failed state");
    }

    try
    {
        scope_base_->stop();
        scope_state_ = ScopeState::Stopped;
    }
    catch (...)
    {
        scope_state_ = ScopeState::Failed;
        throw unity::ResourceException("Scope " + scope_name_ +": terminated due to exception in stop()");
    }
}

string ScopeLoader::name() const noexcept
{
    return scope_name_;
}

string ScopeLoader::libpath() const noexcept
{
    return dyn_loader_->path();
}

ScopeBase* ScopeLoader::scope_base() const
{
    assert(scope_base_);
    return scope_base_.get();
}

} // namespace internal

} // namespace scopes

} // namespace unity
