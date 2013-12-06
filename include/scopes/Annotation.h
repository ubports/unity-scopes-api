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

#ifndef UNITY_API_SCOPES_ANNOTATION_H
#define UNITY_API_SCOPES_ANNOTATION_H

#include <unity/SymbolExport.h>
#include <scopes/Variant.h>
#include <scopes/Hyperlink.h>
#include <scopes/Category.h>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{
class Query;

namespace internal
{
class AnnotationImpl;
class ReplyObject;
}

/**
 * \brief Handles a scope query hyperlink(s) that result in a new search query when clicked by user.
 */
class UNITY_API Annotation final
{
public:
    /**
     * \brief Enumeration of supported Annotation types
     */
    enum AnnotationType
    {
        Hyperlink, //!< A simple hyperlink with just a label
        GroupedHyperlink, //!< A group of hyperlinks, with a label for the group name and labels for all links inside it
        EmblemHyperlink, //!< A hyperlink with a label and an icon
        Card //!< A hyperlink that has a label and icon and is meant to be displayed within a category (with results)
    };

    /**
     * \brief Creates annotation of given type. AnnotationType imposes attributes that are
     * supported or required by that annotation.
     */
    Annotation(AnnotationType atype);

    /// @cond
    Annotation(Annotation const &other);
    Annotation(Annotation&&);
    Annotation& operator=(Annotation const& other);
    Annotation& operator=(Annotation&&);

    virtual ~Annotation();
    /// @endcond

    /**
     * \brief Sets a label for an annotation of AnnotationType::GroupedHyperlink type.
     * Throws InvalidArgumentException for other annotation types.
     */
    void set_label(std::string const& label);

    /**
     * \brief Sets an icon for an annotation of AnnotationType::EmblemHyperlink or
     * AnnotationType::Card type. Throws InvalidArgumentException for other annotation types.
     */
    void set_icon(std::string const& icon);

    /**
     * \brief Adds a hyperlink to the annotation. There needs to be exactly one hyperlink
     * added for the annotations of type AnnotationType::Hyperlink, AnnotationType::EmblemHyperlink
     * and AnnotationType::Card and at least one for AnnotationType::GroupedHyperlink. This method
     * throws InvalidArgumentException if these constraints are violated.
     */
    void add_hyperlink(std::string const& label, Query const& query);

    /**
     * \brief Assigns this annotation to a category. Depending on the AnnotationType set, it
     * get be displayed among results of that category (for AnnotationType::Card) or
     * below it (for remaing annotation types).
     */
    void set_category(Category::SCPtr category);

    /**
     * \brief Returns category assigned to this annotation. Category can be nullptr, in which case
     * the annotation should be displayed at the top annotation area.
     * \return category instance or nullptr.
     */
    Category::SCPtr category() const;

    /**
     * \brief Returns label assigned to this annotation. This is only valid for AnnotationType::GroupedHyperlink
     * and throws InvalidArgumentException for other annotation types.
     * \return label associated with this annotation
     */
    std::string label() const;

    /**
     * \brief Returns icon assigned to this annotation. This is only valid for AnnotationType::EmblemHyperlink and
     * AnnotationType::Card and throws InvalidArgumentException for other annotation types.
     * \return icon associated with this annotation
     */
    std::string icon() const;

    /**
     * \brief Returns number of hyperlinks assigned to this annotation.
     */
    unsigned int num_of_hyperlinks() const;

    /**
     * \brief Returns a hyperlink assigned to given position of this annotation. Throws InvalidArgumentException
     * on invalid index.
     * \returns hyperlink at given position
     */
    Hyperlink::SCPtr hyperlink(unsigned int index) const;

    /**
     * \brief Returns the type of this annotation.
     */
    AnnotationType annotation_type() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    Annotation(internal::AnnotationImpl* impl);
    std::shared_ptr<internal::AnnotationImpl> p;

    friend class internal::ReplyObject;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
