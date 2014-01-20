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

#ifndef UNITY_SCOPES_PREVIEWWIDGETDEFINITIONBUILDER_H
#define UNITY_SCOPES_PREVIEWWIDGETDEFINITIONBUILDER_H

#include <unity/SymbolExport.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <memory>
#include <tuple>

namespace unity
{

namespace scopes
{
class PreviewWidget;

namespace internal
{
class PreviewWidgetDefinitionBuilderImpl;
}

/**
\brief TODO
*/
class UNITY_API PreviewWidgetDefinitionBuilder
{
public:
    PreviewWidgetDefinitionBuilder(std::string const& widget_type);
    virtual ~PreviewWidgetDefinitionBuilder();

    /**
    \brief Adds a top-level attribute mapping to this widget.
     */
    void add_attribute(std::string const& attribute, Variant const& key);

    /**
    \brief Adds attribute mappings to a an array of dictionaries.
    An example of using it to fill in "reviews" definition like this:
     {
         "type": "reviews",
         ...
         "reviews": [{"rating": "myrating", "review": "data", "author": "author"}]
     }

     PreviewWidgetDefinitionBuilder builder("reviews");
     builder.add_attribute("reviews", {{"rating", Variant("myrating")}, {"review", Variant("data")}, {"author", Variant("author")}});
    */
    void add_attribute(std::string const& top_attribute, std::initializer_list<std::pair<std::string, Variant>> const& mappings);

private:
    std::shared_ptr<internal::PreviewWidgetDefinitionBuilderImpl> p;

    friend class PreviewWidget;
};

} // namespace scopes

} // namespace unity

#endif
