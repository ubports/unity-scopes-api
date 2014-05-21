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

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ScopeExceptions.h>

using namespace std;
using namespace std::placeholders;

namespace unity
{

namespace scopes
{

namespace internal
{

MWRegistry::MWRegistry(MiddlewareBase* mw_base) :
    MWObjectProxy(mw_base),
    mw_base_(mw_base)
{
}

MWRegistry::~MWRegistry()
{
}

void MWRegistry::set_update_callback(std::function<void()> callback)
{
    if (!subscriber_)
    {
        // Use lazy initialization here to only subscribe to a publisher if a callback is set
        try
        {
            subscriber_ = mw_base_->create_subscriber(mw_base_->runtime()->registry_identity());
        }
        catch (MiddlewareException const& e)
        {
            cerr << "MWRegistry::set_update_callback(): failed to create registry subscriber: " << e.what() << endl;
        }
    }
    if (subscriber_)
    {
        subscriber_->set_message_callback([callback](string const&){ callback(); });
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
