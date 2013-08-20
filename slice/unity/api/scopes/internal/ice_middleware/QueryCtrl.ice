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

module unity
{

module api
{

module scopes
{

module internal
{

module middleware
{

// The separate QueryCtrl object solely exists so we can cancel a query
// without the risk of blocking. The QueryCtrl::cancel() method is on the -ctrl
// adapter, whereas the Query object is on the normal adapter. This means
// we alwasy have a thread available to respond to an incoming cancel request.

interface QueryCtrl
{
    void cancel();  // Called as oneway, tells the corresponding query it has been cancelled
    void destroy(); // Used by Query to destroy this QueryCtrl once query is done.
};

}; // ice_middleware

}; // internal

}; // scopes

}; // api

}; // unity
