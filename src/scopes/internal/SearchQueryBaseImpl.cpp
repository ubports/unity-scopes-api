/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/internal/SearchQueryBaseImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

SearchQueryBaseImpl::SearchQueryBaseImpl(CannedQuery const& query, SearchMetadata const& metadata)
    : QueryBaseImpl(),
      canned_query_(query),
      search_metadata_(metadata)
{
}

} // namespace internal

} // namespace scopes

} // namespace unity
