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

#ifndef UNITY_API_SCOPES_SCOPEBASE_H
#define UNITY_API_SCOPES_SCOPEBASE_H

#include <unity/SymbolExport.h>
#include <unity/util/NonCopyable.h>
#include <unity/api/scopes/Version.h>

#include <memory>

/**
\brief Expands to the identifier of the scope create function. @hideinitializer
*/
#define UNITY_API_SCOPE_CREATE_FUNCTION unity_api_scope_create

/**
\brief Expands to the identifier of the scope destroy function. @hideinitializer
*/
#define UNITY_API_SCOPE_DESTROY_FUNCTION unity_api_scope_destroy

// Convenience definitions for looking up the create and destroy functions in the symbol table
// of a dynamically loaded scope.
// UNITY_API_SCOPE_CREATE_SYMSTR and UNITY_API_SCOPE_DESTROY_SYMSTR expand to a string literal containing the name
// of the create and destroy function, respectively.

#ifndef DOXYGEN_SKIP
#    define UNITY_API_SCOPE_STR(sym) #sym
#    define UNITY_API_SCOPE_XSTR(sym) UNITY_API_SCOPE_STR(sym)
#endif

/**
\brief Expands to the identifier of the scope create function as a string literal. @hideinitializer
*/
#define UNITY_API_SCOPE_CREATE_SYMSTR UNITY_API_SCOPE_XSTR(UNITY_API_SCOPE_CREATE_FUNCTION)

/**
\brief Expands to the identifier of the scope destroy function as a string literal. @hideinitializer
*/
#define UNITY_API_SCOPE_DESTROY_SYMSTR UNITY_API_SCOPE_XSTR(UNITY_API_SCOPE_DESTROY_FUNCTION)

namespace unity
{

namespace api
{

namespace scopes
{

/**
\file ScopeBase.h
\class ScopeBase
\brief Base class for a scope implementation.

Scopes are accessed by the Unity run time as a shared library (one library per scope).
Each scope must implement a class that derives from ScopeBase, for example:

~~~
#include <unity/api/scopes/ScopeBase.h>

class MyScope : public unity::api::scopes::ScopeBase
{
public:
    MyScope();
    virtual ~MyScope();

    virtual void start();
    virtual void stop();
};
~~~

The derived class must provide implementations of the pure virtual methods start()
and stop(). In addition, the library must provide two functions with "C" linkage:
 - a create function that must return a pointer to the derived instance
 - a destroy function that is passed the pointer returned by the create function

Typically, the create and destroy functions will simply call new and delete, respectively. (However,
there is no requirement that the derived class instance must be heap allocated.)
If the create function throws an exception, the destroy function will not be called. If the create function returns
NULL, the destroy function will be called with NULL as its argument.

Rather than hard-coding the names of the functions, use the #UNITY_API_SCOPE_CREATE_FUNCTION and
#UNITY_API_SCOPE_DESTROY_FUNCTION macros, for example:

~~~
unity::api::scopes::ScopeBase*
UNITY_API_SCOPE_CREATE_FUNCTION()
{
    return new MyScope; // Example only, heap-allocation is not mandatory
}

void
UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope)
{
    delete scope;       // Example only, heap-allocation is not mandatory
}
~~~

After the Unity run time has obtained a pointer to the class instance from the create function, it calls start(),
which allows the scope to intialize itself. Once start() returns, incoming query requests are dispatched to the scope.
When the scope should complete its activities, the run time calls stop(). The calls to start() and
stop() are made by the same thread.
*/

namespace internal
{
class ScopeBaseImpl;
}

class UNITY_API ScopeBase : private util::NonCopyable
{
public:
    /// @cond
    virtual ~ScopeBase() {}
    /// @endcond

    /**
    \brief Called by the Unity run time after initialize() completes.
    If start() throws an exception, stop() will _not_ be called (but finalize() _will_ be called).
    */
    virtual void start() = 0;

    /**
    \brief Called by the Unity run time when the scope should shut down.
    A scope should deallocate as many resources as possible when stop() is called, for example,
    deallocate any caches and close network connections.

    Exceptions from stop() are ignored.
    */
    virtual void stop() = 0;

    /**
    \brief Called by the Unity run time when scope should process a query.
    The passed functor can be used to push the results, either synchronously, while query() is still running,
    or asynchronously, after query() has returned.
    */
    virtual void query(std::string const& q) = 0;

    /**
    \brief This method returns the version information for the scopes API that the scope was compiled with.
    */
    void compiled_version(int& v_major, int& v_minor, int& v_micro) noexcept;

    /**
    \brief This method returns the version information for the scopes API that the scope was linked with.
    */
    void runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept;

protected:
    /// @cond
    ScopeBase() = default;
    /// @endcond
};

// We inline this function so we are guaranteed to be returned the version of the scopes library that the derived
// class was compiled with.

inline
void
ScopeBase::
compiled_version(int& v_major, int& v_minor, int& v_micro) noexcept
{
    v_major = UNITY_SCOPES_VERSION_MAJOR;
    v_minor = UNITY_SCOPES_VERSION_MINOR;
    v_micro = UNITY_SCOPES_VERSION_MICRO;
}

} // namespace scopes

} // namespace api

} // namespace unity

/**
\brief The function called by the Unity run time to initialize the scope.
It must return a pointer to a ScopeBase instance. The returned instance need not be heap-allocated.

If this function throws an exception, the destroy function will _not_ be called. If this function returns NULL,
the destroy function _will_be called with NULL as its argument.
*/
extern "C" unity::api::scopes::ScopeBase* UNITY_API_SCOPE_CREATE_FUNCTION();

/**
\brief The function called by the Unity run time to finalize the scope.
The passed pointer is the pointer that was returned by the create function.

Exceptions thrown by the destroy function are ignored.
*/
extern "C" void UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase*);

namespace unity
{

namespace api
{

namespace scopes
{

/**
\brief Convenience typedef for the create function pointer.
*/
typedef decltype(&UNITY_API_SCOPE_CREATE_FUNCTION) CreateFunction;

/**
\brief Convenience typedef for the destroy function pointer.
*/
typedef decltype(&UNITY_API_SCOPE_DESTROY_FUNCTION) DestroyFunction;

} // namespace scopes

} // namespace api

} // namespace unity

#endif
