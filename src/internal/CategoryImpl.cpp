/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scopes/internal/CategoryImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

//! @cond

CategoryImpl::CategoryImpl(VariantMap const& variant_map)
{
    //TODO
}

CategoryImpl::CategoryImpl(std::string const& id, std::string const& renderer)
    : id_(id)
    , renderer_(renderer)
{
}

std::string const& CategoryImpl::id() const
{
    return id_;
}

std::shared_ptr<VariantMap> CategoryImpl::variant_map() const
{
    auto var = std::make_shared<VariantMap>();
    (*var)["id"] = id_;
    //TODO serialize renderer
    return var;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
