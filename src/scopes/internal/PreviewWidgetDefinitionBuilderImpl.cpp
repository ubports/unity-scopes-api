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

#include <unity/scopes/internal/PreviewWidgetDefinitionBuilderImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

PreviewWidgetDefinitionBuilderImpl::PreviewWidgetDefinitionBuilderImpl(std::string const& widget_type)
{
    widget_def_["type"] = widget_type;
}

void PreviewWidgetDefinitionBuilderImpl::add_attribute(std::string const& attribute, Variant const& key)
{
    widget_def_[attribute] = key;
}

void PreviewWidgetDefinitionBuilderImpl::add_attribute(std::string const& top_attribute, std::initializer_list<std::pair<std::string, Variant>> const& mappings)
{
    VariantArray va;
    auto it = widget_def_.find(top_attribute);
    if (it != widget_def_.end())
    {
        va = it->second.get_array();
    }

    va.push_back(Variant(VariantMap(mappings.begin(), mappings.end())));
    widget_def_[top_attribute] = va;
}

VariantMap PreviewWidgetDefinitionBuilderImpl::get_definition() const
{
    return widget_def_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
