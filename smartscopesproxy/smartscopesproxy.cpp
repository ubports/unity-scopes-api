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
#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>

using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

static void error(std::string const& msg)
{
  assert(!msg.empty());
  std::cerr << "smartscopesproxy: " << msg << std::endl;
}

int main(int argc, char* argv[])
{
  int exit_status = 1;

  std::string server_url_env;
  if (argc > 1)
  {
    server_url_env = "SMART_SCOPES_SERVER=" + std::string(argv[1]);
    ::putenv(const_cast<char*>(server_url_env.c_str()));
  }

  try
  {
    // Instantiate a runtime
    RuntimeImpl::UPtr runtime = RuntimeImpl::create("SSRegistry", SS_RUNTIME_PATH);

    // Get middleware handle
    std::string identity = runtime->registry_identity();

    RegistryConfig c(identity, runtime->registry_configfile());
    std::string mw_kind = c.mw_kind();

    MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);

    // Instantiate a SS registry
    SSRegistryObject::SPtr registry(new SSRegistryObject(middleware));

    // Add the SS registry to the middleware
    middleware->add_registry_object(runtime->registry_identity(), registry);

    // Instantiate a SS scope
    SSScopeObject::UPtr scope = SSScopeObject::UPtr(new SSScopeObject(runtime.get()));

    // Add the SS scope to the middleware
    middleware->add_dflt_scope_object(std::move(scope));

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
  catch (std::string const& e)
  {
    error("fatal error: " + e);
  }
  catch (char const* e)
  {
    error(std::string("fatal error: ") + e);
  }
  catch (...)
  {
    error("terminated due to unknown exception");
  }

  return exit_status;
}
