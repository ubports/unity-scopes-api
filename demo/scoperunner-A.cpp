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

#include <unity/api/scopes/internal/RuntimeImpl.h>
#include <unity/api/scopes/internal/ScopeLoader.h>
#include <unity/api/scopes/internal/ScopeObject.h>
#include <unity/UnityExceptions.h>

#include <iostream>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

int
main(int, char**)
{
    // TODO: the hard-wired strings here need to be read from config.

    string scope_name = string("scope-A");

    try
    {
        RuntimeImpl::UPtr rt = RuntimeImpl::create(scope_name);
        ScopeLoader::SPtr loader = ScopeLoader::load(scope_name, "lib" + scope_name + ".so", rt->registry());
        loader->start();
        MiddlewareBase::SPtr mw = rt->factory()->create(scope_name, "Zmq", "Zmq.Config");
        ScopeObject::SPtr scope(new ScopeObject(rt.get(), loader->scope_base()));
        mw->add_scope_object(scope_name, scope);
        mw->wait_for_shutdown();
    }
    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        return 1;
    }

    return 0;
}
