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

#ifndef UNITY_SCOPES_OPTIONSELECTOR_H
#define UNITY_SCOPES_OPTIONSELECTOR_H

#include <scopes/FilterBase.h>
#include <scopes/FilterOption.h>
#include <scopes/Variant.h>
#include <string>
#include <list>
#include <set>

namespace unity
{

namespace api
{

namespace scopes
{
class FilterState;

namespace internal
{
class OptionSelectorFilterImpl;
class FilterBaseImpl;
}

/**
\brief
*/
class UNITY_API OptionSelectorFilter : public FilterBase
{
public:
    OptionSelectorFilter(std::string const& id, std::string const& label, bool multi_select = false);
    std::string label() const;
    bool multi_select() const;
    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);
    std::list<FilterOption::SCPtr> options() const;
    std::set<FilterOption::SCPtr> active_options(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;

    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);

private:
    OptionSelectorFilter(VariantMap const& var);
    internal::OptionSelectorFilterImpl* fwd() const;
    friend class internal::FilterBaseImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
