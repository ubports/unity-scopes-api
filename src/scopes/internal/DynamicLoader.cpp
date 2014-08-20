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

#include <unity/scopes/internal/DynamicLoader.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <dlfcn.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

DynamicLoader::LibraryHandles DynamicLoader::handles_;
mutex DynamicLoader::mutex_;

DynamicLoader::DynamicLoader(string const& path, Binding b, Unload ul)
{
    // Prevent concurrent access to handles_ and guard against concurrent
    // calls to dlopen()/dlclose(), which are not thread-safe.
    lock_guard<decltype(mutex_)> lock(mutex_);

    handle_ = nullptr;
    path_ = path;
    unload_ = ul;

    // If the caller doesn't want dlclose() to be called, we look whether we've loaded
    // this library previously and, if so, re-use its handle.
    if (ul == Unload::noclose)
    {
        auto it = handles_.find(path);
        if (it != handles_.end())
        {
            handle_ = it->second;
            assert(handle_);
            return;
        }
    }

    assert(ul == Unload::automatic || handle_ == nullptr);

    if ((handle_ = dlopen(path.c_str(), b == Binding::lazy ? RTLD_LAZY : RTLD_NOW)) == nullptr)
    {
        throw ResourceException(dlerror());
    }

    if (ul == Unload::noclose)
    {
        handles_[path] = handle_; // Remember the library for re-use.
    }
}

DynamicLoader::~DynamicLoader()
{
    lock_guard<decltype(mutex_)> lock(mutex_);

    if (unload_ == Unload::automatic)
    {
        assert(handle_);
        dlclose(handle_);
    }
}

DynamicLoader::UPtr DynamicLoader::create(string const& path, Binding b, Unload ul)
{
    return UPtr(new DynamicLoader(path, b, ul));
}

DynamicLoader::VoidFunc DynamicLoader::find_function(string const& symbol)
{
    // The ugly cast is needed because a void* (returned by dlsym()) is not compatible with a function pointer.
    VoidFunc func;
    *reinterpret_cast<void**>(&func) = find_variable(symbol);
    return func;
}

void* DynamicLoader::find_variable(string const& symbol)
{
    lock_guard<decltype(mutex_)> lock(mutex_);

    dlerror();  // Clear any existing error
    void* p = dlsym(handle_, symbol.c_str());
    char const* error = dlerror();
    if (error)
    {
        throw ResourceException(error);
    }
    return p;
}

string DynamicLoader::path() const
{
    lock_guard<decltype(mutex_)> lock(mutex_);

    return path_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
