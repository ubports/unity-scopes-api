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

#ifndef UNITY_API_REPLYS_INTERNAL_MWOBJECT_H
#define UNITY_API_REPLYS_INTERNAL_MWOBJECT_H

#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class MiddlewareBase;

// Base class for the MW<something>Object hierarchy. We store the pointer to the middleware here,
// which is needed to, for example, registry callback objects (such as a Reply object) with the middleware.

class MWObject
{
public:
    UNITY_DEFINES_PTRS(MWObject);

    virtual ~MWObject() noexcept;

    MiddlewareBase* mw_base() const noexcept;

protected:
    MWObject(MiddlewareBase* mw_base);

private:
    MiddlewareBase* mw_base_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
