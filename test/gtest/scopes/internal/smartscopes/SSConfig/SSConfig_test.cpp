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

#include <unity/scopes/internal/smartscopes/SSConfig.h>
#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

TEST(SSConfig, basic)
{
    {
        // No config, defaults apply.
        SSConfig c("");
        EXPECT_EQ(DFLT_SS_HTTP_TIMEOUT, c.http_reply_timeout());
        EXPECT_EQ(DFLT_SS_REG_REFRESH_RATE, c.reg_refresh_rate());
        EXPECT_EQ(DFLT_SS_REG_REFRESH_FAIL_TIMEOUT, c.reg_refresh_fail_timeout());
        EXPECT_EQ(DFLT_SS_SCOPE_IDENTITY, c.scope_identity());
    }

    {
        // Values in configfile apply.
        SSConfig c(TEST_SSREGISTRY_PATH);
        EXPECT_EQ(2, c.http_reply_timeout());
        EXPECT_EQ(77333, c.reg_refresh_rate());
        EXPECT_EQ(17, c.reg_refresh_fail_timeout());
        EXPECT_EQ("Fred", c.scope_identity());
    }
}
