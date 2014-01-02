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

#ifndef UNITY_SCOPES_FILTEROPTION_H
#define UNITY_SCOPES_FILTEROPTION_H

#include <unity/SymbolExport.h>
#include <unity/util/DefinesPtrs.h>
#include <string>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class FilterOptionImpl;
class OptionSelectorFilterImpl;
}

/**
\brief Holds definition of filter option for OptionSelectorFilter.
*/
class UNITY_API FilterOption final
{
public:
/// @cond
    UNITY_DEFINES_PTRS(FilterOption);
/// @endcond

    std::string id() const;
    std::string label() const;
    ~FilterOption();

private:
    FilterOption(std::string const& id, std::string const& label);
    std::shared_ptr<internal::FilterOptionImpl> p;

    friend class internal::OptionSelectorFilterImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
