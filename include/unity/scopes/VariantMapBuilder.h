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
#include <vector>
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
\brief Helper class for creating and populating VariantMap (dictionary) instances.
The main purpose of this class is to ease creation of dictionaries needed for PreviewWidget
instances or any other classes that require non-trivial definitions.

For example, a definiton of a PreviewWidget that corresponds with the following JSON template:
{
    "type": "reviews",
    "rating-icon-empty": null, // icon url
    "rating-icon-half": null, // icon url
    "rating-icon-full": null, // icon url
    "reviews": [{"rating": null, "review": null, "author": null}]
}

can be created with VariantMapBuilder as follows:

\code
VariantMapBuilder builder;
builder.add_attribute("type", "reviews");
builder.add_attribute("rating-icon-empty", Variant::Null);
builder.add_attribute("rating-icon-half", Variant::Null);
builder.add_attribute("rating-icon-full", Variant::Null);
builder.add_tuple("reviews", {{"rating", Variant::null())}, {"review", Variant::null()}, {"author", Variant::null()}});

Preview widget(builder.variant_map());
\endcode
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
    It can be used multiple times for same key to create an array of tuples assigned to that key, for example:
     {
         ...
         "mykey": [{"a": 1, "b": 2}, {"c": 2, "d" : 3}]
     }

     \code
     VariantMapBuilder builder;
     builder.add_tuple("mykey", {{"a", Variant(1)}, {"b", Variant(2)}});
     builder.add_tuple("mykey", {{"c", Variant(2)}, {"d", Variant(3)}});
     \endcode
    */
    void add_tuple(std::string const& array_key, std::initializer_list<std::pair<std::string, Variant>> const& tuple);

    /**
    \brief Adds a tuple of key-value pairs to an array.
    This is and overloaded version of add_tupe that takes std::vector instead of std::initializer_list, making it more friendly for language
    bindings.
    */
    void add_tuple(std::string const& array_key, std::vector<std::pair<std::string, Variant>> const& tuple);

    VariantMap variant_map() const;

private:
    std::shared_ptr<internal::VariantMapBuilderImpl> p;

    friend class PreviewWidget;
};

} // namespace scopes

} // namespace unity

#endif
