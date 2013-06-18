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

#ifndef UNITY_API_SCOPES_INTERNAL_MIDDLEWAREBASE_H
#define UNITY_API_SCOPES_INTERNAL_MIDDLEWAREBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Abstract base class for our middleware transport. Any control functions we need can be added here
// as pure virtual functions. This is used by the remainder of the code to do things to the middleware
// without knowing which middleware it actually is (strategy pattern).

class MiddlewareBase : public util::DefinesPtrs<MiddlewareBase>, private util::NonCopyable
{
public:
    MiddlewareBase();
    virtual ~MiddlewareBase() noexcept;

    virtual void start(std::string const& scope_name) = 0;
    virtual void stop() = 0;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
