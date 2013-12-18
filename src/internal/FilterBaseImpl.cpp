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

#include <scopes/internal/FilterBaseImpl.h>
#include <scopes/FilterState.h>
#include <scopes/internal/FilterStateImpl.h>
#include <scopes/OptionSelectorFilter.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

FilterBaseImpl::FilterBaseImpl(std::string const& id)
    : id_(id)
{
}

FilterBaseImpl::FilterBaseImpl(VariantMap const& var)
{
    auto it = var.find("id");
    if (it == var.end())
    {
        throw unity::LogicException("FilterBase: missing 'id'");
    }
    id_ = it->second.get_string();
}

FilterBaseImpl::~FilterBaseImpl() = default;

std::string FilterBaseImpl::id() const
{
    return id_;
}

VariantMap FilterBaseImpl::serialize() const
{
    VariantMap vm;
    vm["id"] = id_;
    vm["filter_type"] = filter_type();
    serialize(vm);
    return vm;
}

Variant FilterBaseImpl::get(FilterState const& filter_state, std::string const& filter_id)
{
    return filter_state.p->get(filter_id);
}

VariantMap& FilterBaseImpl::get(FilterState const& filter_state)
{
    return filter_state.p->get();
}

FilterBase FilterBaseImpl::deserialize(VariantMap const& var)
{
    auto it = var.find("filter_type");
    if (it != var.end())
    {
        auto ftype = it->second.get_string();
        if (ftype == "option_selector")
        {
            OptionSelectorFilter filter(var);
            return filter;
        }
        throw unity::LogicException("Unknown filter type: " + ftype);
    }
    throw unity::LogicException("FilterBase: Missing 'filter_type'");
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
