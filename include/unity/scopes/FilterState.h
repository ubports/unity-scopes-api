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

namespace unity
{

namespace scopes
{

class FilterBase;

class UNITY_API FilterState final
{
public:
    void store(FilterBase const& filter, Variant const& value);
    Variant get(FilterBase const& filter) const;
};

} // namespace scopes

} // namespace unity

#endif