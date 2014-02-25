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

#ifndef UNITY_SCOPES_FILTERBASE_H
#define UNITY_SCOPES_FILTERBASE_H

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
All implementations of FilterBase define the "look" of
filter in the UI and do not hold any state information.
Actual state of a filters is kept by a FilterState object.
*/
class FilterBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(FilterBase);
    NONCOPYABLE(FilterBase);
    /// @endcond

    /**
     \brief Get an identifier of this filter.
     \return filter id
    */
    std::string id() const;

    /**
     \brief Get type name of this filter.
     \return filer type string
     */
    std::string filter_type() const;

    /// @cond
    VariantMap serialize() const;
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

#endif
