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

#include "PerScopeVariables.h"
#include <mutex>

using namespace std;

namespace
{

int scopeA_var;
int scopeB_var;
std::mutex count_mutex;

}

// Helper functions to get and set scopeA_var and scopeB_var so we can test that multiple scopes in the
// same address space work correctly.

void set_scopeA_var(int val)
{
    lock_guard<mutex> lock(count_mutex);
    scopeA_var = val;
}

int get_scopeA_var()
{
    lock_guard<mutex> lock(count_mutex);
    return scopeA_var;
}

void set_scopeB_var(int val)
{
    lock_guard<mutex> lock(count_mutex);
    scopeB_var = val;
}

int get_scopeB_var()
{
    lock_guard<mutex> lock(count_mutex);
    return scopeB_var;
}

void clear_vars()
{
    lock_guard<mutex> lock(count_mutex);
    scopeA_var = 0;
    scopeB_var = 0;
}
