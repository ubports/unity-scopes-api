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

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/internal/OptionSelectorFilterImpl.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

FilterBaseImpl::FilterBaseImpl(std::string const& id)
    : id_(id),
      display_hints_(FilterBase::DisplayHints::Default)
{
}

FilterBaseImpl::FilterBaseImpl(VariantMap const& var)
{
    auto it = find_or_throw("FilterBase()", var, "id");
    id_ = it->second.get_string();
    it = var.find("display_hints");
    if (it != var.end())
    {
        display_hints_ = static_cast<FilterBase::DisplayHints>(it->second.get_int());
    }
}

FilterBaseImpl::~FilterBaseImpl() = default;

void FilterBaseImpl::set_display_hints(int hints)
{
    display_hints_ = hints;
}

std::string FilterBaseImpl::id() const
{
    return id_;
}

int FilterBaseImpl::display_hints() const
{
    return display_hints_;
}

VariantMap FilterBaseImpl::serialize() const
{
    VariantMap vm;
    vm["id"] = id_;
    if (display_hints_ != FilterBase::DisplayHints::Default)
    {
        vm["display_hints"] = static_cast<int>(display_hints_);
    }
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

FilterBase::SCPtr FilterBaseImpl::deserialize(VariantMap const& var)
{
    auto it = var.find("filter_type");
    if (it != var.end())
    {
        auto ftype = it->second.get_string();
        if (ftype == "option_selector")
        {
            return OptionSelectorFilterImpl::create(var);
        }
        if (ftype == "range_input")
        {
            return RangeInputFilterImpl::create(var);
        }
        throw unity::LogicException("Unknown filter type: " + ftype);
    }
    throw unity::LogicException("FilterBase: Missing 'filter_type'");
}

VariantArray FilterBaseImpl::serialize_filters(Filters const& filters)
{
    VariantArray var;
    for (auto const& f: filters)
    {
        var.push_back(Variant(f->serialize()));
    }
    return var;
}

Filters FilterBaseImpl::deserialize_filters(VariantArray const& var)
{
    Filters filters;
    for (auto const& f: var)
    {
        filters.push_back(FilterBaseImpl::deserialize(f.get_dict()));
    }
    return filters;
}

} // namespace internal

} // namespace scopes

} // namespace unity
