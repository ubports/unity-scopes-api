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

#pragma once

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/internal/ValueSliderLabelsImpl.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{

using namespace experimental;

class ValueSliderFilterImpl : public FilterBaseImpl
{
public:
    ValueSliderFilterImpl(std::string const& id, double min, double max, double default_value, ValueSliderLabels const& labels);
    ValueSliderFilterImpl(VariantMap const& var);
    void set_default_value(double val);
    double min() const;
    double max() const;
    double default_value() const;
    bool has_value(FilterState const& filter_state) const;
    double value(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, double value) const;
    ValueSliderLabels const& labels() const;
    static ValueSliderFilter::SPtr create(VariantMap const& var);
    static void update_state(FilterState& filter_state, std::string const& filter_id, double value);
    void validate_display_hints() const override;

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;
    void check_range(double val) const;

private:
    double min_;
    double max_;
    double default_val_;
    std::unique_ptr<ValueSliderLabels> labels_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
