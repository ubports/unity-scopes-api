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

RatingFilter::RatingFilter(internal::RatingFilterImpl *impl)
    : FilterBase(impl)
{
}

RatingFilter::SPtr RatingFilter::create(std::string const& id, std::string const& label, int top_rating)
{
    return std::shared_ptr<RatingFilter>(new RatingFilter(new internal::RatingFilterImpl(id, label, top_rating)));
}

RatingFilter::SPtr RatingFilter::create(std::string const& id, std::string const& label)
{
    return std::shared_ptr<RatingFilter>(new RatingFilter(new internal::RatingFilterImpl(id, label)));
}

FilterOption::SCPtr RatingFilter::add_option(std::string const& id, std::string const& label)
{
    fwd()->add_option(id, label);
}

void RatingFilter::set_on_icon(std::string const& on_icon)
{
    fwd()->set_on_icon(on_icon);
}

void RatingFilter::set_off_icon(std::string const& off_icon)
{
    fwd()->set_off_icon(off_icon);
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

void RatingFilter::update_state(FilterState& filter_state, int rating)
{
}

void RatingFilter::update_state(FilterState& filter_state, std::string const& filter_id, int rating)
{
}

internal::RatingFilterImpl* RatingFilter::fwd() const
{
    return dynamic_cast<internal::RatingFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity
