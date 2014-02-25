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

#include <unity/scopes/VariantBuilder.h>
#include <unity/scopes/internal/VariantBuilderImpl.h>

namespace unity
{

namespace scopes
{

/// @cond
VariantBuilder::VariantBuilder()
    : p(new internal::VariantBuilderImpl())
{
}

VariantBuilder::VariantBuilder(VariantBuilder const& other)
    : p(new internal::VariantBuilderImpl(*(other.p)))
{
}

VariantBuilder::VariantBuilder(VariantBuilder&&) = default;
VariantBuilder& VariantBuilder::operator=(VariantBuilder&&) = default;

VariantBuilder& VariantBuilder::operator=(VariantBuilder const& other)
{
    if (this != &other)
    {
        p.reset(new internal::VariantBuilderImpl(*(other.p)));
    }
    return *this;
}

VariantBuilder::~VariantBuilder() = default;
/// @endcond

void VariantBuilder::add_tuple(std::initializer_list<std::pair<std::string, Variant>> const& tuple)
{
    p->add_tuple(tuple);
}

void VariantBuilder::add_tuple(std::vector<std::pair<std::string, Variant>> const& tuple)
{
    p->add_tuple(tuple);
}

Variant VariantBuilder::end()
{
    return p->end();
}

} // namespace scopes

} // namespace unity
