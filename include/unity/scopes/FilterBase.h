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
#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>
#include <memory>
#include <list>

namespace unity
{

namespace scopes
{

namespace internal
{
class FilterBaseImpl;
}

/**
\brief Base class for all implementations of filters.

All implementations of FilterBase define the "look" of a
filter in the UI and do not hold any state information.
The actual state of a filters is kept by a FilterState object.
*/

class FilterBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(FilterBase);
    NONCOPYABLE(FilterBase);
    /// @endcond

    /**
     \brief Display hints for the Shell UI.
     */
    enum DisplayHints
    {
        Default = 0,    /**< Default value (no hint) */
        Primary = 1     /**< Display this filter at the top of the screen, if possible */
    };

    /**
     \brief Sets display hints for the Shell UI.

     \param hints A combination of DisplayHints for this filter.
    */
    void set_display_hints(int hints);

    /**
     \brief Get display hints of this filter.

     \return Display hints flags.
    */
    int display_hints() const;

    /**
     \brief Get the identifier of this filter.
     \return The filter id.
    */
    std::string id() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

    /**
     \brief Get the type name of this filter.
     \return The filter type string.
     */
    std::string filter_type() const;

    /// @cond
    virtual ~FilterBase();

protected:
    FilterBase(internal::FilterBaseImpl *pimpl);
    std::unique_ptr<internal::FilterBaseImpl> p;
    /// @endcond
};

/**
 \brief List of filters
*/
typedef std::list<FilterBase::SCPtr> Filters;

} // namespace scopes

} // namespace unity
