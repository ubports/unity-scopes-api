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
    : p(new internal::SearchMetadataImpl(locale, form_factor))
{
}

SearchMetadata::SearchMetadata(int cardinality, std::string const& locale, std::string const& form_factor)
    : p(new internal::SearchMetadataImpl(cardinality, locale, form_factor))
{
}

/// @cond
SearchMetadata::SearchMetadata(internal::SearchMetadataImpl *impl)
    : p(impl)
{
}

SearchMetadata::SearchMetadata(SearchMetadata const& other)
    : p(new internal::SearchMetadataImpl(*(other.p)))
{
}

SearchMetadata::SearchMetadata(SearchMetadata&&) = default;

SearchMetadata& SearchMetadata::operator=(SearchMetadata const& other)
{
    if (this != &other)
    {
        p.reset(new internal::SearchMetadataImpl(*(other.p)));
    }
    return *this;
}

SearchMetadata& SearchMetadata::operator=(SearchMetadata&&) = default;

SearchMetadata::~SearchMetadata() = default;

VariantMap SearchMetadata::serialize() const
{
    return p->serialize();
}

/// @endcond

std::string SearchMetadata::locale() const
{
    return p->locale();
}

std::string SearchMetadata::form_factor() const
{
    return p->form_factor();
}

void SearchMetadata::set_cardinality(int cardinality)
{
    p->set_cardinality(cardinality);
}

int SearchMetadata::cardinality() const
{
    return p->cardinality();
}

void SearchMetadata::set_hint(std::string const& key, Variant const& value)
{
    p->hint(key) = value;
}

VariantMap SearchMetadata::hints() const
{
    return p->hints();
}

Variant& SearchMetadata::operator[](std::string const& key)
{
    return p->hint(key);
}

Variant const& SearchMetadata::operator[](std::string const& key) const
{
    // force const hint() method
    return static_cast<internal::SearchMetadataImpl const *>(p.get())->hint(key);
}

bool SearchMetadata::contains_hint(std::string const& key) const
{
    return p->contains_hint(key);
}

void SearchMetadata::set_internet_connectivity(bool is_connected)
{
    p->set_internet_connectivity(is_connected);
}

std::shared_ptr<bool> SearchMetadata::internet_connectivity() const
{
    return p->internet_connectivity();
}

} // namespace scopes

} // namespace unity
