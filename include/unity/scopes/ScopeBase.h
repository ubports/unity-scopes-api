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

#ifndef UNITY_SCOPES_SCOPEBASE_H
#define UNITY_SCOPES_SCOPEBASE_H

#include <unity/scopes/AbstractScopeBase.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/Version.h>

/**
\brief Expands to the identifier of the scope create function. @hideinitializer
*/
#define UNITY_SCOPE_CREATE_FUNCTION UNITY_SCOPES_VERSIONED_CREATE_SYM

/**
\brief Expands to the identifier of the scope destroy function. @hideinitializer
*/
#define UNITY_SCOPE_DESTROY_FUNCTION unity_scope_destroy

// Convenience definitions for looking up the create and destroy functions in the symbol table
// of a dynamically loaded scope.
// UNITY_SCOPE_CREATE_SYMSTR and UNITY_SCOPE_DESTROY_SYMSTR expand to a string literal containing the name
// of the create and destroy function, respectively.

#ifndef DOXYGEN_SKIP
#    define UNITY_SCOPE_STR(sym) #sym
#    define UNITY_SCOPE_XSTR(sym) UNITY_SCOPE_STR(sym)
#endif

/**
\brief Expands to the identifier of the scope create function as a string literal. @hideinitializer
*/
#define UNITY_SCOPE_CREATE_SYMSTR UNITY_SCOPE_XSTR(UNITY_SCOPE_CREATE_FUNCTION)

/**
\brief Expands to the identifier of the scope destroy function as a string literal. @hideinitializer
*/
#define UNITY_SCOPE_DESTROY_SYMSTR UNITY_SCOPE_XSTR(UNITY_SCOPE_DESTROY_FUNCTION)

namespace unity
{

namespace scopes
{

class CannedQuery;

namespace internal
{

class ScopeBaseImpl;
class RuntimeImpl;

}

namespace testing
{

struct TypedScopeFixtureHelper;

}

/**
\file ScopeBase.h
\class ScopeBase
\brief Base class for a scope implementation.

Scopes are accessed by the scopes run time as a shared library (one library per scope).
Each scope must implement a class that derives from ScopeBase, for example:

~~~
#include <unity/scopes/ScopeBase.h>

class MyScope : public unity::scopes::ScopeBase
{
public:
    MyScope();
    virtual ~MyScope();

    virtual void start(std::string const& scope_id);   // Optional
    virtual void stop();                               // Optional
    virtual void run();                                // Optional
    // ...
};
~~~

In addition, the library must provide two functions with "C" linkage:
 - a create function that must return a pointer to the derived instance
 - a destroy function that is passed the pointer returned by the create function

Typically, the create and destroy functions will simply call `new` and `delete`, respectively. (However,
there is no requirement that the derived class instance must be heap-allocated.)
If the create function throws an exception, the destroy function will not be called. If the create function returns
NULL, the destroy function _will_ be called with NULL as its argument.

Rather than hard-coding the names of the functions, use the #UNITY_SCOPE_CREATE_FUNCTION and
#UNITY_SCOPE_DESTROY_FUNCTION macros, for example:

~~~
unity::scopes::ScopeBase*
UNITY_SCOPE_CREATE_FUNCTION()
{
    return new MyScope; // Example only, heap-allocation is not mandatory
}

void
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope)
{
    delete scope;       // Example only, heap-allocation is not mandatory
}
~~~

After the scopes run time has obtained a pointer to the class instance from the create function, it calls start(),
which allows the scope to intialize itself. This is followed by a call to run(). The call to run() is made by
a separate thread; its only purpose is to pass a thread of control to the scope, for example, to run an event loop.
When the scope should complete its activities, the run time calls stop(). The calls to the create function, start(),
stop(), and the destroy function) are made by the same thread.

The scope implementation, if it does not return from run(), is expected to return from run() in response to a
call to stop() in a timely manner.
*/

class ScopeBase : public AbstractScopeBase
{
public:
    /// @cond
    NONCOPYABLE(ScopeBase);
    virtual ~ScopeBase();
    /// @endcond

    /**
    \brief Called by the scopes run time after the create function completes.

    If start() throws an exception, stop() will _not_ be called.

    The call to start() is made by the same thread that calls the create function.

    \param scope_id The name of the scope as defined by the scope's configuration file.
    */
    virtual void start(std::string const& scope_id);

    /**
    \brief Called by the scopes run time when the scope should shut down.

    A scope should deallocate as many resources as possible when stop() is called, for example,
    deallocate any caches and close network connections. In addition, if the scope implements run()
    and did not return from run(), it must return from run() in response to the call to stop().

    Exceptions from stop() are ignored.

    The call to stop() is made by the same thread that calls the create function and start().
    */
    virtual void stop();

    /**
    \brief Called by the scopes run time after it has called start() to hand a thread of control to the scope.

    run() passes a thread of control to the scope to do with as it sees fit, for example, to run an event loop.
    During finalization, the scopes run time joins with the thread that called run(). This means that, if
    the scope implementation does not return from run(), it is expected to arrange for run() to complete
    in a timely manner in response to a call to stop(). Failure to do so will cause deadlock during finalization.

    If run() throws an exception, the run time handles the exception and calls stop() in response.
    */
    virtual void run();

    /**
    \brief Called by the scopes run time when a scope needs to instantiate a query.

    This method must return an instance that is derived from `QueryBase`. The implementation
    of this method must return in a timely manner, that is, it should perform only minimal
    initialization that is guaranteed to complete quickly. The call to search() is made
    by an arbitrary thread.
    \param query The query string to be executed by the returned object instance.
    \param metadata additional data sent by the client.
    \return The query instance.
    */
    virtual SearchQueryBase::UPtr search(CannedQuery const& query, SearchMetadata const& metadata) = 0;

