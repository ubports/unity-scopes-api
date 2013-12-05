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

#ifndef UNITY_INTERNAL_HYPERLINKIMPL_H
#define UNITY_INTERNAL_HYPERLINKIMPL_H

#include <unity/SymbolExport.h>
#include <string>
#include <memory>
#include <scopes/Query.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class UNITY_API HyperlinkImpl
{
public:
    HyperlinkImpl(std::string const& label, Query const& query);
    std::string label() const;
    Query query() const;
    VariantMap serialize() const;

private:
    std::string label_;
    Query::SCPtr query_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
