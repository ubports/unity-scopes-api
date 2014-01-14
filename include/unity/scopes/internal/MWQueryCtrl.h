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

#ifndef UNITY_SCOPES_INTERNAL_MWQUERYCTRL_H
#define UNITY_SCOPES_INTERNAL_MWQUERYCTRL_H

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWQueryCtrl : public virtual MWObjectProxy
{
public:
    virtual ~MWQueryCtrl() noexcept;

    virtual void cancel() = 0;
    virtual void destroy() = 0;

protected:
    MWQueryCtrl(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
