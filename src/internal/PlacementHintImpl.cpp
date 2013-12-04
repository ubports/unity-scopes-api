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

#include <scopes/internal/PlacementHintImpl.h>
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

PlacementHintImpl::PlacementHintImpl(PlacementHint::Placement placement)
    : placement_(placement)
{
}

PlacementHintImpl::PlacementHintImpl(PlacementHint::Placement placement, Category::SCPtr category)
    : placement_(placement),
      category_(category)
{
}

PlacementHint::Placement PlacementHintImpl::placement() const
{
    return placement_;
}

Category::SCPtr PlacementHintImpl::category() const
{
    if (placement_ == PlacementHint::Placement::SearchBarArea || placement_ == PlacementHint::Placement::AnnotationArea)
    {
        throw unity::LogicException("PlacementHint: category is not specified for SearchBarArea or AnnotationArea placement");
    }
    return category_;
}

VariantMap PlacementHintImpl::serialize() const
{
    std::string pstr;
    switch (placement_)
    {
        case PlacementHint::SearchBarArea:
            pstr = "searchbar";
            break;
        case PlacementHint::AnnotationArea:
            pstr = "annotation";
            break;
        case PlacementHint::ResultsArea:
            pstr = "results";
            break;
        case PlacementHint::CategoryArea:
            pstr = "category";
            break;
        default: // this should never happen
            assert(false);
            break;
    }

    VariantMap vm;
    vm["area"] = pstr;
    if (category_)
    {
        vm["category"] = category_->id();
    }
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
