/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_MWQUERYPROXYFWD_H
#define UNITY_API_SCOPES_INTERNAL_MWQUERYPROXYFWD_H

#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class MWQuery;
typedef std::shared_ptr<MWQuery> MWQueryProxy;

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
