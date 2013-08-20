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

#include <unity/api/scopes/internal/ReplyObject.h>

#include <unity/api/scopes/ReplyBase.h>
#include <unity/Exception.h>

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

ReplyObject::ReplyObject(ReplyBase::SPtr const& reply_base) :
    reply_base_(reply_base),
    finished_(false)
{
    assert(reply_base);
}

ReplyObject::~ReplyObject() noexcept
{
    try
    {
        finished();
    }
    catch (...)
    {
    }
}

void ReplyObject::push(std::string const& result)
{
    // We catch all exeptions so, if the application's push() method throws,
    // we can call finished(). Finished will be called exactly once, whether
    // push() or finished() throw or not.
    //
    // Assuming that the pushing side doesn't have bugs, we should never
    // receive an over-the-wire call to push() after finished() was called,
    // or more than one call to finished(). But, to be on the safe side, we
    // enforce here again that finish() will be called exactly once, and that
    // push() will not be called once finished() was called. This means that,
    // even if the client side is broken, the server side here maintains the
    // correct guarantees.
    //
    // We rethrow any exceptiosn to give the skeleton class that calls us
    // a chance to any clean-up it needs to do, such as unregistering itself
    // from the middleware.

    // TODO: the endless exception catch/cleanup/rethrow gets tedious quickly.
    //       Think about making a template that takes a functor to wrap this up.
    if (finished_.load())
    {
        return;
    }

    try
    {
        reply_base_->push(result);      // Forward the result to the application code.
    }
    catch (unity::Exception const& e)
    {
        // TODO: log error
        try
        {
            finished();
        }
        catch (...)
        {
        }
        throw;
    }
    catch (...)
    {
        // TODO: log error
        try
        {
            finished();
        }
        catch (...)
        {
        }
        throw;
    }
}

void ReplyObject::finished()
{
    // We permit exactly one finished() call for a query. This avoids
    // a race condition where the executing down-stream query invokes
    // finished() concurrently with the QueryCtrl forwarding a cancel()
    // call to this reply's finished() method.
    if (!finished_.exchange(true))
    {
        return;
    }

    try
    {
        reply_base_->finished();    // Inform the application code that the query is complete.
    }
    catch (unity::Exception const& e)
    {
        // TODO: log error
        try
        {
            finished();
        }
        catch (...)
        {
        }
        throw;
    }
    catch (...)
    {
        // TODO: log error
        try
        {
            finished();
        }
        catch (...)
        {
        }
        throw;
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
