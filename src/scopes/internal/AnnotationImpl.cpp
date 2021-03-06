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

#include <unity/scopes/internal/AnnotationImpl.h>
#include <unity/UnityExceptions.h>
#include <sstream>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

AnnotationImpl::AnnotationImpl(Annotation::Type annotation_type)
    : annotation_type_(annotation_type)
{
}

AnnotationImpl::AnnotationImpl(const VariantMap &variant_map)
    : annotation_type_(static_cast<Annotation::Type>(0xefff))    // Impossible value
{
    auto it = variant_map.find("type");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Annotation(): Invalid variant, missing 'type'");
    }
    auto typestr = it->second.get_string();
    if (typestr == "link")
    {
        annotation_type_ = Annotation::Type::Link;
    }
    else if (typestr == "groupedlink")
    {
        annotation_type_ = Annotation::Type::GroupedLink;
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

    it = variant_map.find("links");
    if (it != variant_map.end())
    {
        auto links_var = it->second.get_array();
        for (auto const& h: links_var)
        {
            links_.push_back(std::shared_ptr<Link>(new Link(h.get_dict())));
        }
    }

    throw_if_inconsistent();
}

AnnotationImpl::~AnnotationImpl()
{
}

void AnnotationImpl::set_label(std::string const& label)
{
    if (annotation_type_ != Annotation::Type::GroupedLink)
    {
        throw LogicException("Annotation::set_label(): label is allowed in GroupedLink only");
    }
    if (label.empty())
    {
        throw InvalidArgumentException("Annotation::set_label(): Invalid empty label string");
    }
    label_ = label;
}

void AnnotationImpl::set_icon(std::string const& icon)
{
    if (annotation_type_ != Annotation::Type::Link)
    {
        throw LogicException("Annotation::set_icon(): icon is allowed in Link annotations only");
    }
    if (icon.empty())
    {
        throw InvalidArgumentException("Annotation::set_icon(): Invalid empty icon string");
    }
    icon_ = icon;
}

void AnnotationImpl::add_link(std::string const& label, CannedQuery const& query)
{
    if (annotation_type_ != Annotation::Type::GroupedLink && !links_.empty())
    {
        throw InvalidArgumentException("Annotation::add_link(): multiple links are supported by GroupedLink only");
    }
    try
    {
        links_.push_back(std::shared_ptr<Link>(new Link(label, query)));
    }
    catch (...)
    {
        throw ResourceException("Annotation::add_link(): cannot create link");
    }
}

std::string AnnotationImpl::label() const
{
    if (annotation_type_ != Annotation::Type::GroupedLink)
    {
        throw LogicException("Annotation::label(): label is allowed in GroupedLink only");
    }
    return label_;
}

std::string AnnotationImpl::icon() const
{
    if (annotation_type_ != Annotation::Type::Link)
    {
        throw LogicException("Annotation::icon(): icon is allowed in Link annotations only");
    }
    return icon_;
}

std::list<Link::SCPtr> AnnotationImpl::links() const
{
    return links_;
}

Annotation::Type AnnotationImpl::annotation_type() const
{
    return annotation_type_;
}

void AnnotationImpl::throw_if_inconsistent() const
{
    if (links_.empty())
    {
        throw InvalidArgumentException("No links present");
    }

    switch (annotation_type_)
    {
        case Annotation::Type::Link:
            // nothing to verify
            break;
        case Annotation::Type::GroupedLink:
            if (label_.empty())
            {
                throw InvalidArgumentException("Label must not be empty for GroupedLink annotation");
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
        case Annotation::Type::Link:
            vm["type"] = "link";
            break;
        case Annotation::Type::GroupedLink:
            vm["type"] = "groupedlink";
            break;
        default:
            assert(0); // should never happen
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
    for (auto const& link: links_)
    {
        links_var.push_back(Variant(link->serialize()));
    }
    vm["links"] = links_var;
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace unity
