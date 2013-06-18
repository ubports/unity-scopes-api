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

#include <unity/api/scopes/internal/ScopeLoader.h>
#include <unity/api/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include "MyScope.h"
#include "Counters.h"
#include "PerScopeVariables.h"

#include <gtest/gtest.h>

#define EXPORT __attribute__ ((visibility("default")))

using namespace std;

class ScopeA : public MyScope
{
public:
    virtual void start() { inc_start(); }
    virtual void stop() { inc_stop(); }
};

extern "C"
{

EXPORT
unity::api::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_API_SCOPE_CREATE_FUNCTION()
{
    inc_create();
    return new ScopeA;
}

EXPORT
void
// cppcheck-suppress unusedFunction
UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope_base)
{
    inc_destroy();
    delete scope_base;
    set_scopeB_var(2);
}

}
