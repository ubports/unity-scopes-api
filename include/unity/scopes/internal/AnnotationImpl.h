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

#ifndef UNITY_SCOPES_INTERNAL_ANNOTATIONIMPL_H
#define UNITY_SCOPES_INTERNAL_ANNOTATIONIMPL_H

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/Link.h>
#include <list>

namespace unity
{

namespace scopes
{

namespace internal
{

class AnnotationImpl
{
public:
    explicit AnnotationImpl(Annotation::Type annotation_type);
    AnnotationImpl(const VariantMap &variant_map);
    virtual ~AnnotationImpl();

    void set_label(std::string const& label);
    void set_icon(std::string const& icon);
    void add_link(std::string const& label, CannedQuery const& query);
    std::string label() const;
    std::string icon() const;
    std::list<Link::SCPtr> links() const;
    Annotation::Type annotation_type() const;
    VariantMap serialize() const;

private:
    void throw_if_inconsistent() const;
    Annotation::Type annotation_type_;
    std::string label_;
    std::string icon_;
    std::list<Link::SCPtr> links_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
