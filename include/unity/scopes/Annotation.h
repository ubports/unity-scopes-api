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

#pragma once

#include <unity/scopes/Variant.h>
#include <unity/scopes/Link.h>
#include <unity/util/DefinesPtrs.h>
#include <list>
#include <memory>

namespace unity
{

namespace scopes
{
class CannedQuery;

namespace internal
{
class AnnotationImpl;
class ResultReplyObject;
}

namespace experimental
{

// TODO: This class looks a lot like a union: either it's a Link or it's a GroupedLink, and
//       some methods apply to one, but not the other (and vice versa). This might be
//       two classes instead?

/**
\brief Query link(s) that result in a new search query when clicked by the user.
*/

class Annotation final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Annotation);
    /// @endcond

    /*!
    \brief Enumeration of supported Annotation types
    */
    enum Type
    {
        Link, /**< A simple link with just a label and/or an icon */
        GroupedLink /**< A group of links, with a label for the group name and labels for all links inside it */
    };

    /**
    \brief Creates annotation of given type. The Type imposes attributes that are
    supported or required by that annotation.
    */
    explicit Annotation(Type atype);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Annotation(Annotation const &other);
    Annotation(Annotation&&);
    Annotation& operator=(Annotation const& other);
    Annotation& operator=(Annotation&&);
    //@}

    /// @cond
    virtual ~Annotation();
    /// @endcond

    /**
    \brief Sets a label for an annotation.

    \throws InvalidArgumentException if label is the empty string.
    \throws LogicException if the type is not Type::GroupedLink.
    */
    void set_label(std::string const& label);

    /**
    \brief Sets an icon for an annotation.

    \throws InvalidArgumentException if icon is the empty string.
    \throws LogicException if the type is not Type::Link.
    */
    void set_icon(std::string const& icon);

    /**
    \brief Adds a link to the annotation.

    There needs to be exactly one link added for an annotations of type Type::Link
    and one or more for Type::GroupedLink.
    \throws InvalidArgumentException if an attempt is made to add more than one annotation and the
    type is not Type::GroupedLink.
    */
    void add_link(std::string const& label, CannedQuery const& query);

    /**
    \brief Returns label assigned to this annotation.
    \return label The label associated with this annotation.
    \throws LogicException if the type is not Type::GroupedLink.
    */
    std::string label() const;

    /**
    \brief Returns icon assigned to this annotation.
    \return The icon associated with this annotation.
    \throws LogicException if the type is not Type::Link.
    */
    std::string icon() const;

    /**
    \brief Returns all links assigned to given position of this annotation.
    \return The link at the given position.
    */
    std::list<Link::SCPtr> links() const;

    /**
    \brief Returns the type of this annotation.
    \return The type of the annotation.
    */
    Type annotation_type() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    Annotation(internal::AnnotationImpl* impl);
    std::unique_ptr<internal::AnnotationImpl> p;

    friend class internal::ResultReplyObject;
};

} // namespace experimental

} // namespace scopes

} // namespace unity
