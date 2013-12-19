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

#include "MyScope.h"
#include "Counters.h"
#include "PerScopeVariables.h"

#include <scopes/internal/ScopeLoader.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

using namespace std;

class ScopeA : public MyScope
{
public:
    virtual int start(std::string const&, unity::api::scopes::RegistryProxy const&) override
    {
        inc_start();
        return VERSION;
    }

    virtual void stop() override
    {
        inc_stop();
    }
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
        set_scopeA_var(1);
    }

}