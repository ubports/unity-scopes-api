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

#include <unity/api/scopes/internal/DynamicLoader.h>
#include <unity/UnityExceptions.h>

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

DynamicLoader::DynamicLoader(string const& path, Binding b, Unload ul)
    : unload_(ul)
{
    if ((handle_ = dlopen(path.c_str(), b == Binding::lazy ? RTLD_LAZY : RTLD_NOW)) == nullptr)
    {
        throw ResourceException(dlerror());
    }
}

DynamicLoader::~DynamicLoader() noexcept
{
    if (unload_ == Unload::automatic)
    {
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
    *(void **)&func = find_variable(symbol);
    return func;
}

void* DynamicLoader::find_variable(string const& symbol)
{
    dlerror();  // Clear any existing error
    void* p = dlsym(handle_, symbol.c_str());
    char const* error = dlerror();
    if (error)
    {
        throw ResourceException(error);
    }
    return p;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
