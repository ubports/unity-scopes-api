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

#ifndef UNITY_API_SCOPES_SCOPEBASE_H
#define UNITY_API_SCOPES_SCOPEBASE_H

#include <unity/SymbolExport.h>
#include <unity/util/NonCopyable.h>
#include <unity/api/scopes/RegistryProxy.h>
#include <unity/api/scopes/ReplyProxy.h>
#include <unity/api/scopes/Version.h>

#include <memory>
#include <iostream> // TODO:REMOVE THIS

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

    virtual int start();
    virtual void stop();
    virtual void run();
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
When the scope should complete its activities, the run time calls stop(). The calls to the create function, start(),
stop(), and the destroy function) are made by the same thread. The call to run() is made by a _different_ thread.
*/

class UNITY_API ScopeBase : private util::NonCopyable
{
public:
    /// @cond
    virtual ~ScopeBase();
    /// @endcond

    /**
    \brief This value must be returned from the start() method.
    */
    static constexpr int VERSION = UNITY_SCOPES_VERSION_MAJOR;

    /**
    \brief Called by the Unity run time after initialize() completes.
    If start() throws an exception, stop() will _not_ be called.

    The call to start() is made by the same thread that calls the create function.

    \return Any return value other than SCOPES_MAJOR_VERSION will cause the Unity run time
    to refuse to load the scope. The return value is used to ensure that the shared library
    containing the scope is ABI compatible with the Unity scopes run time.
    */
    virtual int start(RegistryProxy::SPtr const& registry) = 0;

    /**
    \brief Called by the Unity run time when the scope should shut down.
    A scope should deallocate as many resources as possible when stop() is called, for example,
    deallocate any caches and close network connections.

    Exceptions from stop() are ignored.

    The call to stop() is made by the same thread that calls the create function and start().
    */
    virtual void stop() = 0;

    /**
    \brief Called by the Unity run time to hand a thread of control to the scope.
    run() passes a thread of control to the scope to do with as it sees fit, for example, to run an event loop.

    If run() throws an exception, stop() _will_ be called.

    The call to run() is made by a separate thread (not the thread that calls the create function and start()).
    */
    virtual void run() = 0;

    /**
    \brief Called by the Unity run time when scope should process a query.
    TODO: complete documentation.

    Calls to query() are made by an arbitrary thread. If the scope is configured to run single-threaded, all calls
    to query() are made by the same thread. If the scope is configured to run multi-threaded, each call to query()
    is made by an arbitrary thread, and several calls to query() may be made concurrently.
    */
    virtual void query(std::string const& q, ReplyProxy::SPtr const& reply) = 0;

    /**
    \brief This method returns the version information for the scopes API that the scope was linked with.
    */
    void runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept;

protected:
    /// @cond
    ScopeBase();
    /// @endcond
};

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
