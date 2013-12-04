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

#ifndef UNITY_INTERNAL_ANNOTATIONOBJECTBASEIMPL_H
#define UNITY_INTERNAL_ANNOTATIONOBJECTBASEIMPL_H

#include <unity/SymbolExport.h>
#include <scopes/PlacementHint.h>
#include <scopes/Query.h>
#include <scopes/Variant.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class UNITY_API AnnotationObjectBaseImpl
{
public:
    AnnotationObjectBaseImpl(PlacementHint const& placement);
    virtual ~AnnotationObjectBaseImpl();
    PlacementHint placement_hint() const;
    virtual Query canned_query() const = 0;
    virtual const char* type_string() const = 0;
    VariantMap serialize() const;

protected:
    virtual void serialize(VariantMap&) const = 0;

private:
    PlacementHint placement_hint_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
