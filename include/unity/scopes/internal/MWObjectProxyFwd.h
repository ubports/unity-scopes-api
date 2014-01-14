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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_REPLYS_INTERNAL_MWOBJECTPROXYFWD_H
#define UNITY_REPLYS_INTERNAL_MWOBJECTPROXYFWD_H

#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWObjectProxy;
typedef std::shared_ptr<MWObjectProxy> MWProxy;

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
