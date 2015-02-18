/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma once

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <QtCore/QPair>
#include <QtCore/QString>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop
#include <QtCore/QVector>

#include <memory>

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{
class QVariantBuilderImpl;
}

/**
\brief Helper class for creating and populating QVariant containers.

The main purpose of this class is to ease creation of variant containers needed for QPreviewWidget
instances or any other classes that require non-trivial variant definitions.

For example, the value of the "rating" key of the following JSON template
\code
{
    "type": "reviews",
    ...
    "reviews": [{"rating": null, "review": null, "author": null}]
}
\endcode

can be created with QVariantBuilder as follows:

\code
VariantBuilder builder;
builder.add_tuple({{"rating", Variant::null())}, {"review", Variant::null()}, {"author", Variant::null()}});
\endcode
*/
class QVariantBuilder final
{
public:
    QVariantBuilder();
    virtual ~QVariantBuilder();

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QVariantBuilder(QVariantBuilder const& other);
    QVariantBuilder(QVariantBuilder&& other);

    QVariantBuilder& operator=(QVariantBuilder const& other);
    QVariantBuilder& operator=(QVariantBuilder&& other);
    //@}

    /**
    \brief Adds a tuple of key-value pairs to an array.

    It can be used multiple times to create an array of tuples, for example:
    \code
       [{"a": 1, "b": 2}, {"c": 2, "d" : 3}]
    \endcode
    can be created with:
    \code
    QVariantBuilder builder;
    builder.add_tuple({{"a", Variant(1)}, {"b", Variant(2)}});
    builder.add_tuple({{"c", Variant(2)}, {"d", Variant(3)}});
    \endcode
    */
    //    void add_tuple(std::initializer_list<QPair<QString, QVariant>> const& tuple);

    /**
    \brief Adds a tuple of key-value pairs to an array.

    This is an overloaded version of add_tuple that accepts `std::vector` instead of
    `std::initializer_list`, making it more convenient for language bindings.
    */
    void add_tuple(QVector<QPair<QString, QVariant>> const& tuple);

    /**
     \brief Retrieves a completed variant.

     Returns the completed variant and resets this builder, so the builder can be re-used.
     \return The completed variant.
     \throws unity::LogicException if the builder does not hold a variant.
    */
    QVariant end();

private:
    ///@cond
    QVariantBuilder(internal::QVariantBuilderImpl* impl);
    std::unique_ptr<internal::QVariantBuilderImpl> p;
    friend class internal::QVariantBuilderImpl;
    ///@endcond
};
}  // namespace qt

}  // namespace scopes

}  // namespace unity
