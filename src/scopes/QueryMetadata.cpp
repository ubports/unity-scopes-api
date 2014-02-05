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

#include <unity/scopes/QueryMetadata.h>
#include <unity/scopes/internal/QueryMetadataImpl.h>

namespace unity
{

namespace scopes
{

QueryMetadata::QueryMetadata(QueryMetadata const& other)
{
    p.reset(new internal::QueryMetadataImpl(*(other.p)));
}

QueryMetadata::QueryMetadata(internal::QueryMetadataImpl *impl)
    : p(impl)
{
}

QueryMetadata::QueryMetadata(std::string const& locale, std::string const& form_factor)
    : p(new internal::QueryMetadataImpl(locale, form_factor))
{
}

QueryMetadata::QueryMetadata(QueryMetadata&&) = default;

QueryMetadata& QueryMetadata::operator=(QueryMetadata&&) = default;

QueryMetadata::~QueryMetadata()
{
}

std::string QueryMetadata::locale() const
{
    return p->locale();
}

std::string QueryMetadata::form_factor() const
{
    return p->form_factor();
}

VariantMap QueryMetadata::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace unity
