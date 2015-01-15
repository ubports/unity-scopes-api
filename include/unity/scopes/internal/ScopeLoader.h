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

#pragma once

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/internal/DynamicLoader.h>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

// ScopeLoader loads the .so for a scope and TODO: complete this and updated comments below.

class ScopeLoader final
{
public:
    NONCOPYABLE(ScopeLoader);
    UNITY_DEFINES_PTRS(ScopeLoader);

    // Creates a ScopeLoader for a scope with the given ID and library.
    static UPtr load(std::string const& scope_id, std::string const& libpath);

    // unload() explicitly finalizes the scope. This is called by the destructor too, but calling it explicity
    // allows the caller to receive any exceptions that may have been produced by the scope thread.
    void unload();

    // These methods start or stop a scope, that is, call through to the scope-provided versions in the dynamic library.
    void start();
    void stop();

    // Returns the scope ID
    std::string scope_id() const noexcept;

    // Returns the library path for the scope
    std::string libpath() const noexcept;

    // Returns the actual implementation provided by the scope.
    unity::scopes::ScopeBase* scope_base() const;

    ~ScopeLoader();

private:
    ScopeLoader(std::string const& scope_id, std::string const& path);

    std::string scope_id_;
    unity::scopes::internal::DynamicLoader::UPtr dyn_loader_;
    std::exception_ptr exception_;

    std::thread scope_thread_;
    std::unique_ptr<ScopeBase, DestroyFunction> scope_base_;

    enum class ScopeState
    {
        Stopped, Started, Failed, Finished
    };
    ScopeState scope_state_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
