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

#include <scopes/internal/RuntimeImpl.h>
#include <scopes/internal/ScopeLoader.h>
#include <scopes/internal/ScopeObject.h>
#include <unity/UnityExceptions.h>

#include <iostream>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

int
main(int, char**)
{
    // TODO: the hard-wired strings here need to be read from config.

    string scope_nameB = string("scope-B");
    string scope_nameC = string("scope-C");

    try
    {
        RuntimeImpl::UPtr rtB = RuntimeImpl::create(scope_nameB);
        MiddlewareBase::SPtr mwB = rtB->factory()->create(rtB->scope_name(), "Zmq", "Zmq.Config");
        ScopeLoader::SPtr loaderB = ScopeLoader::load(rtB->scope_name(), "lib" + rtB->scope_name() + ".so",
                                                      rtB->registry());
        loaderB->start();
        ScopeObject::SPtr scopeB(new ScopeObject(rtB.get(), loaderB->scope_base()));
        mwB->add_scope_object(rtB->scope_name(), scopeB);

        RuntimeImpl::UPtr rtC = RuntimeImpl::create(scope_nameC);
        MiddlewareBase::SPtr mwC = rtC->factory()->create(rtC->scope_name(), "Zmq", "Zmq.Config");
        ScopeLoader::SPtr loaderC = ScopeLoader::load(rtC->scope_name(), "lib" + rtC->scope_name() + ".so",
                                                      rtC->registry());
        loaderC->start();
        ScopeObject::SPtr scopeC(new ScopeObject(rtC.get(), loaderC->scope_base()));
        mwC->add_scope_object(rtC->scope_name(), scopeC);

        mwB->wait_for_shutdown();
        mwC->wait_for_shutdown();
    }

    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        return 1;
    }

    return 0;
}
