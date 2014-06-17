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

#ifndef UNITY_INTERNAL_RADIOBUTTONSFILTERIMPL_H
#define UNITY_INTERNAL_RADIOBUTTONSFILTERIMPL_H

#include <unity/scopes/internal/OptionSelectorFilterImpl.h>
#include <unity/scopes/RadioButtonsFilter.h>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace experimental;

class RadioButtonsFilterImpl : public OptionSelectorFilterImpl
{
public:
    RadioButtonsFilterImpl(std::string const& id, std::string const& label);
    RadioButtonsFilterImpl(VariantMap const& var);
    FilterOption::SCPtr active_option(FilterState const& filter_state) const;
    std::string filter_type() const override;
    static RadioButtonsFilter::SPtr create(VariantMap const& var);
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
