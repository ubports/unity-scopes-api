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

#include <scopes/PlacementHint.h>
#include <scopes/internal/PlacementHintImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

PlacementHint::PlacementHint(internal::PlacementHintImpl *pimpl)
    : p(std::shared_ptr<internal::PlacementHintImpl>(pimpl))
{
}

PlacementHint PlacementHint::search_bar_area()
{
    return PlacementHint(new internal::PlacementHintImpl(Placement::SearchBarArea));
}

PlacementHint PlacementHint::annotation_area()
{
    return PlacementHint(new internal::PlacementHintImpl(Placement::AnnotationArea));
}

PlacementHint PlacementHint::results_area(Category::SCPtr category)
{
    return PlacementHint(new internal::PlacementHintImpl(Placement::ResultsArea, category));
}

PlacementHint PlacementHint::category_area(Category::SCPtr category)
{
    return PlacementHint(new internal::PlacementHintImpl(Placement::CategoryArea, category));
}

PlacementHint::Placement PlacementHint::placement() const
{
    return p->placement();
}

Category::SCPtr PlacementHint::category() const
{
    return p->category();
}

VariantMap PlacementHint::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace api

} // namespace unity
