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

interface Reply;

// The separate Query object solely exists so we can control a query
// without the risk of blocking. The Scope::createQuery() method is on the -ctrl
// adapter, whereas the Query object is on the normal adapter. This allows
// createQuery() to start the query running by calling run(), which causes run()
// to be dispatched in a thread from the normal adapter, and createQuery() is guaranteed
// to not block.

interface Query
{
    void run(Reply* r); // Called as oneway
};

}; // ice_middleware

}; // internal

}; // scopes

}; // api

}; // unity
