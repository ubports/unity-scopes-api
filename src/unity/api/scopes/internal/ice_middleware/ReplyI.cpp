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

#include <unity/api/scopes/internal/ice_middleware/ReplyI.h>

#include <unity/api/scopes/ScopeBase.h>

#include <Ice/ObjectAdapter.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

ReplyI::ReplyI(ReplyObject::SPtr const& ro) :
    ro_(ro),
    finished_(false)
{
    assert(ro);
}

// Methods below are marked with a
//    // noexcept
// comment. That's because they really are noexcept, but we can't say this in the signature
// because we are deriving from the Slice-generated base.

void ReplyI::send(string const& result, Ice::Current const& c)  // noexcept
{
    if (finished_.load())
    {
        return; // Ignore calls to send() after finished() completes.
    }

    try
    {
        ro_->send(result);
    }
    catch (...)
    {
        // TODO: log error
        finished(c);
    }
}

void ReplyI::finished(Ice::Current const& c)  // noexcept
{
    if (finished_.exchange(true))
    {
        return;
    }

    try
    {
        try
        {
            ro_->finished();
        }
        catch (...)
        {
            // Any errors are logged already.
        }

        // Remove self from object adapter, which will end up destroying this Reply servant.
        try
        {
            c.adapter->remove(c.id);
        }
        catch (...)
        {
            // TODO: log error
        }
    }
    catch (...)
    {
        // TODO: log error
    }
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
