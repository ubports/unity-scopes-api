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

#ifndef UNITY_API_SCOPES_PLACEMENTHINT_H
#define UNITY_API_SCOPES_PLACEMENTHINT_H

#include <memory>
#include <scopes/Variant.h>
#include <scopes/Category.h>
#include <unity/SymbolExport.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class PlacementHintImpl;
}

class UNITY_API PlacementHint final
{
public:
    PlacementHint() = delete;
    static PlacementHint search_bar_area();
    static PlacementHint annotation_area();
    static PlacementHint results_area(Category::SCPtr category);
    static PlacementHint category_area(Category::SCPtr category);
    VariantMap serialize() const;

private:
    PlacementHint(internal::PlacementHintImpl *pimpl);
    std::shared_ptr<internal::PlacementHintImpl> p;
};


} // namespace scopes

} // namespace api

} // namespace unity

#endif
