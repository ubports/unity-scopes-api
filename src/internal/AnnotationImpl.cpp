/* Copyright (C) 2013 Canonical Ltd
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

#include <scopes/internal/AnnotationImpl.h>
#include <unity/UnityExceptions.h>
#include <cassert>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

AnnotationImpl::AnnotationImpl(Annotation::AnnotationType annotation_type)
    : annotation_type_(annotation_type)
{
}

AnnotationImpl::~AnnotationImpl()
{
}

void AnnotationImpl::set_label(std::string const& label)
{
    if (annotation_type_ != Annotation::AnnotationType::GroupedHyperlink)
    {
        throw InvalidArgumentException("Annotation::set_label(): label is allowed in GroupedHyperlink only");
    }
    label_ = label;
}

void AnnotationImpl::set_icon(std::string const& icon)
{
    if (annotation_type_ != Annotation::AnnotationType::EmblemHyperlink &&
        annotation_type_ != Annotation::AnnotationType::Card)
    {
        throw InvalidArgumentException("Annotation::set_icon(): icon is allowed in EmblemHyperlink only");
    }
    icon_ = icon;
}

void AnnotationImpl::add_hyperlink(std::string const& label, Query const& query)
{
    if (annotation_type_ != Annotation::AnnotationType::GroupedHyperlink && hyperlinks_.size() > 0)
    {
        throw InvalidArgumentException("Annotation::add_hyperlink(): multiple hyperlinks are supported by GroupedHyperlink only");
    }
    hyperlinks_.push_back(std::shared_ptr<Hyperlink>(new Hyperlink(label, query)));
}

void AnnotationImpl::set_category(Category::SCPtr category)
{
    category_ = category;
}

Category::SCPtr AnnotationImpl::category() const
{
    return category_;
}

std::string AnnotationImpl::label() const
{
    if (annotation_type_ != Annotation::AnnotationType::GroupedHyperlink)
    {
        throw InvalidArgumentException("Annotation::label(): label is allowed in GroupedHyperlink only");
    }
    return label_;
}

std::string AnnotationImpl::icon() const
{
    if (annotation_type_ != Annotation::AnnotationType::EmblemHyperlink &&
        annotation_type_ != Annotation::AnnotationType::Card)
    {
        throw InvalidArgumentException("Annotation::icon(): icon is allowed in EmblemHyperlink Card annotations only");
    }
    return icon_;
}

unsigned int AnnotationImpl::num_of_hyperlinks() const
{
    return hyperlinks_.size();
}

Hyperlink::SCPtr AnnotationImpl::hyperlink(unsigned int index) const
{
    if (index >= hyperlinks_.size())
    {
        throw InvalidArgumentException("Annotation::get_hyperlink(): invalid hyperlink index");
    }
    return hyperlinks_[index];
}

Annotation::AnnotationType AnnotationImpl::annotation_type() const
{
    return annotation_type_;
}

VariantMap AnnotationImpl::serialize() const
{
    if (hyperlinks_.size() == 0)
    {
        throw InvalidArgumentException("Annotation::serialize(): no hyperlinks");
    }

    VariantMap vm;
    switch (annotation_type_)
    {
        case Annotation::AnnotationType::Hyperlink:
            vm["type"] = "hyperlink";
            break;
        case Annotation::AnnotationType::GroupedHyperlink:
            vm["type"] = "groupedhyperlink";
            if (label_.empty())
            {
                throw InvalidArgumentException("Annotation::serialize(): label must not be empty for GroupedHyperlink");
            }
            break;
        case Annotation::AnnotationType::EmblemHyperlink:
            if (icon_.empty())
            {
                throw InvalidArgumentException("Annotation::serialize(): icon must not be empty for EmblemHyperlink");
            }
            vm["type"] = "emblemhyperlink";
            break;
        case Annotation::AnnotationType::Card:
            if (icon_.empty())
            {
                throw InvalidArgumentException("Annotation::serialize(): icon must not be empty for Card");
            }
            vm["type"] = "card";
            break;
        default:
            assert(0); // should never happen
    }

    if (category_)
    {
        vm["category"] = category_->id();
    }
    if (!label_.empty())
    {
        vm["label"] = label_;
    }
    if (!icon_.empty())
    {
        vm["icon"] = icon_;
    }

    VariantArray links_var;
    for (auto link: hyperlinks_)
    {
        links_var.push_back(Variant(link->serialize()));
    }
    vm["hyperlinks"] = links_var;
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
