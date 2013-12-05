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

#ifndef UNITY_API_SCOPES_RUNTIME_H
#define UNITY_API_SCOPES_RUNTIME_H

#include <scopes/RegistryProxyFwd.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{
class RuntimeImpl;
}

/**
\brief The main object to access the scopes run time.

All interactions with the scopes run time require a Runtime object to be instantiated first. The Runtime instance
controls the overall life cycle; once a Runtime instance goes out of scope, the application must not make further
calls on any instance obtained via the destroyed Runtime.

The application must instantiate a Runtime object only after `main()` is entered, and it must destroy the instance
before leaving `main()`; failure to do so results in undefined behavior.
*/

class UNITY_API Runtime : private util::NonCopyable
{
public:
    /// @cond
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

    \param scope_name The name of the scope
    \param scope_base The scope implementation
    */
    void run_scope(std::string const& scope_name, ScopeBase *const scope_base);

    /**
    \brief Destroys a Runtime instance.

    The destructor implicitly calls destroy() if the application code does not explicitly destroy the instance.
    You _must not_ permit a Runtime instance to persist beyond the end of `main()`; doing so has undefined behavior.
    */
    ~Runtime() noexcept;

private:
    Runtime(std::string const& configfile); // Instantiation only via create()

    std::unique_ptr<internal::RuntimeImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
