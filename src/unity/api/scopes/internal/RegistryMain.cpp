/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */


#include <unity/api/scopes/internal/RegistryConfig.h>
#include <unity/api/scopes/internal/RegistryObject.h>
#include <unity/api/scopes/internal/RuntimeConfig.h>
#include <unity/api/scopes/internal/RuntimeImpl.h>
#include <unity/api/scopes/internal/ScopeProxyImpl.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

int
main(int, char*[])
{
    int exit_status = 0;

    try
    {
        // Get the identity of the registry from config. We need this to instantiate the run time.
        string registry_identity;
        {
            RuntimeConfig c;
            registry_identity = c.registry_identity();
        }
        RuntimeImpl::UPtr runtime = RuntimeImpl::create(registry_identity);

        // Instantiate and start middleware.

        MiddlewareBase::SPtr middleware;
        {
            RegistryConfig config(registry_identity, runtime->registry_configfile());
            string mw_kind = config.mw_kind();
            string mw_configfile = config.mw_configfile();

            // TODO: dodgy: why pass the identity a second time when the runtime already has it?
            middleware = runtime->factory()->create(registry_identity, mw_kind, mw_configfile);
        }
        //middleware->start();

        // Instantiate the registry implementation.
        RegistryObject::SPtr registry(new RegistryObject);

        // TODO: populate registry here

        // TODO: temporary hack for demonstratioun purposes. The actual scopes need to be retrieved from
        //       configuration instead.
        auto proxy = middleware->create_scope_proxy("scope-A", "uds -f /tmp/scope-A");
        registry->add(string("scope-A"), proxy);
        proxy = middleware->create_scope_proxy("scope-B", "uds -f /tmp/scope-B");
        registry->add("scope-B", proxy);
        proxy = middleware->create_scope_proxy("scope-C", "uds -f /tmp/scope-C");
        registry->add("scope-C", proxy);
        proxy = middleware->create_scope_proxy("scope-D", "uds -f /tmp/scope-D");
        registry->add("scope-D", proxy);

        // END TODO temporary hack

        // Add the registry implementation to the middleware. This causes
        // incoming requests to be dispatched to the implementation.
        middleware->add_registry_object(registry_identity, registry);

        // Wait until we are done.
        middleware->wait_for_shutdown();
        runtime->destroy();
    }
    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        exit_status = 1;
    }

    return exit_status;
}
