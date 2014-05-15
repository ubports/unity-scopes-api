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
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

RatingFilterImpl::RatingFilterImpl(std::string const& id, std::string const& label)
    : RadioButtonsFilterImpl(id, label)
{
}

RatingFilterImpl::RatingFilterImpl(std::string const& id, std::string const& label, int top_rating)
    : RadioButtonsFilterImpl(id, label)
{
    if (top_rating < 2 || top_rating > max_rating)
    {
        std::stringstream err;
        err << "RatingFilterImpl(): top_rating " << top_rating << " outside of allowed range for filter '" << id << "'";
        throw unity::LogicException(err.str());
    }

    // create rating options from 1..top_rating range, e.g. "1+", "2+", ..., "5".
    for (int i = 1; i <= top_rating; i++)
    {
        auto const num = std::to_string(i);
        RadioButtonsFilterImpl::add_option(num, i < top_rating ? num + "+" : num);
    }
}

RatingFilterImpl::RatingFilterImpl(VariantMap const& var)
    : RadioButtonsFilterImpl(var)
{
}

FilterOption::SCPtr RatingFilterImpl::add_option(std::string const& id, std::string const& label)
{
    if (num_of_options() < max_rating)
    {
        return RadioButtonsFilterImpl::add_option(id, label);
    }
    else
    {
        std::stringstream err;
        err << "RatingFilterImpl::add_option(): maximum number of rating options reached for filter '" << id << "'";
        throw unity::LogicException(err.str());
    }
}

void RatingFilterImpl::set_on_icon(std::string const& on_icon)
{
    on_icon_ = on_icon;
}

void RatingFilterImpl::set_off_icon(std::string const& off_icon)
{
    off_icon_ = off_icon;
}

std::string RatingFilterImpl::on_icon() const
{
    return on_icon_;
}

std::string RatingFilterImpl::off_icon() const
{
    return off_icon_;
}

RatingFilter::SPtr RatingFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<RatingFilter>(new RatingFilter(new RatingFilterImpl(var)));
}

std::string RatingFilterImpl::filter_type() const
{
    return "rating";
}

} // namespace internal

} // namespace scopes

} // namespace unity
