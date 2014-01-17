/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace unity::scopes::internal::smartscopes;

TEST(SSRegistryObject, basic)
{
    ::putenv("SMART_SCOPES_SERVER=http://127.0.0.1:8000");
    SSRegistryObject reg(SS_RUNTIME_PATH);
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
}
