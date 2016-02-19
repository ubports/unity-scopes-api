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
#include <unity/scopes/RangeInputFilter.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RangeInputFilterImpl : public FilterBaseImpl
{
public:
    RangeInputFilterImpl(std::string const& id,
            Variant const& default_start_value, Variant const& default_end_value,
            std::string const& start_prefix_label, std::string const& start_postfix_label,
            std::string const& central_label,
            std::string const& end_prefix_label, std::string const& end_postfix_label);

    RangeInputFilterImpl(VariantMap const& var);

    std::string start_prefix_label() const;
    std::string start_postfix_label() const;
    std::string end_prefix_label() const;
    std::string end_postfix_label() const;
    std::string central_label() const;

    Variant default_start_value() const;
    Variant default_end_value() const;
    bool has_start_value(FilterState const& filter_state) const;
    bool has_end_value(FilterState const& filter_state) const;
    double start_value(FilterState const& filter_state) const;
    double end_value(FilterState const& filter_state) const;

    void update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value);

    std::string filter_type() const override;
    void validate_display_hints() const override;

    static RangeInputFilter::SPtr create(VariantMap const& var);

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);

private:
    bool has_value(FilterState const& filter_state, unsigned int index) const;
    double get_value(FilterState const& filter_state, unsigned int index) const;
    static void check_type(Variant const& val, std::string const& filter_id, std::string const& varname);

    Variant default_start_value_;
    Variant default_end_value_;
    std::string start_prefix_label_;
    std::string start_postfix_label_;
    std::string end_prefix_label_;
    std::string end_postfix_label_;
    std::string central_label_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
