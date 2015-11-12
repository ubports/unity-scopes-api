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

#pragma once

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/FilterOption.h>
#include <string>
#include <list>
#include <set>

namespace unity
{

namespace scopes
{
class FilterState;

namespace internal
{

class OptionSelectorFilterImpl : public FilterBaseImpl
{
public:
    OptionSelectorFilterImpl(std::string const& id, std::string const& label, bool multi_select);
    OptionSelectorFilterImpl(VariantMap const& var);
    std::string label() const;
    bool multi_select() const;
    virtual FilterOption::SCPtr add_option(std::string const& id, std::string const& label);
    std::list<FilterOption::SCPtr> options() const;
    int num_of_options() const;
    std::set<FilterOption::SCPtr> active_options(FilterState const& filter_state) const;
    bool has_active_option(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);
    static OptionSelectorFilter::SPtr create(VariantMap const& var);
    void validate_display_hints() const override;

protected:
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);
    std::string filter_type() const override;

private:
    void throw_on_missing(VariantMap::const_iterator const& it, VariantMap::const_iterator const& endit, std::string const& name);
    std::string label_;
    bool multi_select_;
    std::list<FilterOption::SCPtr> options_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
