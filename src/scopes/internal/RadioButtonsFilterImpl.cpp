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

#include <unity/scopes/internal/RadioButtonsFilterImpl.h>
#include <unity/scopes/FilterState.h>
#include <sstream>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

RadioButtonsFilterImpl::RadioButtonsFilterImpl(std::string const& id, std::string const& label)
    : OptionSelectorFilterImpl(id, label, false)
{
}

RadioButtonsFilterImpl::RadioButtonsFilterImpl(VariantMap const& var)
    : OptionSelectorFilterImpl(var)
{
    if (multi_select()) // this should never happen unless a client bypasses the api
    {
        std::stringstream err;
        err << "RadioButtonsFilterImpl(): invalid filter data, multi selection is not available with RadioButtonsFilter, filter id '" << id() << "'";
        throw unity::LogicException(err.str());
    }
}

FilterOption::SCPtr RadioButtonsFilterImpl::active_option(FilterState const& filter_state) const
{
    auto const active = active_options(filter_state);
    if (active.size() > 1) // this should never happen unless a client bypasses the api
    {
        std::stringstream err;
        err << "RadioButtonsFilterImpl::active_option(): internal error, more than one button active, filter id '" << id() << "'";
        throw unity::LogicException(err.str());
    }

    if (active.size() == 1)
    {
        return *active.begin();
    }

    return nullptr;
}

std::string RadioButtonsFilterImpl::filter_type() const
{
    return "radio_buttons";
}

void RadioButtonsFilterImpl::validate_display_hints(int hints) const
{
    OptionSelectorFilterImpl::validate_display_hints(hints);
    if (hints & FilterBase::DisplayHints::Primary)
    {
        throw unity::InvalidArgumentException("set_display_hints(): Primary navigation flag is not supported by " + filter_type() + " filters");
    }
}

RadioButtonsFilter::SPtr RadioButtonsFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<RadioButtonsFilter>(new RadioButtonsFilter(new RadioButtonsFilterImpl(var)));
}

} // namespace internal

} // namespace scopes

} // namespace unity
