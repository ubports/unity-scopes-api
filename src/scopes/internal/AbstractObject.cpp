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

#include <unity/scopes/internal/AbstractObject.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

AbstractObject::AbstractObject()
{
}

AbstractObject::~AbstractObject()
{
}

void AbstractObject::set_disconnect_function(function<void()> func) noexcept
{
    assert(func);
    lock_guard<mutex> lock(mutex_);
    disconnect_func_ = func;
}

void AbstractObject::disconnect() noexcept
{
    try
    {
        // Lock needed because disconnect() may be called
        // from another thread, such as the reaper.
        lock_guard<mutex> lock(mutex_);
        assert(disconnect_func_);
        disconnect_func_();
    }
    catch (...)
    {
        // Only happens if no servant with the corresponding identity is registered.
        // If we have concurrent calls into a servant, each of which tries to disconnect
        // the servant, only the first one succeeds; second and subsequent calls wil be ignored.
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
