/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/MWObject.h>

#include <cassert>

using namespace std;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

MWObject::MWObject(MiddlewareBase* mw_base) :
    mw_base_(mw_base)
{
    assert(mw_base);
}

MWObject::~MWObject() noexcept
{
}

MiddlewareBase* MWObject::mw_base() const noexcept
{
    return mw_base_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
