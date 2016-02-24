/*
 * Copyright (C) 2016 Canonical Ltd
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
    class FilterGroupImpl;
}

/**
\brief Groups several filters into an expandable widget in the UI.
*/
class FilterGroup
{
    /// @cond
    NONCOPYABLE(FilterGroup);
    UNITY_DEFINES_PTRS(FilterGroup);
    /// @endcond

public:
    /**
     \brief Create a WidgetGroup.

     \return An instance of WidgetGroup.
     */
    static FilterGroup::SCPtr create(std::string const& label);

    /**
     \brief Get the label of the widget group.

     \return The label.
    */
    std::string label() const;

    /// @cond
    virtual ~FilterGroup();
    /// @endcond

private:
    /// @cond
    FilterGroup() = delete;
    FilterGroup(internal::FilterGroupImpl *pimpl);
    std::unique_ptr<internal::FilterGroupImpl> p;
    /// @endcond
};

} // namespace scopes

} // namespace unity
