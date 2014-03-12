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

#ifndef UNITY_SCOPES_RUNTIME_H
#define UNITY_SCOPES_RUNTIME_H

#include <unity/scopes/ObjectProxyFwd.h>
#include <unity/scopes/RegistryProxyFwd.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace scopes
{

class ScopeBase;

namespace internal
{
class RuntimeImpl;
}

/**
\brief The main object for query originators to access the scopes run time.

All interactions with the scopes run time require a Runtime object to be instantiated first. The Runtime instance
controls the overall life cycle; once a Runtime instance goes out of scope, the application must not make further
calls on any instance obtained via the destroyed Runtime.

The application must instantiate a Runtime object only after `main()` is entered, and it must destroy the instance
before leaving `main()`; failure to do so results in undefined behavior.

Note that scope implementations do not need to instantiate a Runtime instance.
*/

class Runtime
{
public:
    /// @cond
    NONCOPYABLE(Runtime);
    UNITY_DEFINES_PTRS(Runtime);
    /// @endcond

    /**
    \brief Instantiates the scopes run time for a client with the given (optional) configuration file.

    The life time of the run time is under control of the caller. Letting the returned `unique_ptr` go out
    of scope shuts down the run time.

    You _must not_ create a Runtime instance until after `main()` is entered, and you _must_ destroy it
    before leaving `main()` (either by explicitly calling destroy(), or by letting the returned
    `unique_ptr` go out of scope). Failure to do so causes undefined behavior.

    \param configfile The path name of the configuration file to use.

    If configfile is the empty string, a default configuration is used.
    */
    // TODO: Update above to state what the default configuration is exactly
    static UPtr create(std::string const& configfile = "");

    /**
    \brief Shuts down the run time, reclaiming all associated resources.

    Calling destroy() is optional; the destructor implicitly calls destroy() if it was not called explicitly.
    However, no exceptions are thrown by the destructor. If you want to log or handle any exceptions during
    shutdown, call destroy() explicitly before letting the `unique_ptr` returned by create() go out of scope.
    */
    void destroy();

    /**
    \brief Returns a proxy to the Registry object.

    The returned proxy allows application code to interact with the registry, which provides access to the available
    scopes.
    */
    RegistryProxy registry() const;

    /**
    \brief Run a scope without going through the scope runner.

    This method is intended to run a scope that can not be loaded via the scope runner, such as those written in languages that can not be dynamically loaded.

    \param scope_base The scope implementation
    */
    void run_scope(ScopeBase *const scope_base);

    // TODO: Flesh out documentation for this, especially syntax.
    /**
    \brief Convert a string to a proxy.

    This method is intended for testing purposes. Do not use string_to_proxy() in production code!
    string_to_proxy() converts a string to a proxy. The returned proxy must be down-cast using
    `dynamic_pointer_cast` to the correct type before it can be used.

    \param s The string to convert into a proxy.
    \return The converted proxy. It is possible for the return value to be `nullptr`, but only
    if the passed string represents a null proxy. Otherwise, the return value is a non-null proxy,
    or an exception (for example, if the proxy string did not parse correctly, or if the proxy
    could not be instantiated due to incorrect values inside the string).
    */
    ObjectProxy string_to_proxy(std::string const& s) const;

    /**
    \brief Converts a proxy to a string.

    proxy_to_string() converts the passed proxy to a string. Note that it is typically easier
    to call the ObjectProxy::to_string() method to achieve the same thing. However, proxy_to_string()
    is needed in order to obtain a string for a null proxy (because it is not possible to invoke
    a member function on a null proxy).

    \param proxy The proxy to convert to a string.
    \return The string representation of the proxy.
    */
    std::string proxy_to_string(ObjectProxy const& proxy) const;

    /**
    \brief Destroys a Runtime instance.

    The destructor implicitly calls destroy() if the application code does not explicitly destroy the instance.
    You _must not_ permit a Runtime instance to persist beyond the end of `main()`; doing so has undefined behavior.
    */
    ~Runtime();

private:
    Runtime(std::string const& scope_id, std::string const& configfile); // Instantiation only via create()

    std::unique_ptr<internal::RuntimeImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
