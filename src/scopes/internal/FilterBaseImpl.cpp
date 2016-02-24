/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/internal/OptionSelectorFilterImpl.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>
#include <unity/scopes/internal/RadioButtonsFilterImpl.h>
#include <unity/scopes/internal/RatingFilterImpl.h>
#include <unity/scopes/internal/SwitchFilterImpl.h>
#include <unity/scopes/internal/ValueSliderFilterImpl.h>
#include <unity/scopes/internal/DateTimePickerFilterImpl.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

FilterBaseImpl::FilterBaseImpl(std::string const& id)
    : id_(id),
      display_hints_(FilterBase::DisplayHints::Default)
{
    if (id_.empty())
    {
        throw InvalidArgumentException("FilterBase(): invalid empty id string");
    }
}

FilterBaseImpl::FilterBaseImpl(VariantMap const& var)
    : display_hints_(FilterBase::DisplayHints::Default)
{
    auto it = find_or_throw("FilterBase()", var, "id");
    id_ = it->second.get_string();
    it = var.find("title");
    if (it != var.end())
    {
        title_ = it->second.get_string();
    }

    it = var.find("display_hints");
    if (it != var.end())
    {
        switch (it->second.which())
        {
            case Variant::Type::Int:
                set_display_hints(static_cast<FilterBase::DisplayHints>(it->second.get_int()));
                break;
            // when receiving filter state from the server, strings are used
            case Variant::Type::String:
                set_display_hints(it->second.get_string() == "primary" ?
                    FilterBase::DisplayHints::Primary : FilterBase::DisplayHints::Default);
                break;
            default:
                break;
        }
    }
}

FilterBaseImpl::~FilterBaseImpl() = default;

void FilterBaseImpl::add_to_filter_group(FilterGroup::SCPtr const& group)
{
    // Filter can only belong to one group. The API guarantees this by only allowing group to be passed at Filter's construction time,
    // so this assert should never be reached.
    assert(filter_group_);
    if (!group)
    {
        throw unity::InvalidArgumentException("FilterBaseImpl::add_to_filter_group(): invalid null group");
    }
    filter_group_ = group;
}

void FilterBaseImpl::set_display_hints(int hints)
{
    // note: make sure all_flags is updated whenever new values are added to the DisplayHints enum
    static const int all_flags = static_cast<int>(FilterBase::DisplayHints::Primary);
    if (hints < 0 || hints > all_flags)
    {
        std::stringstream err;
        err << "FilterBaseImpl::set_display_hints(): Invalid display hint for filter '" << id_ << "'";
        throw unity::InvalidArgumentException(err.str());
    }
    display_hints_ = hints;
}

std::string FilterBaseImpl::id() const
{
    return id_;
}

int FilterBaseImpl::display_hints() const
{
    return display_hints_;
}

void FilterBaseImpl::set_title(std::string const& title)
{
    title_ = title;
}

std::string FilterBaseImpl::title() const
{
    return title_;
}

VariantMap FilterBaseImpl::serialize() const
{
    VariantMap vm;
    vm["id"] = id_;
    if (title_.size() > 0)
    {
        vm["title"] = title_;
    }
    if (display_hints_ != FilterBase::DisplayHints::Default)
    {
        vm["display_hints"] = static_cast<int>(display_hints_);
    }
    vm["filter_type"] = filter_type();
    serialize(vm);
    return vm;
}

Variant FilterBaseImpl::get(FilterState const& filter_state, std::string const& filter_id)
{
    return filter_state.p->get(filter_id);
}

VariantMap& FilterBaseImpl::get(FilterState const& filter_state)
{
    return filter_state.p->get();
}

FilterBase::SCPtr FilterBaseImpl::deserialize(VariantMap const& var)
{
    auto it = var.find("filter_type");
    if (it != var.end())
    {
        auto ftype = it->second.get_string();
        if (ftype == "option_selector")
        {
            return OptionSelectorFilterImpl::create(var);
        }
        if (ftype == "range_input")
        {
            return RangeInputFilterImpl::create(var);
        }
        if (ftype == "radio_buttons")
        {
            return RadioButtonsFilterImpl::create(var);
        }
        if (ftype == "rating")
        {
            return RatingFilterImpl::create(var);
        }
        if (ftype == "switch")
        {
            return SwitchFilterImpl::create(var);
        }
        if (ftype == "value_slider")
        {
            return ValueSliderFilterImpl::create(var);
        }
        if (ftype == "datetime_picker")
        {
            return DateTimePickerFilterImpl::create(var);
        }
        throw unity::LogicException("Unknown filter type: " + ftype);
    }
    throw unity::LogicException("FilterBase: Missing 'filter_type'");
}

VariantArray FilterBaseImpl::serialize_filters(Filters const& filters)
{
    VariantArray var;
    for (auto const& f: filters)
    {
        var.push_back(Variant(f->serialize()));
    }
    return var;
}

Filters FilterBaseImpl::deserialize_filters(VariantArray const& var)
{
    Filters filters;
    for (auto const& f: var)
    {
        filters.push_back(FilterBaseImpl::deserialize(f.get_dict()));
    }
    return filters;
}

void FilterBaseImpl::validate_filters(Filters const& filters)
{
    for (auto const& f: filters)
    {
        if (f == nullptr)
        {
            throw unity::LogicException("FilterBaseImpl::validate_filters(): invalid null filter pointer");
        }
        {
            OptionSelectorFilter::SCPtr optsel = std::dynamic_pointer_cast<OptionSelectorFilter const>(f);
            if (optsel)
            {
                if (optsel->options().size() == 0)
                {
                    std::stringstream err;
                    err << "FilterBaseImpl::validate_filters(): invalid empty OptionSelectorFilter '" << f->id() << "'";
                    throw unity::LogicException(err.str());
                }
                continue;
            }
        }
        {
            RatingFilter::SCPtr rating = std::dynamic_pointer_cast<RatingFilter const>(f);
            if (rating)
            {
                if (rating->options().size() == 0)
                {
                    std::stringstream err;
                    err << "FilterBaseImpl::validate_filters(): invalid empty RatingFilter '" << f->id() << "'";
                    throw unity::LogicException(err.str());
                }
                continue;
            }
        }
        {
            RadioButtonsFilter::SCPtr radiobtn = std::dynamic_pointer_cast<RadioButtonsFilter const>(f);
            if (radiobtn)
            {
                if (radiobtn->options().size() == 0)
                {
                    std::stringstream err;
                    err << "FilterBaseImpl::validate_filters(): invalid empty RadioButtonsFilter '" << f->id() << "'";
                    throw unity::LogicException(err.str());
                }
                continue;
            }
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
