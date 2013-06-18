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

#ifndef SCOPELOADERTEST_MYSCOPE
#define SCOPELOADERTEST_MYSCOPE

#include <unity/SymbolExport.h>
#include "Counters.h"

// Test scope class that allows us to count the number of calls to start() and stop().

class UNITY_API MyScope : public unity::api::scopes::ScopeBase
{
public:
    MyScope() {}
    virtual ~MyScope() {}

    virtual void start() { inc_start(); }
    virtual void stop() { inc_stop(); }
    virtual void query(std::string const& /* q */) {}
};

#endif
