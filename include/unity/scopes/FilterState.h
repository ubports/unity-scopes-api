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

#include <unity/SymbolExport.h>
#include <unity/scopes/Variant.h>
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
class UNITY_API FilterState final
{
public:
/// @cond
    FilterState();
    FilterState(FilterState const& other);
    FilterState(FilterState &&);
    FilterState& operator=(FilterState const& other);
    FilterState& operator=(FilterState&& other);
    ~FilterState();
    VariantMap serialize() const;
/// @endcond

    /**
     \brief Check if state for given filter has been stored.
     \return true if state for filter with id is present.
     */
    bool has_filter(std::string const& id) const;

    /**
      \brief Removes state for specific filter.
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
