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

#include <scopes/internal/AbstractObject.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

AbstractObject::AbstractObject()
{
}

AbstractObject::~AbstractObject() noexcept
{
}

void AbstractObject::set_disconnect_function(function<void()> func) noexcept
{
    assert(func);
    disconnect_func_ = func;
}

void AbstractObject::disconnect() noexcept
{
    try
    {
        assert(disconnect_func_);
        disconnect_func_();
    }
    catch (...)
    {
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
