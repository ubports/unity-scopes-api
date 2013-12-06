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

#ifndef UNITY_INTERNAL_ANNOTATIONIMPL_H
#define UNITY_INTERNAL_ANNOTATIONIMPL_H

#include <unity/SymbolExport.h>
#include <scopes/Query.h>
#include <scopes/Variant.h>
#include <scopes/Annotation.h>
#include <scopes/Hyperlink.h>
#include <scopes/Category.h>
#include <vector>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class CategoryRegistry;

class UNITY_API AnnotationImpl
{
public:
    explicit AnnotationImpl(Annotation::AnnotationType annotation_type);
    AnnotationImpl(internal::CategoryRegistry const& reg, const VariantMap &variant_map);
    virtual ~AnnotationImpl();

    void set_label(std::string const& label);
    void set_icon(std::string const& icon);
    void add_hyperlink(std::string const& label, Query const& query);
    void set_category(Category::SCPtr category);
    Category::SCPtr category() const;
    std::string label() const;
    std::string icon() const;
    unsigned int num_of_hyperlinks() const;
    Hyperlink::SCPtr hyperlink(unsigned int index) const;
    Annotation::AnnotationType annotation_type() const;
    VariantMap serialize() const;

private:
    void throw_if_inconsistent() const;
    Annotation::AnnotationType annotation_type_;
    std::string label_;
    std::string icon_;
    Category::SCPtr category_;
    std::vector<Hyperlink::SCPtr> hyperlinks_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