    /**
    \brief Called by the scopes run time when a scope needs to respond to a result activation request.

    This method must return an instance that is derived from `ActivationQueryBase`. The implementation
    of this method must return in a timely manner, that is, it should perform only minimal
    initialization that is guaranteed to complete quickly. The call to activate() is made
    by an arbitrary thread.
    The default implementation returns an instance of ActivationQueryBase that responds with
    ActivationResponse::Status::NotHandled.
    \param result The result that should be activated.
    \param metadata additional data sent by the client.
    \return The activation instance.
    */
    virtual ActivationQueryBase::UPtr activate(Result const& result, ActionMetadata const& metadata);

    /**
    \brief Invoked when a scope is requested to handle a preview action.

    This method must return an instance that is derived from `ActivationQueryBase`. The implementation
    of this method must return in a timely manner, that is, it should perform only minimal
    initialization that is guaranteed to complete quickly. The call to activate() is made
    by an arbitrary thread.
    The default implementation returns an instance of ActivationQueryBase that responds with
    ActivationResponse::Status::NotHandled.
    \param result The result that was previewed.
    \param metadata Additional data sent by client.
    \param widget_id The identifier of the 'actions' widget of the activated action.
    \param action_id The identifier of the action that was activated.
    \return The activation instance.
    */
    virtual ActivationQueryBase::UPtr perform_action(Result const& result,
                                                     ActionMetadata const& metadata,
                                                     std::string const& widget_id,
                                                     std::string const& action_id);

    /**
    \brief Invoked when a scope is requested to create a preview for a particular result.

    This method must return an instance that is derived from `PreviewQueryBase`. The implementation
    of this method must return in a timely manner, that is, it should perform only minimal
    initialization that is guaranteed to complete quickly. The call to activate() is made
    by an arbitrary thread.
    \param result The result that should be previewed.
    \param metadata Additional data sent by the client.
    \return The preview instance.
    */
    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) = 0;

    /**
    \brief Returns the version information for the scopes API that the scope was linked with.
    */
    static void runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept;

    /**
    \brief Returns the directory that stores the scope's configuration files and shared library.

    \note The scope directory is available only after this ScopeBase is instantiated; do not
    call this method from the constructor!

    \return The scope's configuration directory.
    \throws LogicException if called from the constructor of this instance.
    */
    virtual std::string scope_directory() const final;

    /**
    \brief Returns a directory that is (exclusively) writable for the scope.

    This directory allows scopes to store persistent information, such
    as cached content or similar.

    \note The cache directory is available only after this ScopeBase is instantiated; do not
    call this method from the constructor!

    \return The root directory of a filesystem sub-tree that is writable for the scope.
    \throws LogicException if called from the constructor of this instance.
    */
    virtual std::string cache_directory() const final;

    /**
    \brief Returns a tmp directory that is (exclusively) writable for the scope.

    This directory is periodically cleaned of unused files. The exact amount of time
    may vary, but is on the order of a few hours.

    \note The tmp directory is available only after this ScopeBase is instantiated; do not
    call this method from the constructor!

    \return A directory for temporary files.
    \throws LogicException if called from the constructor of this instance.
    */
    virtual std::string tmp_directory() const final;

    /**
    \brief Returns the proxy to the registry.

    \note The registry proxy is available only after this ScopeBase is instantiated; do not
    call this method from the constructor!

    \return The proxy to the registry.
    \throws LogicException if called from the constructor of this instance.
    */
    virtual unity::scopes::RegistryProxy registry() const final;

    /**
    \brief Returns a dictionary with the scope's current settings.

    Instead of storing the return value, it is preferable to call settings()
    each time your implementation requires a settings value. This ensures
    that, if a user changes settings while the scope is running, the new settings
    take effect with the next query.

    \note The settings are available only after this ScopeBase is instantiated; do not
    call this method from the constructor!

    \return The scope's current settings.
    \throws LogicException if called from the constructor of this instance.
    */
    virtual VariantMap settings() const final;

protected:
    /// @cond
    ScopeBase();
private:
    std::unique_ptr<internal::ScopeBaseImpl> p;

    friend class internal::RuntimeImpl;
    friend class internal::ScopeObject;
    friend struct testing::TypedScopeFixtureHelper;
    /// @endcond
};

} // namespace scopes

} // namespace unity

/**
\brief The function called by the scopes run time to initialize the scope.
It must return a pointer to an instance derived from `ScopeBase`. The returned
instance need not be heap-allocated, but must remain in scope until the
destroy function is called by the scopes run time.

If this function throws an exception, the destroy function will _not_ be called. If this function returns NULL,
the destroy function _will_ be called with NULL as its argument.

\note The only purpose of the create function is to return the an instance.
Do not do anything in the implementation that might block, and do
not attempt to call any methods on `ScopeBase` from the constructor.

\return The pointer to the ScopeBase instance.
*/
extern "C" unity::scopes::ScopeBase* UNITY_SCOPE_CREATE_FUNCTION();

/**
\brief The function called by the scopes run time to finalize the scope.
The passed pointer is the pointer that was returned by the create function.

Exceptions thrown by the destroy function are ignored.

\param p The pointer to the instance to be destroyed (previously returned by the create function).
*/
extern "C" void UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* p);

namespace unity
{

namespace scopes
{

/**
\brief Convenience typedef for the create function pointer.
*/
typedef decltype(&UNITY_SCOPE_CREATE_FUNCTION) CreateFunction;

/**
\brief Convenience typedef for the destroy function pointer.
*/
typedef decltype(&UNITY_SCOPE_DESTROY_FUNCTION) DestroyFunction;

} // namespace scopes

} // namespace unity

#endif
