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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/QueryMetadata.h>
#include <unity/scopes/internal/QueryMetadataImpl.h>

namespace unity
{

namespace scopes
{

/// @cond

QueryMetadata::QueryMetadata(internal::QueryMetadataImpl* impl)
    : p(impl)
{
}

QueryMetadata::QueryMetadata(QueryMetadata const& other) = delete;
QueryMetadata::QueryMetadata(QueryMetadata&&) = default;
QueryMetadata& QueryMetadata::operator=(QueryMetadata const& other) = delete;
QueryMetadata& QueryMetadata::operator=(QueryMetadata&&) = default;

QueryMetadata::~QueryMetadata()
{
}

/// @endcond

std::string QueryMetadata::locale() const
{
    return p->locale();
}

std::string QueryMetadata::form_factor() const
{
    return p->form_factor();
}

void QueryMetadata::set_internet_connectivity(ConnectivityStatus connectivity_status)
{
    p->set_internet_connectivity(connectivity_status);
}

QueryMetadata::ConnectivityStatus QueryMetadata::internet_connectivity() const
{
    return p->internet_connectivity();
}

/// @cond

VariantMap QueryMetadata::serialize() const
{
    return p->serialize();
}

/// @endcond

} // namespace scopes

} // namespace unity
