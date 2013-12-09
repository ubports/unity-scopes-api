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
#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>
#include <sstream>
#include <cassert>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

AnnotationImpl::AnnotationImpl(AnnotationType annotation_type)
    : annotation_type_(annotation_type)
{
}

AnnotationImpl::AnnotationImpl(internal::CategoryRegistry const& reg, const VariantMap &variant_map)
{
    auto it = variant_map.find("type");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Annotation(): Invalid variant, missing 'type'");
    }
    auto typestr = it->second.get_string();
    if (typestr == "hyperlink")
    {
        annotation_type_ = AnnotationType::Hyperlink;
    }
    else if (typestr == "groupedhyperlink")
    {
        annotation_type_ = AnnotationType::GroupedHyperlink;
    }
    else if (typestr == "emblemhyperlink")
    {
        annotation_type_ = AnnotationType::EmblemHyperlink;
    }
    else if (typestr == "card")
    {
        annotation_type_ = AnnotationType::Card;
    }

    it = variant_map.find("label");
    if (it != variant_map.end())
    {
        set_label(it->second.get_string());
    }

    it = variant_map.find("icon");
    if (it != variant_map.end())
    {
        set_icon(it->second.get_string());
    }

    it = variant_map.find("cat_id");
    if (it != variant_map.end())
    {
        auto cat_id = it->second.get_string();
        category_ = reg.lookup_category(cat_id);
        if (category_ == nullptr)
        {
            std::ostringstream s;
            s << "Annotation(): Category '" << cat_id << "' not found in the registry";
            throw InvalidArgumentException(s.str());
        }
    }

    it = variant_map.find("hyperlinks");
    if (it != variant_map.end())
    {
        auto hyperlinks_var = it->second.get_array();
        for (const auto h: hyperlinks_var)
        {
            hyperlinks_.push_back(std::shared_ptr<Link>(new Link(h.get_dict())));
        }
    }

    throw_if_inconsistent();
}

AnnotationImpl::~AnnotationImpl()
{
}

void AnnotationImpl::set_label(std::string const& label)
{
    if (annotation_type_ != AnnotationType::GroupedHyperlink)
    {
        throw InvalidArgumentException("Annotation::set_label(): label is allowed in GroupedHyperlink only");
    }
    label_ = label;
}

void AnnotationImpl::set_icon(std::string const& icon)
{
    if (annotation_type_ != AnnotationType::EmblemHyperlink &&
        annotation_type_ != AnnotationType::Card)
    {
        throw InvalidArgumentException("Annotation::set_icon(): icon is allowed in EmblemHyperlink only");
    }
    icon_ = icon;
}

void AnnotationImpl::add_hyperlink(std::string const& label, Query const& query)
{
    if (annotation_type_ != AnnotationType::GroupedHyperlink && hyperlinks_.size() > 0)
    {
        throw InvalidArgumentException("Annotation::add_hyperlink(): multiple hyperlinks are supported by GroupedHyperlink only");
    }
    hyperlinks_.push_back(std::shared_ptr<Link>(new Link(label, query)));
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
    if (annotation_type_ != AnnotationType::GroupedHyperlink)
    {
        throw InvalidArgumentException("Annotation::label(): label is allowed in GroupedHyperlink only");
    }
    return label_;
}

std::string AnnotationImpl::icon() const
{
    if (annotation_type_ != AnnotationType::EmblemHyperlink &&
        annotation_type_ != AnnotationType::Card)
    {
        throw InvalidArgumentException("Annotation::icon(): icon is allowed in EmblemHyperlink Card annotations only");
    }
    return icon_;
}

unsigned int AnnotationImpl::num_of_hyperlinks() const
{
    return hyperlinks_.size();
}

Link::SCPtr AnnotationImpl::hyperlink(unsigned int index) const
{
    if (index >= hyperlinks_.size())
    {
        throw InvalidArgumentException("Annotation::get_hyperlink(): invalid hyperlink index");
    }
    return hyperlinks_[index];
}

AnnotationType AnnotationImpl::annotation_type() const
{
    return annotation_type_;
}

void AnnotationImpl::throw_if_inconsistent() const
{
    if (hyperlinks_.size() == 0)
    {
        throw InvalidArgumentException("No hyperlinks present");
    }

    switch (annotation_type_)
    {
        case AnnotationType::Hyperlink:
            // nothing to verify
            break;
        case AnnotationType::GroupedHyperlink:
            if (label_.empty())
            {
                throw InvalidArgumentException("Label must not be empty for GroupedHyperlink annotation");
            }
            break;
        case AnnotationType::EmblemHyperlink:
            if (icon_.empty())
            {
                throw InvalidArgumentException("Icon must not be empty for EmblemHyperlink annotation");
            }
            break;
        case AnnotationType::Card:
            if (category_ == nullptr)
            {
                throw InvalidArgumentException("Category must be set for Card annotation");
            }
            if (icon_.empty())
            {
                throw InvalidArgumentException("Icon must not be empty for Card annotation");
            }
            break;
        default:
            throw InvalidArgumentException("Unknown annotation type");
    }
}

VariantMap AnnotationImpl::serialize() const
{
    throw_if_inconsistent();

    VariantMap vm;
    switch (annotation_type_)
    {
        case AnnotationType::Hyperlink:
            vm["type"] = "hyperlink";
            break;
        case AnnotationType::GroupedHyperlink:
            vm["type"] = "groupedhyperlink";
            break;
        case AnnotationType::EmblemHyperlink:
            vm["type"] = "emblemhyperlink";
            break;
        case AnnotationType::Card:
            vm["type"] = "card";
            break;
        default:
            assert(0); // should never happen
    }

    if (category_)
    {
        vm["cat_id"] = category_->id();
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
