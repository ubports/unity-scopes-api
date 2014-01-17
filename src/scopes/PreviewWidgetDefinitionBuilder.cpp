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

#include <unity/scopes/PreviewWidgetDefinitionBuilder.h>
#include <unity/scopes/internal/PreviewWidgetDefinitionBuilderImpl.h>

namespace unity
{

namespace scopes
{

PreviewWidgetDefinitionBuilder::PreviewWidgetDefinitionBuilder(std::string const& widget_type)
    : p(new internal::PreviewWidgetDefinitionBuilderImpl(widget_type))
{
}

PreviewWidgetDefinitionBuilder::~PreviewWidgetDefinitionBuilder() = default;

void PreviewWidgetDefinitionBuilder::add_attribute(std::string const& attribute, Variant const& key)
{
    p->add_attribute(attribute, key);
}

void PreviewWidgetDefinitionBuilder::add_attribute(std::string const& top_attribute, std::initializer_list<std::pair<std::string, Variant>> const& mappings)
{
    p->add_attribute(top_attribute, mappings);
}

} // namespace scopes

} // namespace unity
