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

#include <unity/scopes/internal/DateTimePickerFilterImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/FilterState.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

DateTimePickerFilterImpl::DateTimePickerFilterImpl(std::string const& id, DateTimePickerFilter::Mode mode)
    : FilterBaseImpl(id),
    m_mode(mode)
{
}

DateTimePickerFilterImpl::DateTimePickerFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

void DateTimePickerFilterImpl::set_time_label(std::string const& label)
{
    m_time_label = label;
}

void DateTimePickerFilterImpl::set_date_label(std::string const& label)
{
    m_date_label = label;
}

void DateTimePickerFilterImpl::set_minimum(std::chrono::system_clock::time_point const& date)
{
    m_min.reset(new std::chrono::system_clock::time_point(date));
}

void DateTimePickerFilterImpl::set_maximum(std::chrono::system_clock::time_point const& date)
{
    m_max.reset(new std::chrono::system_clock::time_point(date));
}

std::string DateTimePickerFilterImpl::time_label() const
{
    return m_time_label;
}

std::string DateTimePickerFilterImpl::date_label() const
{
    return m_date_label;
}

DateTimePickerFilter::Mode DateTimePickerFilterImpl::mode() const
{
    return m_mode;
}

bool DateTimePickerFilterImpl::has_minimum() const
{
    return m_min != nullptr;
}

bool DateTimePickerFilterImpl::has_maximum() const
{
    return m_max != nullptr;
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::minimum() const
{
    if (m_min)
    {
        return *m_min;
    }

    throw LogicException("DateTimePickerFilter::minimum(): minimum value is not set");
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::maximum() const
{
    if (m_max)
    {
        return *m_max;
    }
    throw LogicException("DateTimePickerFilter::maximum(): maximum value is not set");
}

bool DateTimePickerFilterImpl::has_selected_date(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        try
        {
            auto const var = FilterBaseImpl::get(filter_state, id()).get_dict(); // this can throw if of different type
            return var.find("date") != var.end();
        }
        catch (...)
        {
        }
    }
    return false;
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::selected_date(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        bool valid = true;
        try
        {
            auto vm = FilterBaseImpl::get(filter_state, id()).get_dict();
            auto it = vm.find("date");
            if (it != vm.end())
            {
                auto date = to_timepoint(it->second.get_int64_t());
                if (is_valid_date(date))
                {
                    return date;
                }
                valid = false;
            }
        }
        catch (...)
        {
        }
        if (!valid)
        {
            throw LogicException("DateTimePickerFilter::selected_date(): date for filter '" + id() + "' out of allowed range");
        }
    }
    throw unity::scopes::NotFoundException("DateTimePickerFilter::selected_date(): date not set for filter ", id());
}

void DateTimePickerFilterImpl::update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const
{
    if (!is_valid_date(date))
    {
        throw LogicException("DateTimePickerFilter::update_state(): date for filter '" + id() + "' out of allowed range");
    }

    update_state(filter_state, id(), date);
}

void DateTimePickerFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date)
{
    if (filter_id.empty())
    {
        throw InvalidArgumentException("DateTimePickerFilter::update_state(): Invalid empty filter_id string");
    }

    VariantMap& state = FilterBaseImpl::get(filter_state);
    VariantMap vm;
    vm["date"] = to_seconds_from_epoch(date);
    state[filter_id] = vm;
}

bool DateTimePickerFilterImpl::is_valid_date(std::chrono::system_clock::time_point const& date) const
{
    return ((m_min == nullptr || date >= *m_min - std::chrono::seconds(1)) && (m_max == nullptr || date < *m_max + std::chrono::seconds(1)));
}

void DateTimePickerFilterImpl::validate_display_hints(int hints) const
{
    if (hints & FilterBase::DisplayHints::Primary)
    {
        throw unity::InvalidArgumentException("set_display_hints(): Primary navigation flag is not supported by " + filter_type() + " filters");
    }
}

void DateTimePickerFilterImpl::serialize(VariantMap& var) const
{
    var["mode"] = static_cast<int>(m_mode.to_ulong());
    if (m_time_label.size())
    {
        var["time_label"] = m_time_label;
    }
    if (m_date_label.size())
    {
        var["date_label"] = m_date_label;
    }
    if (m_min)
    {
        var["min"] = to_seconds_from_epoch(*m_min);
    }
    if (m_max)
    {
        var["max"] = to_seconds_from_epoch(*m_max);
    }
}

void DateTimePickerFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("DateTimePickerFilter::deserialize()", var, "mode");
    m_mode = DateTimePickerFilter::Mode(it->second.get_int());
    it = var.find("time_label");
    if (it != var.end())
    {
        m_time_label = it->second.get_string();
    }
    else
    {
        m_time_label.clear();
    }

    it = var.find("date_label");
    if (it != var.end())
    {
        m_date_label = it->second.get_string();
    }
    else
    {
        m_date_label.clear();
    }

    it = var.find("min");
    if (it != var.end())
    {
        set_minimum(to_timepoint(it->second.get_int64_t()));
    }

    it = var.find("max");
    if (it != var.end())
    {
        set_maximum(to_timepoint(it->second.get_int64_t()));
    }
}

DateTimePickerFilter::SPtr DateTimePickerFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<DateTimePickerFilter>(new DateTimePickerFilter(new DateTimePickerFilterImpl(var)));
}

std::string DateTimePickerFilterImpl::filter_type() const
{
    return "datetime_picker";
}

int64_t DateTimePickerFilterImpl::to_seconds_from_epoch(std::chrono::system_clock::time_point const& tp)
{
    auto dur = tp.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(dur).count();
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::to_timepoint(int64_t seconds_from_epoch)
{
    std::chrono::seconds const secs(seconds_from_epoch);
    return std::chrono::system_clock::time_point(secs);
}

}

}

}
