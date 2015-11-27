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
    ValueSliderFilterImpl(std::string const& id, int min, int max, int default_value, ValueSliderLabels const& labels);
    ValueSliderFilterImpl(VariantMap const& var);
    void set_default_value(int val);
    int min() const;
    int max() const;
    int default_value() const;
    std::string value_label_template() const;
    bool has_value(FilterState const& filter_state) const;
    int value(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, int value) const;
    ValueSliderLabels const& labels() const;
    static ValueSliderFilter::SPtr create(VariantMap const& var);
    static void update_state(FilterState& filter_state, std::string const& filter_id, int value);
    void validate_display_hints() const override;

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;
    void check_range(int val) const;

private:
    std::string label_template_;
    int min_;
    int max_;
    int default_val_;
    std::unique_ptr<ValueSliderLabels> labels_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
