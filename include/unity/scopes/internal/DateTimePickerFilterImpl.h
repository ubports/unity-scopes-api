/*
 * Copyright (C) 2015 Canonical Ltd
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
#include <unity/scopes/DateTimePickerFilter.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class DateTimePickerFilterImpl: public FilterBaseImpl
{
public:
    DateTimePickerFilterImpl(std::string const& id, DateTimePickerFilter::Mode mode);

    void set_time_label(std::string const& label);
    void set_date_label(std::string const& label);

    void set_minimum(std::chrono::system_clock::time_point const& date);
    void set_maximum(std::chrono::system_clock::time_point const& date);

    std::string time_label() const;
    std::string date_label() const;

    std::chrono::system_clock::time_point minimum() const;
    std::chrono::system_clock::time_point maximum() const;

    bool has_selected_date(FilterState const& filter_state) const;
    std::chrono::system_clock::time_point selected_date(FilterState const& filter_state) const;

    void update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const;

    static void update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date);

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;
};

}

}

}
