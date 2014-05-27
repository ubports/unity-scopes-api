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

using namespace std;

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

void MWRegistry::set_scope_state_callback(std::string const& scope_id, std::function<void(bool)> callback)
{
    if (!scope_state_subscriber_)
    {
        // Use lazy initialization here to only subscribe to the publisher if a callback is set
        try
        {
            scope_state_subscriber_ = mw_base_->create_subscriber(mw_base_->runtime()->registry_identity(), scope_id);
        }
        catch (std::exception const& e)
        {
            cerr << "MWRegistry::set_scope_state_callback(): failed to create registry subscriber: " << e.what() << endl;
        }
    }
    if (scope_state_subscriber_)
    {
        scope_state_subscriber_->set_message_callback([callback](string const& state){ callback(state == "started"); });
    }
}

void MWRegistry::set_list_update_callback(std::function<void()> callback)
{
    if (!list_update_subscriber_)
    {
        // Use lazy initialization here to only subscribe to the publisher if a callback is set
        try
        {
            list_update_subscriber_ = mw_base_->create_subscriber(mw_base_->runtime()->registry_identity());
        }
        catch (std::exception const& e)
        {
            cerr << "MWRegistry::set_list_update_callback(): failed to create registry subscriber: " << e.what() << endl;
        }
    }
    if (list_update_subscriber_)
    {
        list_update_subscriber_->set_message_callback([callback](string const&){ callback(); });
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
