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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scope-api-testconfig.h>
#include <scopes/CategoryRenderer.h>
#include <unity/UnityExceptions.h>
#include <fstream>
#include <gtest/gtest.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(CategoryRenderer, basic)
{
    CategoryRenderer rdr;
    EXPECT_TRUE(rdr.data().size() > 0);
    EXPECT_EQ(CategoryRenderer::DEFAULT, rdr.data());
}

TEST(CategoryRenderer, from_file)
{
    const std::string input_file(TEST_BUILD_ROOT "/gtest/unity/api/scopes/CategoryRenderer/renderer.json");

    EXPECT_THROW(CategoryRenderer::from_file("/non-existing-file"), std::ifstream::failure);
    EXPECT_NO_THROW(CategoryRenderer::from_file(input_file));

    auto rdr = CategoryRenderer::from_file(input_file);
    EXPECT_TRUE(rdr.data().size() > 0);
}
