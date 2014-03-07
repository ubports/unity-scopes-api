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

#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/internal/SearchMetadataImpl.h>

namespace unity
{

namespace scopes
{

SearchMetadata::SearchMetadata(std::string const& locale, std::string const& form_factor)
    : QueryMetadata(new internal::SearchMetadataImpl(locale, form_factor))
{
}

SearchMetadata::SearchMetadata(int cardinality, std::string const& locale, std::string const& form_factor)
    : QueryMetadata(new internal::SearchMetadataImpl(cardinality, locale, form_factor))
{
}

/// @cond
SearchMetadata::SearchMetadata(internal::SearchMetadataImpl *impl)
    : QueryMetadata(impl)
{
}

SearchMetadata::SearchMetadata(SearchMetadata const& other)
    : QueryMetadata(new internal::SearchMetadataImpl(*(other.fwd())))
{
}

SearchMetadata::SearchMetadata(SearchMetadata&&) = default;

SearchMetadata& SearchMetadata::operator=(SearchMetadata const &other)
{
    if (this != &other)
    {
        p.reset(new internal::SearchMetadataImpl(*(other.fwd())));
    }
    return *this;
}

SearchMetadata& SearchMetadata::operator=(SearchMetadata&&) = default;

SearchMetadata::~SearchMetadata() = default;
/// @endcond

void SearchMetadata::set_cardinality(int cardinality)
{
    fwd()->set_cardinality(cardinality);
}

int SearchMetadata::cardinality() const
{
    return fwd()->cardinality();
}

void SearchMetadata::set_hint(std::string const& key, Variant const& value)
{
    fwd()->hint(key) = value;
}

VariantMap SearchMetadata::hints() const
{
    return fwd()->hints();
}

Variant& SearchMetadata::operator[](std::string const& key)
{
    return fwd()->hint(key);
}

Variant const& SearchMetadata::operator[](std::string const& key) const
{
    return fwd()->hint(key);
}

bool SearchMetadata::contains_hint(std::string const& key) const
{
    return fwd()->contains_hint(key);
}

internal::SearchMetadataImpl* SearchMetadata::fwd() const
{
    return dynamic_cast<internal::SearchMetadataImpl*>(p.get());
}

} // namespace scopes

} // namespace unity
