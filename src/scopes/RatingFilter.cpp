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

#include <unity/scopes/RatingFilter.h>
#include <unity/scopes/internal/RatingFilterImpl.h>

namespace unity
{

namespace scopes
{

namespace experimental
{

RatingFilter::RatingFilter(internal::RatingFilterImpl *impl)
    : FilterBase(impl)
{
}

RatingFilter::UPtr RatingFilter::create(std::string const& id, std::string const& label, int top_rating)
{
    return std::unique_ptr<RatingFilter>(new RatingFilter(new internal::RatingFilterImpl(id, label, top_rating)));
}

RatingFilter::UPtr RatingFilter::create(std::string const& id, std::string const& label)
{
    return std::unique_ptr<RatingFilter>(new RatingFilter(new internal::RatingFilterImpl(id, label)));
}

FilterOption::SCPtr RatingFilter::add_option(std::string const& id, std::string const& label)
{
    return fwd()->add_option(id, label);
}

void RatingFilter::set_on_icon(std::string const& on_icon)
{
    fwd()->set_on_icon(on_icon);
}

void RatingFilter::set_off_icon(std::string const& off_icon)
{
    fwd()->set_off_icon(off_icon);
}

std::string RatingFilter::label() const
{
    return fwd()->label();
}

std::list<FilterOption::SCPtr> RatingFilter::options() const
{
    return fwd()->options();
}

std::string RatingFilter::on_icon() const
{
    return fwd()->on_icon();
}

std::string RatingFilter::off_icon() const
{
    return fwd()->off_icon();
}

FilterOption::SCPtr RatingFilter::active_rating(FilterState const& filter_state) const
{
    return fwd()->active_option(filter_state);
}

bool RatingFilter::has_active_rating(FilterState const& filter_state) const
{
    return fwd()->has_active_option(filter_state);
}

void RatingFilter::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const
{
    fwd()->update_state(filter_state, option, active);
}

void RatingFilter::update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value)
{
    internal::RatingFilterImpl::update_state(filter_state, filter_id, option_id, value);
}

internal::RatingFilterImpl* RatingFilter::fwd() const
{
    return dynamic_cast<internal::RatingFilterImpl*>(p.get());
}

} // namespace experimental

} // namespace scopes

} // namespace unity
