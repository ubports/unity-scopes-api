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

#ifndef UNITY_API_SCOPES_INTERNAL_DYNAMICLOADER_H
#define UNITY_API_SCOPES_INTERNAL_DYNAMICLOADER_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <map>
#include <mutex>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

//
// Simple helper class that wraps dlopen(), dlsym() and dlclose().
//
// To use it, call create() with a path. If the supplied path contains a slash, the library will be loaded
// from the specified location; otherwise, the usual lookup performed by dlopen() applies.
//
// The Binding flag determines whether unresolved external functions are diagnosed only when a function
// requiring an unresolved symbol is looked for (lazy binding) or whether all functions are bound
// immediately on loading. (References to variables are always bound immediately, regardless of the flag.)
//
// By default, the destructor calls dlclose(). If you don't want the destructor to call dlclose(), pass
// Unload::noclose to create(). This will leave the library loaded in memory, even after this instance is destroyed.
// Note that, if a thread is still executing inside the library when it is unloaded, or if the code elsewhere
// still holds a reference to something in the library and uses the reference once the library is unloaded,
// the behavior is undefined.
//
// To resolve a symbol in the library, call find_function() or find_variable().
// If the symbol can't be located, these calls throw unity::ResourceException.
//
// find_function() returns a pointer to a function returning void and accepting no arguments.
// find_variable() returns a void*.
// You are responsible for casting the return value to the correct type. Dereferencing the returned pointer
// after casting it to the wrong type has undefined behavior.
//
// Any other errors from the underlying dlopen(), dlsym(), and dlclose() functions throw unity::ResourceException.
//
// Typical use pattern:
//
//  DynamicLoader::UPtr dl = DynamicLoader::create("libexample.so");
//  // ...
//  DynamicLoader::VoidFunc f = dl->find_function("some_function");
//  void (*int_func)(int) = reinterpret_cast<void (*)(int)>(f);
//  int_func(5);
//

class UNITY_API DynamicLoader
{
public:
    NONCOPYABLE(DynamicLoader)

    UNITY_DEFINES_PTRS(DynamicLoader);

    enum class Binding
    {
        lazy, now
    };
    enum class Unload
    {
        automatic, noclose
    };

    static UPtr create(std::string const& path, Binding b = Binding::lazy, Unload ul = Unload::automatic);

    typedef void (*VoidFunc)();
    VoidFunc find_function(std::string const& symbol);

    void* find_variable(std::string const& symbol);

    std::string path() const;

    ~DynamicLoader() noexcept;

private:
    DynamicLoader(std::string const& path, Binding b, Unload ul);

    std::string path_;
    void* handle_;
    Unload unload_;

    typedef std::map<std::string, void*> LibraryHandles;
    static LibraryHandles handles_;                        // Handles of libraries that will not be unloaded
    static std::mutex mutex_;                              // Protects m_handle_map
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
