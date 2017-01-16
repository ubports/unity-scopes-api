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

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/internal/DynamicLoader.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace
{
char const* scopelib = TEST_DIR "/libscopelib.so";
}

bool create_called = false;

// This test loads the scope library via the DynamicLoader and calls the library's create entry point
// (unity_scope_create(), which returns a pointer to a ScopeBase). It then calls the derived instance's
// start(), and stop() member functions before calling the library's destroy entry point
// (unity_scope_destroy(), which deallocates the dervied instance again.
//
// If all this works without any exceptions or crashes, we know that the basic mechanism of loading and unloading
// scopes from shared libraries, and getting a thread of control into the scope works correctly.

TEST(ScopeBase, basic)
{
    DynamicLoader::UPtr dl = DynamicLoader::create(scopelib);

    char const* create_sym = UNITY_SCOPE_CREATE_SYMSTR;
    unity::scopes::CreateFunction create
        = reinterpret_cast<unity::scopes::CreateFunction>(dl->find_function(create_sym));

    char const* destroy_sym = UNITY_SCOPE_DESTROY_SYMSTR;
    unity::scopes::DestroyFunction destroy
        = reinterpret_cast<unity::scopes::DestroyFunction>(dl->find_function(destroy_sym));

    ScopeBase* b = create();
    EXPECT_NE(nullptr, b);

    // For coverage testing
    int vmajor;
    int vminor;
    int vmicro;

    b->runtime_version(vmajor, vminor, vmicro);
    EXPECT_EQ(UNITY_SCOPES_VERSION_MAJOR, vmajor);
    EXPECT_EQ(UNITY_SCOPES_VERSION_MINOR, vminor);
    EXPECT_EQ(UNITY_SCOPES_VERSION_MICRO, vmicro);

    b->start("some_scope");
    b->stop();
    destroy(b);
}
