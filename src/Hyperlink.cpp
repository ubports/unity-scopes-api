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
#include <scopes/internal/HyperlinkImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

Hyperlink::Hyperlink(std::string const& label, Query const& query)
    : p(new internal::HyperlinkImpl(label, query))
{
}

Hyperlink::Hyperlink(VariantMap const& variant_map)
    : p(new internal::HyperlinkImpl(variant_map))
{
}

std::string Hyperlink::label() const
{
    return p->label();
}

Query Hyperlink::query() const
{
    return p->query();
}

VariantMap Hyperlink::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace api

} // namespace unity
