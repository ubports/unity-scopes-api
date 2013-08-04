/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/DynamicLoader.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <dlfcn.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

DynamicLoader::LibraryHandles DynamicLoader::m_handles;
mutex DynamicLoader::m_mutex;

DynamicLoader::DynamicLoader(string const& path, Binding b, Unload ul) :
    m_path(path),
    m_handle(nullptr),
    m_unload(ul)
{
    // Prevent concurrent access to m_handles and guard against concurrent
    // calls to dlopen()/dlclose(), which are not thread-safe.
    lock_guard<decltype(m_mutex)> lock(m_mutex);

    // If the caller doesn't want dlclose() to be called, we look whether we've loaded
    // this library previously and, if so, re-use its handle.
    if (ul == Unload::noclose)
    {
        auto it = m_handles.find(path);
        if (it != m_handles.end())
        {
            m_handle = it->second;
            assert(m_handle);
            return;
        }
    }

    assert(ul == Unload::automatic || m_handle == nullptr);

    if ((m_handle = dlopen(path.c_str(), b == Binding::lazy ? RTLD_LAZY : RTLD_NOW)) == nullptr)
    {
        throw ResourceException(dlerror());
    }

    if (ul == Unload::noclose)
    {
        m_handles[path] = m_handle; // Remember the library for re-use.
    }
}

DynamicLoader::~DynamicLoader() noexcept
{
    if (m_unload == Unload::automatic)
    {
        assert(m_handle);
        lock_guard<decltype(m_mutex)> lock(m_mutex);
        dlclose(m_handle);
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
    *(void **)&func = find_variable(symbol);
    return func;
}

void* DynamicLoader::find_variable(string const& symbol)
{
    dlerror();  // Clear any existing error
    void* p = dlsym(m_handle, symbol.c_str());
    char const* error = dlerror();
    if (error)
    {
        throw ResourceException(error);
    }
    return p;
}

string DynamicLoader::path() const
{
    return m_path;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
