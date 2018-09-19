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

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <functional>
#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

class AbstractObject
{
public:
    NONCOPYABLE(AbstractObject);

    UNITY_DEFINES_PTRS(AbstractObject);

    virtual ~AbstractObject();

    // Sets callback to allow this facade to disconnect itself from the middleware.
    // Called by disconnect().
    void set_disconnect_function(std::function<void()> func) noexcept;

protected:
    AbstractObject();

    void disconnect() noexcept; // Disconnect self from middleware

private:
    std::function<void()> disconnect_func_;
    std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
