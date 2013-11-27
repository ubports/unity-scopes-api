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

#include <scopes/Result.h>
#include <scopes/internal/ResultImpl.h>
#include <scopes/internal/CategoryRegistry.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Result::Result(Category::SCPtr category)
    : ResultItem(new internal::ResultImpl(category))
{
}

Result::Result(Category::SCPtr category, const VariantMap &variant_map)
    : ResultItem(new internal::ResultImpl(category, variant_map))
{
}

Result::Result(const VariantMap &variant_map, internal::CategoryRegistry const& reg)
    : ResultItem(new internal::ResultImpl(variant_map, reg))
{
}

Result::Result(Result&&) = default;
Result& Result::operator=(Result&&) = default;

Result::Result(Result const& other)
    : ResultItem(new internal::ResultImpl(*(other.fwd())))
{
}

Result& Result::operator=(Result const& other)
{
    if (this != &other)
    {
        p = std::make_shared<internal::ResultImpl>(*(other.fwd()));
    }
    return *this;
}

//! @endcond

Category::SCPtr Result::category() const
{
    return fwd()->category();
}

internal::ResultImpl* Result::fwd() const
{
    return dynamic_cast<internal::ResultImpl*>(p.get());
}

} // namespace scopes

} // namespace api

} // namespace unity
