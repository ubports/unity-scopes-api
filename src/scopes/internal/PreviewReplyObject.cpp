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
#include <unity/scopes/PreviewWidget.h>

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
    if (it != data.end() && it->second.which() == Variant::Type::Array)
    {
        VariantArray arr = it->second.get_array();
        PreviewWidgetList list;
        for (unsigned i = 0; i < arr.size(); i++)
        {
            if (arr[i].which() != Variant::Type::Dict) continue;
            VariantMap inner(arr[i].get_dict());
            auto json_it = inner.find("data");
            if (json_it->second.which() != Variant::Type::String) continue;

            list.emplace_back(PreviewWidget(json_it->second.get_string()));
        }
        receiver_->push(list);
    }

    auto data_it = data.find("preview-data");
    if (data_it != data.end() && data_it->second.which() == Variant::Type::Dict)
    {
        VariantMap data = data_it->second.get_dict();
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            receiver_->push(it->first, it->second);
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
