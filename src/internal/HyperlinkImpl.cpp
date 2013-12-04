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

#include <scopes/internal/HyperlinkImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

HyperlinkImpl::HyperlinkImpl(Query const& query, PlacementHint const &placement)
    : AnnotationObjectBaseImpl(placement),
      query_(query)
{
}

Query HyperlinkImpl::canned_query() const
{
    return query_;
}

void HyperlinkImpl::serialize(VariantMap& vm) const
{
    vm["query"] = query_.serialize();
}

const char* HyperlinkImpl::type_string() const
{
    return "hyperlink";
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
