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

#include <unity/scopes/internal/PreviewReplyObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ListenerBase.h>

#include <iostream> // TODO: remove this once logging is added

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

PreviewReplyObject::PreviewReplyObject(PreviewListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name) :
    ReplyObject(std::static_pointer_cast<ListenerBase>(receiver), runtime, scope_name),
    receiver_(receiver)
{
}

PreviewReplyObject::~PreviewReplyObject() noexcept
{
}

void PreviewReplyObject::process_data(VariantMap const& data)
{
    auto it = data.find("widgets");
    if (it != data.end())
    {
        // TODO: push the widget list
    }

    it = data.find("data");
    if (it != data.end())
    {
        // TODO: push the actual data
    }
    // FIXME: just a test that it's talking to PreviewListener
    receiver_->push("example-data", Variant("foo"));
}

} // namespace internal

} // namespace scopes

} // namespace unity
