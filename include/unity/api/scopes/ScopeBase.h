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

#include <unity/util/NonCopyable.h>
#include <unity/api/scopes/Version.h>

#include <memory>

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

Scopes are provided to the Unity run time by providing them in a shared library (one library per scope).
Each scope must implement a class that derives from ScopeBase, for example:

~~~
#include <unity/api/scopes/ScopeBase.h>

class MyScope : public unity::api::scopes::ScopeBase
{
public:
    MyScope();
    virtual ~MyScope();

    virtual void initialize();
    virtual void finalize();

    virtual void start();
    virtual void stop();    // Optional
};
~~~

The derived class must provide implementations of the pure virtual methods initialize(), finalize(), and start();
In addition, the library must provide two functions with "C" linkage. The create function must return a pointer to
the derived instance, which the Unity run time passes back to the destroy function before the scope library is
unloaded. Typically, the create and destroy functions will simply call new and delete, respectively. (However,
there is no requirement that the derived class instance must be heap allocated.)

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

After the Unity run time has obtained a pointer to the class instance from the create function, it calls initialize(),
followed by start(), which passes the thread of control to the scope implementation. When the scope should complete
its activities, the run time calls stop(). Once stop() (which may be overridden by the derived class) returns,
the run time calls finalize(). The calls to initialize(), start(), and finalize() are made by the same thread.
(However, stop() is called by a different thread.)
Once finalize() completes, the run time calls the destroy function and then unloads the library from memory.

The implementation of the scope is free to use the thread that calls start() in any way it deems fit, such as
to run an event loop. If the implementation of start() does not need to use the thread calling start(),
the implementation can call run(). run() blocks the calling thread until the run time has called stop()
and stop() has completed, at which point run() unblocks the calling thread and returns.
The derived class can override stop() to, for example, arrange for its event loop to terminate and join with
any threads it has created.
*/

namespace internal
{
class ScopeBaseImpl;
}

class UNITY_API ScopeBase : private util::NonCopyable
{
public:
    /// @cond
    virtual ~ScopeBase();
    /// @endcond

    /**
    Called by the Unity run time after loading the scope's library. If initialize() throws an exception,
    start() and finalize() will _not_ be called.
    */
    virtual void initialize() = 0;

    /**
    Called by the Unity run time after stop() completes. Exceptions from finalize() are ignored.
    */
    virtual void finalize() = 0;

    /**
    Called by the Unity run time after initialize() completes. If start() throws an exception, stop()
    will _not_ be called (but finalize() _will_ be called).
    */
    virtual void start() = 0;

    /**
    Called by the Unity run time when the scope should shut down. Exceptions from stop() are ignored.
    */
    virtual void stop();

    /**
    This method suspends the calling thread, so run() blocks the caller. The call completes after the Unity
    run time has called stop() and stop() has returned.
    */
    void run() noexcept;

    /**
    This method returns the version information for the scopes API that the scope was compiled with.
    */
    void compiled_version(int& v_major, int& v_minor, int& v_micro) noexcept;

    /**
    This method returns the version information for the scopes API that the scope was linked with.
    */
    void runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept;

protected:
    /// @cond
    ScopeBase();
    /// @endcond

private:
    std::unique_ptr<internal::ScopeBaseImpl> p_;
};

// We inline this function so we are guaranteed to be returned the version of the scopes library that the derived
// class was compiled with.

inline
void ScopeBase::compiled_version(int& v_major, int& v_minor, int& v_micro) noexcept
{
    v_major = UNITY_SCOPES_VERSION_MAJOR;
    v_minor = UNITY_SCOPES_VERSION_MINOR;
    v_micro = UNITY_SCOPES_VERSION_MICRO;
}

} // namespace scopes

} // namespace api

} // namespace unity

/**
Expands to the identifier of the scope creation function. @hideinitializer
*/
#define UNITY_API_SCOPE_CREATE_FUNCTION unity_api_scope_create

/**
Expands to the identifier of the scope destroy function. @hideinitializer
*/
#define UNITY_API_SCOPE_DESTROY_FUNCTION unity_api_scope_destroy

/// @cond
extern "C" unity::api::scopes::ScopeBase* UNITY_API_SCOPE_CREATE_FUNCTION();
extern "C" void UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase*);
/// @endcond

#endif
