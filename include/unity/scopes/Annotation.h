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

#ifndef UNITY_SCOPES_ANNOTATION_H
#define UNITY_SCOPES_ANNOTATION_H

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

/**
 * \brief Query link(s) that result in a new search query when clicked by the user.
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
     * \brief Creates annotation of given type. The Type imposes attributes that are
     * supported or required by that annotation.
     */
    explicit Annotation(Type atype);

    /// @cond
    Annotation(Annotation const &other);
    Annotation(Annotation&&);
    Annotation& operator=(Annotation const& other);
    Annotation& operator=(Annotation&&);

    virtual ~Annotation();
    /// @endcond

    /**
     * \brief Sets a label for an annotation. This currently makes sense for Type::GroupedLink only.
     */
    void set_label(std::string const& label);

    /**
     * \brief Sets an icon for an annotation. This currently makes sense for Type::Link.
     */
    void set_icon(std::string const& icon);

    /**
     * \brief Adds a link to the annotation. There needs to be exactly one link
     * added for the annotations of type Type::Link
     * and at least one for Type::GroupedLink. This method
     * throws InvalidArgumentException if these constraints are violated.
     */
    void add_link(std::string const& label, CannedQuery const& query);

    /**
     * \brief Returns label assigned to this annotation. This currently makes sense for Type::GroupedLink only.
     * \return label The label associated with this annotation.
     */
    std::string label() const;

    /**
     * \brief Returns icon assigned to this annotation. This currently only makes sense for Type::Link.
     * \return The icon associated with this annotation.
     */
    std::string icon() const;

    /**
     * \brief Returns all links assigned to given position of this annotation.
     * \return The link at the given position.
     */
    std::list<Link::SCPtr> links() const;

    /**
     * \brief Returns the type of this annotation.
     * \return Teh type of the annotation.
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

} // namespace scopes

} // namespace unity

#endif
