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

#ifndef UNITY_SCOPES_VARIANTBUILDER_H
#define UNITY_SCOPES_VARIANTBUILDER_H

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

namespace internal
{
class VariantBuilderImpl;
}

/**
\brief Helper class for creating and populating Variant containers.

The main purpose of this class is to ease creation of variant containers needed for PreviewWidget
instances or any other classes that require non-trivial variant definitions.

For example, the value of "rating" key of the following JSON template:
{
    "type": "reviews",
    ...
    "reviews": [{"rating": null, "review": null, "author": null}]
}

can be created with VariantBuilder as follows:

\code
VariantBuilder builder;
builder.add_tuple({{"rating", Variant::null())}, {"review", Variant::null()}, {"author", Variant::null()}});
\endcode
*/
class UNITY_API VariantBuilder
{
public:
    VariantBuilder();
    VariantBuilder(VariantBuilder const& other);
    VariantBuilder(VariantBuilder&& other);
    virtual ~VariantBuilder();

    VariantBuilder& operator=(VariantBuilder const& other);
    VariantBuilder& operator=(VariantBuilder&& other);

    /**
    \brief Adds a tuple of key-value pairs to an array.
    It can be used multiple times to create an array of tuples, for example:
    \code
       [{"a": 1, "b": 2}, {"c": 2, "d" : 3}]
    \endcode
    can be created with:
    \code
    VariantBuilder builder;
    builder.add_tuple({{"a", Variant(1)}, {"b", Variant(2)}});
    builder.add_tuple({{"c", Variant(2)}, {"d", Variant(3)}});
    \endcode
    */
    void add_tuple(std::initializer_list<std::pair<std::string, Variant>> const& tuple);

    /**
    \brief Adds a tuple of key-value pairs to an array.
    This is and overloaded version of add_tupe that takes std::vector instead of std::initializer_list, making it more friendly for language
    bindings.
    */
    void add_tuple(std::vector<std::pair<std::string, Variant>> const& tuple);

    /**
     \brief Retrieves created Variant.
     Returns created Variant and resets the builder, so that it can be reused for creating a new Variant. Throw unity::LogicException if
     no Variant has been created.
     \return created variant
    */
    Variant end();

private:
    std::shared_ptr<internal::VariantBuilderImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
