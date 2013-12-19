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

#ifndef UNITY_SCOPES_QUERYCTRL_H
#define UNITY_SCOPES_QUERYCTRL_H

#include <scopes/ObjectProxy.h>
#include <scopes/QueryCtrlProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class QueryCtrlImpl;
}

/**
\brief QueryCtrl allows a query to be cancelled.
*/

class UNITY_API QueryCtrl : public virtual ObjectProxy
{
public:
    /**
    \brief Cancels an executing query.
    Calling cancel() informs the scope to which a query was sent that the query should be cancelled.
    Calling cancel() on a query that is complete has no effect.
    cancel() is guaranteed to not block the caller.
    */
    void cancel() const;

    /**
    \brief Destroys a QueryCtrl.
    Letting a QueryCtrl go out of scope has no effect on the query, that is, the destructor does _not_
    implicitly call cancel().
    */
    virtual ~QueryCtrl() noexcept;

protected:
    QueryCtrl(internal::QueryCtrlImpl* impl);         // Instantiated only by QueryCtrlImpl
    friend class internal::QueryCtrlImpl;

private:
    internal::QueryCtrlImpl* fwd() const;
};

} // namespace scopes

} // namespace unity

#endif
