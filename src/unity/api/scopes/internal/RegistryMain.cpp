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
#include <unity/api/scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>

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
        RuntimeImpl::UPtr runtime = RuntimeImpl::create("Registry", "RegistryMain.ini");

        string identity = runtime->registry_identity();
        RegistryConfig c(identity, runtime->registry_configfile());
        string mw_kind = c.mw_kind();
        string mw_endpoint = c.endpoint();
        string mw_configfile = c.mw_configfile();

        MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);

        // Instantiate the registry implementation.
        RegistryObject::SPtr registry(new RegistryObject);

        // TODO: populate registry here

        // TODO: temporary hack for demonstration purposes. The actual scopes need to be retrieved from
        //       configuration instead.
        auto proxy = middleware->create_scope_proxy("scope-A", "uds -f uds-scope-A-ctrl");
        registry->add(string("scope-A"), proxy);
        proxy = middleware->create_scope_proxy("scope-B", "uds -f uds-scope-B-ctrl");
        registry->add("scope-B", proxy);
        proxy = middleware->create_scope_proxy("scope-C", "uds -f uds-scope-C-ctrl");
        registry->add("scope-C", proxy);
        proxy = middleware->create_scope_proxy("scope-D", "uds -f uds-scope-D-ctrl");
        registry->add("scope-D", proxy);
        proxy = middleware->create_scope_proxy("scope-slow", "uds -f uds-scope-slow-ctrl");
        registry->add("scope-slow", proxy);

        // END TODO temporary hack

        // Add the registry implementation to the middleware. This causes
        // incoming requests to be dispatched to the implementation.
        middleware->add_registry_object(runtime->registry_identity(), registry);

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
