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

#ifndef UNITY_SCOPES_FILTERSTATE_H
#define UNITY_SCOPES_FILTERSTATE_H

#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>
#include <memory>

namespace unity
{

namespace scopes
{

class FilterBase;

namespace internal
{
class FilterStateImpl;
class FilterBaseImpl;
}

/**
\brief Captures state of multiple filters.

State can be examined by passing an instance of FilterState to
appropriate methods of filters (FilterBase implementations).
*/

class FilterState final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(FilterState);

    ~FilterState();
    VariantMap serialize() const;
    /// @endcond

    /**
    \brief Constructs a an empty filter state instance.
    */
    FilterState();

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    FilterState(FilterState const& other);
    FilterState(FilterState &&);
    FilterState& operator=(FilterState const& other);
    FilterState& operator=(FilterState&& other);
    //@}

    /**
    \brief Check if state for the given filter has been stored.
    \return True if state for a filter with the given ID is present; false otherwise.
    */
    bool has_filter(std::string const& id) const;

    /**
    \brief Removes the state for a specific filter.
    \param id The identity of the filter to remove.
    */
    void remove(std::string const& id);

private:
    FilterState(internal::FilterStateImpl *pimpl);
    std::unique_ptr<internal::FilterStateImpl> p;
    friend class internal::FilterBaseImpl;
    friend class internal::FilterStateImpl;
};

} // namespace scopes

} // namespace unity

#endif
