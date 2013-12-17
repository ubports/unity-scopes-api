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

#ifndef UNITY_API_SCOPES_FILTERSTATE_H
#define UNITY_API_SCOPES_FILTERSTATE_H

#include <unity/SymbolExport.h>
#include <scopes/Variant.h>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

class FilterBase;

namespace internal
{
class FilterStateImpl;
class FilterBaseImpl;
}

class UNITY_API FilterState final
{
public:
    FilterState();
    FilterState(FilterState const& other);
    FilterState(FilterState &&);
    FilterState& operator=(FilterState const& other);
    FilterState& operator=(FilterState&& other);
    bool has_filter(std::string const& id) const;
    void reset(std::string const& id);
    void store(FilterBase const& filter, Variant const& value);
    Variant get(FilterBase const& filter) const;

    void set_option_selector_value(std::string const& filter_id, std::string const& option_id, bool value);

private:
    Variant get(std::string const& filter_id) const;
    std::shared_ptr<internal::FilterStateImpl> p;
    friend class internal::FilterBaseImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
