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

interface QueryCtrl;
interface Reply;

// Factory for queries. The Scope object runs on the ctrl-adapter.
// createQuery() creates a Query object on the normal adapter, and a QueryCtrl object
// on the ctrl-adapter. It then calls run() on the Query object to set it executing in its own thread, before
// returning the QueryCtrl proxy that permits cancellation. This guarantees that createQuery() will not block.

interface Scope
{
    QueryCtrl* createQuery(string q, Reply* r);
};

}; // ice_middleware

}; // internal

}; // scopes

}; // api

}; // unity
