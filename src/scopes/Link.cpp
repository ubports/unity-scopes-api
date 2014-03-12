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


#include <unity/scopes/Link.h>
#include <unity/scopes/internal/LinkImpl.h>

namespace unity
{

namespace scopes
{

Link::Link(std::string const& label, CannedQuery const& query)
    : p(new internal::LinkImpl(label, query))
{
}

/// @cond
Link::Link(VariantMap const& variant_map)
    : p(new internal::LinkImpl(variant_map))
{
}

Link::Link(Link&&) = default;

Link::Link(Link const& other)
    : p(new internal::LinkImpl(*(other.p)))
{
}

Link& Link::operator=(Link const& other)
{
    if (this != &other)
    {
        p.reset(new internal::LinkImpl(*(other.p)));
    }
    return *this;
}

Link::~Link() = default;

VariantMap Link::serialize() const
{
    return p->serialize();
}
/// @endcond

std::string Link::label() const
{
    return p->label();
}

CannedQuery Link::query() const
{
    return p->query();
}

} // namespace scopes

} // namespace unity
