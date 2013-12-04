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

#include <scopes/Hyperlink.h>
#include <scopes/PlacementHint.h>
#include <scopes/internal/HyperlinkImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

Hyperlink::Hyperlink(Query const& query, PlacementHint const &placement)
    : AnnotationObjectBase(new internal::HyperlinkImpl(query, placement))
{
}

Query Hyperlink::canned_query() const
{
    return fwd()->canned_query();
}

internal::HyperlinkImpl* Hyperlink::fwd() const
{
    return dynamic_cast<internal::HyperlinkImpl*>(p.get());
}

} // namespace scopes

} // namespace api

} // namespace unity
