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


#include <unity/api/scopes/internal/MiddlewareFactory.h>
#include <unity/api/scopes/internal/RegistryConfig.h>
#include <unity/api/scopes/internal/RegistryObject.h>
#include <unity/api/scopes/internal/RuntimeConfig.h>
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
        MiddlewareBase::SPtr middleware;
        string registry_identity;
        {
            // Get the main config.
            string registry_configfile;
            string registry_middleware;
            string factory_configfile;
            {
                RuntimeConfig c;
                registry_identity = c.registry_identity();
                registry_configfile = c.registry_configfile();
                factory_configfile = c.factory_configfile();
            }

            string registry_kind;
            string registry_endpoint;
            string registry_mw_configfile;
            {
                RegistryConfig c(registry_identity, registry_configfile);
                registry_kind = c.mw_kind();
                registry_endpoint = c.endpoint();
                registry_mw_configfile = c.mw_configfile();
            }

            // Instantiate the middleware factory
            MiddlewareFactory factory(factory_configfile);

            cerr << "registry: creating middleware: " << registry_mw_configfile << endl;
            // TODO: dodgy: why pass the identity a second time when the runtime already has it?
            middleware = factory.create(registry_identity, registry_kind, registry_mw_configfile);
            middleware->start();
        }

        // Instantiate the registry implementation.
        RegistryObject::SPtr registry(new RegistryObject);

        // TODO: populate registry here

        // TODO: temporary hack for demonstration purposes. The actual scopes need to be retrieved from
        //       configuration instead.
        auto proxy = middleware->create_scope_proxy("scope-A", "uds -f sockets/scope-A-ctrl");
        registry->add(string("scope-A"), proxy);
        proxy = middleware->create_scope_proxy("scope-B", "uds -f sockets/scope-B-ctrl");
        registry->add("scope-B", proxy);
        proxy = middleware->create_scope_proxy("scope-C", "uds -f sockets/scope-C-ctrl");
        registry->add("scope-C", proxy);
        proxy = middleware->create_scope_proxy("scope-D", "uds -f sockets/scope-D-ctrl");
        registry->add("scope-D", proxy);

        // END TODO temporary hack

        // Add the registry implementation to the middleware. This causes
        // incoming requests to be dispatched to the implementation.
        middleware->add_registry_object(registry_identity, registry);

        // Wait until we are done.
        middleware->wait_for_shutdown();
    }
    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        exit_status = 1;
    }

    return exit_status;
}
