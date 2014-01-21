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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/PreviewWidgetImpl.h>
#include <unity/scopes/PreviewWidget.h>

namespace unity
{

namespace scopes
{

namespace internal

{

//! @cond

PreviewWidgetImpl::PreviewWidgetImpl(std::string const& json_text)
    : data_(json_text)
{
    //TODO: json validation
}

PreviewWidgetImpl::PreviewWidgetImpl(VariantMap const& variant_map)
{
    auto it = variant_map.find("data");
    if (it != variant_map.end() && it->second.which() == Variant::Type::String)
    {
        data_ = it->second.get_string();
    }
}

std::string PreviewWidgetImpl::data() const
{
    return data_;
}

VariantMap PreviewWidgetImpl::serialize() const
{
    VariantMap vm;
    vm["data"] = data_;
    return vm;
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace unity
