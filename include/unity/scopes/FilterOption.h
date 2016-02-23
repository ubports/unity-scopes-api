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

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>
#include <string>
#include <memory>

namespace unity
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
class FilterOption final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(FilterOption);
    NONCOPYABLE(FilterOption);
    /// @endcond

    /**
     \brief Get the identifier of this filter option.
     \return The option identifier.
     */
    std::string id() const;

    /**
     \brief Get the label of this filter option.
     \return The option label.
    */
    std::string label() const;

    /// @cond
    ~FilterOption();
    /// @endcond

    /**
     \brief Return the default value of this filter option.
     \return default value
    */
    bool default_value() const;

private:
    FilterOption(std::string const& id, std::string const& label);
    std::unique_ptr<internal::FilterOptionImpl> p;

    friend class internal::OptionSelectorFilterImpl;
};

} // namespace scopes

} // namespace unity
