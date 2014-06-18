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

#ifndef UNITY_INTERNAL_VALUESLIDERFILTERIMPL_H
#define UNITY_INTERNAL_VALUESLIDERFILTERIMPL_H

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/ValueSliderFilter.h>

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
    ValueSliderFilterImpl(std::string const& id, std::string const& label, std::string const& label_template, double min, double max);
    ValueSliderFilterImpl(VariantMap const& var);
    void set_slider_type(ValueSliderFilter::SliderType tp);
    ValueSliderFilter::SliderType slider_type() const;
    void set_default_value(double val);
    std::string label() const;
    double min() const;
    double max() const;
    double default_value() const;
    std::string value_label_template() const;
    bool has_value(FilterState const& filter_state) const;
    double value(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, double value) const;
    static ValueSliderFilter::SPtr create(VariantMap const& var);
    static void update_state(FilterState& filter_state, std::string const& filter_id, double value);

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;
    void check_range(double val) const;

private:
    std::string label_;
    std::string label_template_;
    ValueSliderFilter::SliderType slider_type_;
    double default_val_;
    double min_;
    double max_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
