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

#include <unity/scopes/internal/ScopeBaseImpl.h>
#include <unity/scopes/internal/SettingsDB.h>
#include <unity/scopes/Version.h>
#include <unity/UnityExceptions.h>

#include <boost/filesystem.hpp>

#include <cassert>
#include <libgen.h>
#include <sys/stat.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeLoader::ScopeLoader(string const& scope_id,
                         string const& libpath,
                         string const& data_dir,
                         RegistryProxy const& registry) :
    scope_id_(scope_id),
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
        throw unity::ResourceException("Scope " + scope_id_ + " returned nullptr from " + UNITY_SCOPE_CREATE_SYMSTR);
    }

    {
        boost::filesystem::path libp(libpath);
        boost::filesystem::path scope_dir(libp.parent_path());
        scope_base_->p->set_scope_directory(scope_dir.native());
    }

    string settings_dir = data_dir + "/" + scope_id;
    string scope_dir = scope_base_->scope_directory();
    string settings_db = settings_dir + "/settings.db";
    string settings_schema = scope_dir + "/" + scope_id + "-settings.ini";
    if (boost::filesystem::exists(settings_schema))
    {
        // Make sure the settings directories exist. (No permission for group and others; data might be sensitive.)
        ::mkdir(data_dir.c_str(), 0700);
        ::mkdir(settings_dir.c_str(), 0700);

        shared_ptr<SettingsDB> db(SettingsDB::create_from_ini_file(settings_db, settings_schema));
        scope_base_->p->set_settings_db(db);
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

ScopeLoader::UPtr ScopeLoader::load(string const& scope_id,
                                    string const& libpath,
                                    string const& data_dir,
                                    RegistryProxy const& registry)
{
    return UPtr(new ScopeLoader(scope_id, libpath, data_dir, registry));
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
        throw unity::LogicException("Cannot start scope " + scope_id_ + " in Finished state");
    }

    if (scope_state_ == ScopeState::Failed)
    {
        throw unity::LogicException("Cannot start scope " + scope_id_ + " in Failed state");
    }

    try
    {
        scope_base_->start(scope_id_, registry_);
    }
    catch (...)
    {
        scope_state_ = ScopeState::Failed;
        throw unity::ResourceException("Scope " + scope_id_ +": terminated due to exception in start()");
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
        throw unity::LogicException("Cannot stop scope " + scope_id_ + " in Finished state");
    }

    if (scope_state_ == ScopeState::Failed)
    {
        throw unity::LogicException("Cannot stop scope " + scope_id_ + " in Failed state");
    }

    try
    {
        scope_base_->stop();
        scope_state_ = ScopeState::Stopped;
    }
    catch (...)
    {
        scope_state_ = ScopeState::Failed;
        throw unity::ResourceException("Scope " + scope_id_ +": terminated due to exception in stop()");
    }
}

string ScopeLoader::scope_id() const noexcept
{
    return scope_id_;
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
