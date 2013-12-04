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

#include <future>

#include <scopes/Runtime.h>
#include <scopes/ScopeBase.h>

#include <scopes/internal/RuntimeImpl.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Runtime::Runtime(string const& scope_name, string const& configfile) :
    p(internal::RuntimeImpl::create(scope_name, configfile))
{
}

Runtime::~Runtime() noexcept
{
}

//! @endcond

Runtime::UPtr Runtime::create(string const& scope_name, string const& configfile)
{
    return UPtr(new Runtime(scope_name, configfile));
}

void Runtime::destroy()
{
    p->destroy();
}

RegistryProxy Runtime::registry() const
{
    return p->registry();
}

void Runtime::run_scope(ScopeBase *const scope_base)
{
    auto mw = p->factory()->create(p->scope_name(), "Zmq", "Zmq.Config");

    scope_base->start(p->scope_name(), p->registry());
    // Ensure the scope gets stopped.
    unique_ptr<ScopeBase, void(*)(ScopeBase*)> cleanup_scope(scope_base, [](ScopeBase *scope_base) { scope_base->stop(); });

    // Give a thread to the scope to do with as it likes. If the scope
    // doesn't want to use it and immediately returns from run(),
    // that's fine.
    auto run_future = std::async(launch::async, [scope_base] { scope_base->run(); });

    // Create a servant for the scope and register the servant.
    auto scope = unique_ptr<internal::ScopeObject>(new internal::ScopeObject(p.get(), scope_base));
    auto proxy = mw->add_scope_object(p->scope_name(), move(scope));

    mw->wait_for_shutdown();

    run_future.get();
}

} // namespace scopes

} // namespace api

} // namespace unity
