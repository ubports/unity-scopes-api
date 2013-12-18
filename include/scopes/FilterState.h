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
class ReplyImpl;
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
    void remove(std::string const& id);

private:
    FilterState(internal::FilterStateImpl *pimpl);
    std::shared_ptr<internal::FilterStateImpl> p;
    friend class internal::FilterBaseImpl;
    friend class internal::FilterStateImpl;
    friend class internal::ReplyImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
