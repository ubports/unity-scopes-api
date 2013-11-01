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

#include "MyScope.h"
#include "Counters.h"

#include <scopes/internal/ScopeLoader.h>

#include <cassert>

extern "C"
{

    EXPORT
    unity::api::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_CREATE_FUNCTION()
    {
        inc_create();
        return nullptr;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope_base)
    {
        inc_destroy();
        // We use an abort instead of EXPECT_EQ because we can't see the symbols
        // in gtest.a here.
        if (scope_base != nullptr)
        {
            abort();
        }
    }

}
