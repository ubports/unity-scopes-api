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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/CategorisedResultImpl.h>
#include <unity/scopes/internal/CategoryRegistry.h>

namespace unity
{

namespace scopes
{

//! @cond

CategorisedResult::CategorisedResult(Category::SCPtr category)
    : Result(new internal::CategorisedResultImpl(category))
{
}

CategorisedResult::CategorisedResult(std::shared_ptr<internal::ResultImpl> impl)
    : Result(impl)
{
}

CategorisedResult::CategorisedResult(Category::SCPtr category, const VariantMap &variant_map)
    : Result(new internal::CategorisedResultImpl(category, variant_map))
{
}

CategorisedResult::CategorisedResult(CategorisedResult&&) = default;
CategorisedResult& CategorisedResult::operator=(CategorisedResult&&) = default;

CategorisedResult::CategorisedResult(CategorisedResult const& other)
    : Result(new internal::CategorisedResultImpl(*(other.fwd())))
{
}

CategorisedResult& CategorisedResult::operator=(CategorisedResult const& other)
{
    if (this != &other)
    {
        p = std::make_shared<internal::CategorisedResultImpl>(*(other.fwd()));
    }
    return *this;
}

//! @endcond

Category::SCPtr CategorisedResult::category() const
{
    return fwd()->category();
}

internal::CategorisedResultImpl* CategorisedResult::fwd() const
{
    return dynamic_cast<internal::CategorisedResultImpl*>(p.get());
}

} // namespace scopes

} // namespace unity
