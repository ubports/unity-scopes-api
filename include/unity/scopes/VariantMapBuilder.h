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

#ifndef UNITY_SCOPES_VARIANTMAPBUILDER_H
#define UNITY_SCOPES_VARIANTMAPBUILDER_H

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
class VariantMapBuilderImpl;
}

/**
\brief TODO
*/
class UNITY_API VariantMapBuilder
{
public:
    VariantMapBuilder();
    virtual ~VariantMapBuilder();

    /**
    \brief Adds a top-level key-value mapping.
     */
    void add_attribute(std::string const& key, Variant const& value);

    /**
    \brief Adds a tuple of key-value pairs to an array.
    An example of using it to fill in "reviews" definition like this:
     {
         "type": "reviews",
         ...
         "reviews": [{"rating": "myrating", "review": "data", "author": "author"}]
     }

     PreviewWidgetDefinitionBuilder builder("reviews");
     builder.add_attribute("reviews", {{"rating", Variant("myrating")}, {"review", Variant("data")}, {"author", Variant("author")}});
    */
    void add_tuple(std::string const& array_key, std::initializer_list<std::pair<std::string, Variant>> const& tuple);

private:
    std::shared_ptr<internal::VariantMapBuilderImpl> p;

    friend class PreviewWidget;
};

} // namespace scopes

} // namespace unity

#endif
