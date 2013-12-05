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


#include <scopes/Annotation.h>
#include <scopes/PlacementHint.h>
#include <scopes/internal/AnnotationImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

Annotation::Annotation(AnnotationType atype)
    : p(new internal::AnnotationImpl(atype))
{
}

Annotation::~Annotation()
{
}

VariantMap Annotation::serialize() const
{
    return p->serialize();
}

void Annotation::set_label(std::string const& label)
{
    p->set_label(label);
}

void Annotation::set_icon(std::string const& icon)
{
    p->set_icon(icon);
}

void Annotation::add_hyperlink(std::string const& label, Query const& query)
{
    p->add_hyperlink(label, query);
}

void Annotation::set_category(Category::SCPtr category)
{
    p->set_category(category);
}

Category::SCPtr Annotation::category() const
{
    return p->category();
}

std::string Annotation::label() const
{
    return p->label();
}

std::string Annotation::icon() const
{
    return p->icon();
}

unsigned int Annotation::num_of_hyperlinks() const
{
    return p->num_of_hyperlinks();
}

Hyperlink::SCPtr Annotation::hyperlink(unsigned int index) const
{
    return p->hyperlink(index);
}

Annotation::AnnotationType Annotation::annotation_type() const
{
    return p->annotation_type();
}

} // namespace scopes

} // namespace api

} // namespace unity
