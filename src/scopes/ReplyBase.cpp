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

#include <unity/scopes/ReplyBase.h>
#include <unity/scopes/internal/ReplyImpl.h>

#include <cassert>

namespace unity
{

namespace scopes
{

//! @cond

ReplyBase::ReplyBase(internal::ReplyImpl* impl) :
    ObjectProxy(impl)
{
    assert(impl);
}

ReplyBase::~ReplyBase()
{
}

void ReplyBase::finished() const
{
    return fwd()->finished();
}

void ReplyBase::error(std::exception_ptr ex) const
{
    return fwd()->error(ex);
}

internal::ReplyImpl* ReplyBase::fwd() const
{
    return dynamic_cast<internal::ReplyImpl*>(pimpl());
}

//! @endcond

} // namespace scopes

} // namespace unity
