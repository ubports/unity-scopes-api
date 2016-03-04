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

#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>

namespace unity
{

namespace scopes
{

RangeInputFilter::SPtr RangeInputFilter::create(std::string const& id,
        std::string const& start_prefix_label, std::string const& start_postfix_label,
        std::string const& central_label,
        std::string const& end_prefix_label, std::string const& end_postfix_label)
{
    return std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new internal::RangeInputFilterImpl(id,
                    Variant::null(), Variant::null(),
                    start_prefix_label, start_postfix_label,
                    central_label,
                    end_prefix_label, end_postfix_label)));
}

RangeInputFilter::SPtr RangeInputFilter::create(std::string const& id,
        Variant const& default_start_value,
        Variant const& default_end_value,
        std::string const& start_prefix_label, std::string const& start_postfix_label,
        std::string const& central_label,
        std::string const& end_prefix_label, std::string const& end_postfix_label)
{
    return std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new internal::RangeInputFilterImpl(id,
                    default_start_value, default_end_value,
                    start_prefix_label, start_postfix_label,
                    central_label,
                    end_prefix_label, end_postfix_label)));
}

RangeInputFilter::SPtr RangeInputFilter::create(std::string const& id,
        std::string const& start_prefix_label, std::string const& start_postfix_label,
        std::string const& central_label,
        std::string const& end_prefix_label, std::string const& end_postfix_label,
        FilterGroup::SCPtr const& group
        )
{
    auto filter = std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new internal::RangeInputFilterImpl(id,
                    Variant::null(), Variant::null(),
                    start_prefix_label, start_postfix_label,
                    central_label,
                    end_prefix_label, end_postfix_label)));
    filter->p->add_to_filter_group(group);
    return filter;
}

RangeInputFilter::SPtr RangeInputFilter::create(std::string const& id,
        Variant const& default_start_value,
        Variant const& default_end_value,
        std::string const& start_prefix_label, std::string const& start_postfix_label,
        std::string const& central_label,
        std::string const& end_prefix_label, std::string const& end_postfix_label,
        FilterGroup::SCPtr const& group
        )
{
    auto filter = std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new internal::RangeInputFilterImpl(id,
                    default_start_value, default_end_value,
                    start_prefix_label, start_postfix_label,
                    central_label,
                    end_prefix_label, end_postfix_label)));
    filter->p->add_to_filter_group(group);
    return filter;
}


RangeInputFilter::RangeInputFilter(internal::RangeInputFilterImpl* impl)
    : FilterBase(impl)
{
}

std::string RangeInputFilter::start_prefix_label() const
{
    return fwd()->start_prefix_label();
}

std::string RangeInputFilter::start_postfix_label() const
{
    return fwd()->start_postfix_label();
}

std::string RangeInputFilter::end_prefix_label() const
{
    return fwd()->end_prefix_label();
}

std::string RangeInputFilter::end_postfix_label() const
{
    return fwd()->end_postfix_label();
}

std::string RangeInputFilter::central_label() const
{
    return fwd()->central_label();
}

Variant RangeInputFilter::default_start_value() const
{
    return fwd()->default_start_value();
}

Variant RangeInputFilter::default_end_value() const
{
    return fwd()->default_end_value();
}

bool RangeInputFilter::has_start_value(FilterState const& filter_state) const
{
    return fwd()->has_start_value(filter_state);
}

bool RangeInputFilter::has_end_value(FilterState const& filter_state) const
{
    return fwd()->has_end_value(filter_state);
}

double RangeInputFilter::start_value(FilterState const& filter_state) const
{
    return fwd()->start_value(filter_state);
}

double RangeInputFilter::end_value(FilterState const& filter_state) const
{
    return fwd()->end_value(filter_state);
}

void RangeInputFilter::update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const
{
    fwd()->update_state(filter_state, start_value, end_value);
}

void RangeInputFilter::update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value)
{
    internal::RangeInputFilterImpl::update_state(filter_state, filter_id, start_value, end_value);
}

internal::RangeInputFilterImpl* RangeInputFilter::fwd() const
{
    return dynamic_cast<internal::RangeInputFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity
