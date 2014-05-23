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

class ValueSliderFilterImpl : public FilterBaseImpl
{
public:
    ValueSliderFilterImpl(std::string const& id, std::string const& label_template, int min, int max);
    void set_slider_type(ValueSliderFilter::SliderType tp);
    ValueSliderFilter::SliderType slider_type() const;
    void set_default_value(int val);
    std::string label(FilterState const& filter_state) const;
    std::string label_template() const;
    bool has_value(FilterState const& filter_state) const;
    int get_value(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, int value) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, int value);

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;
    void check_range(int val) const;

private:
    std::string label_template_;
    ValueSliderFilter::SliderType slider_type_;
    int default_val_;
    int min_;
    int max_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
