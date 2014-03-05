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

#include <unity/scopes/internal/Reply.h>

#include <unity/scopes/internal/ReplyImpl.h>

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

//! @cond

Reply::Reply(internal::ReplyImpl* impl) : ObjectProxy(impl)
{
    assert(impl);
}

Reply::~Reply() = default;

void Reply::finished() const
{
    return fwd()->finished();
}

void Reply::error(std::exception_ptr ex) const
{
    return fwd()->error(ex);
}

internal::ReplyImpl* Reply::fwd() const
{
    return dynamic_cast<internal::ReplyImpl*>(pimpl());
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace unity
