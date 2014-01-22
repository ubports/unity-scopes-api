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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>
#include <libgen.h>

using namespace std;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

char const* prog_name;

namespace
{

void error(string const& msg)
{
    assert(!msg.empty());
    cerr << prog_name << ": " << msg << endl;
}

} // namespace

int
main(int argc, char* argv[])
{
    prog_name = basename(argv[0]);

    char const* const config_file = argc > 1 ? argv[1] : "";

    int exit_status = 1;

    try
    {
        RuntimeImpl::UPtr runtime = RuntimeImpl::create("SSRegistry", config_file);

        string identity = runtime->registry_identity();

        RegistryConfig c(identity, runtime->registry_configfile());
        string mw_kind = c.mw_kind();

        MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);

        SSRegistryObject::SPtr registry(new SSRegistryObject(middleware));

        // Add the registry to the middleware
        ///! middleware->add_registry_object(runtime->registry_identity(), registry);

        // Wait until shutdown
        middleware->wait_for_shutdown();
        exit_status = 0;
    }
    catch (unity::Exception const& e)
    {
        error(e.to_string());
    }
    catch (std::exception const& e)
    {
        error(e.what());
    }
    catch (string const& e)
    {
        error("fatal error: " + e);
    }
    catch (char const* e)
    {
        error(string("fatal error: ") + e);
    }
    catch (...)
    {
        error("terminated due to unknown exception");
    }

    return exit_status;
}
