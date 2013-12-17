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

#ifndef UNITY_INTERNAL_FILTERBASEIMPL_H
#define UNITY_INTERNAL_FILTERBASEIMPL_H

#include <unity/SymbolExport.h>
#include <scopes/Variant.h>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{
class FilterState;

namespace internal
{

class UNITY_API FilterBaseImpl
{
public:
    FilterBaseImpl(std::string const& id);
    virtual ~FilterBaseImpl();
    std::string id() const;
    VariantMap serialize() const;

protected:
    static Variant get(FilterState const& filter_state, std::string const& filter_id);
    static VariantMap& get(FilterState const& filter_state);

private:
    std::string id_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
