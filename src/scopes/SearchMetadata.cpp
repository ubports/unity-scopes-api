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
SearchMetadata::SearchMetadata(internal::SearchMetadataImpl* impl)
    : QueryMetadata(impl)
{
}

SearchMetadata::SearchMetadata(SearchMetadata const& other)
    : QueryMetadata(new internal::SearchMetadataImpl(*static_cast<internal::SearchMetadataImpl*>(other.p.get())))
{
}

SearchMetadata::SearchMetadata(SearchMetadata&&) = default;

SearchMetadata& SearchMetadata::operator=(SearchMetadata const& other)
{
    if (this != &other)
    {
        p.reset(new internal::SearchMetadataImpl(*static_cast<internal::SearchMetadataImpl*>(other.p.get())));
    }
    return *this;
}

SearchMetadata& SearchMetadata::operator=(SearchMetadata&&) = default;

SearchMetadata::~SearchMetadata() = default;

/// @endcond

void SearchMetadata::set_cardinality(int cardinality)
{
    static_cast<internal::SearchMetadataImpl*>(p.get())->set_cardinality(cardinality);
}

int SearchMetadata::cardinality() const
{
    return static_cast<internal::SearchMetadataImpl*>(p.get())->cardinality();
}

void SearchMetadata::set_location(Location const& location)
{
    static_cast<internal::SearchMetadataImpl*>(p.get())->set_location(location);
}

Location SearchMetadata::location() const
{
    return static_cast<internal::SearchMetadataImpl*>(p.get())->location();
}

void SearchMetadata::set_hint(std::string const& key, Variant const& value)
{
    static_cast<internal::SearchMetadataImpl*>(p.get())->hint(key) = value;
}

VariantMap SearchMetadata::hints() const
{
    return static_cast<internal::SearchMetadataImpl*>(p.get())->hints();
}

Variant& SearchMetadata::operator[](std::string const& key)
{
    return static_cast<internal::SearchMetadataImpl*>(p.get())->hint(key);
}

Variant const& SearchMetadata::operator[](std::string const& key) const
{
    // force const hint() method
    return static_cast<internal::SearchMetadataImpl const*>(p.get())->hint(key);
}

bool SearchMetadata::contains_hint(std::string const& key) const
{
    return static_cast<internal::SearchMetadataImpl*>(p.get())->contains_hint(key);
}

} // namespace scopes

} // namespace unity
