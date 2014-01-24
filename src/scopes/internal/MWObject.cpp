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

#include <unity/scopes/internal/MWObjectProxy.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

MWObjectProxy::MWObjectProxy(MiddlewareBase* mw_base) :
    mw_base_(mw_base)
{
    assert(mw_base);
}

MWObjectProxy::~MWObjectProxy()
{
}

MiddlewareBase* MWObjectProxy::mw_base() const noexcept
{
    return mw_base_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
