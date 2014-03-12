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

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/AnnotationImpl.h>

namespace unity
{

namespace scopes
{

Annotation::Annotation(Type atype)
    : p(new internal::AnnotationImpl(atype))
{
}

/// @cond
Annotation::Annotation(Annotation const &other)
    : p(new internal::AnnotationImpl(*(other.p)))
{
}

Annotation::Annotation(internal::AnnotationImpl* impl)
    : p(impl)
{
}

Annotation::Annotation(Annotation&&) = default;

Annotation& Annotation::operator=(Annotation const& other)
{
    if (this != &other)
    {
        p.reset(new internal::AnnotationImpl(*(other.p)));
    }
    return *this;
}

Annotation& Annotation::operator=(Annotation&&) = default;

Annotation::~Annotation()
{
}

VariantMap Annotation::serialize() const
{
    return p->serialize();
}
/// @endcond

void Annotation::set_label(std::string const& label)
{
    p->set_label(label);
}

void Annotation::set_icon(std::string const& icon)
{
    p->set_icon(icon);
}

void Annotation::add_link(std::string const& label, CannedQuery const& query)
{
    p->add_link(label, query);
}

std::string Annotation::label() const
{
    return p->label();
}

std::string Annotation::icon() const
{
    return p->icon();
}

std::list<Link::SCPtr> Annotation::links() const
{
    return p->links();
}

Annotation::Type Annotation::annotation_type() const
{
    return p->annotation_type();
}

} // namespace scopes

} // namespace unity
