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

#include <unity/scopes/CategoryRenderer.h>
#include <unity/UnityExceptions.h>
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(CategoryRenderer, basic)
{
    CategoryRenderer rdr;
    EXPECT_FALSE(rdr.data().empty());
    EXPECT_EQ(DEFAULT_RENDERER, rdr.data());
}

TEST(CategoryRenderer, from_file)
{
    const std::string input_file(TEST_DIR "/renderer.json");

    EXPECT_THROW(CategoryRenderer::from_file("/non-existing-file"), unity::ResourceException);
    EXPECT_NO_THROW(CategoryRenderer::from_file(input_file));

    auto rdr = CategoryRenderer::from_file(input_file);
    EXPECT_FALSE(rdr.data().empty());
}

TEST(CategoryRenderer, exceptions)
{
    try
    {
        // only dict is allowed
        CategoryRenderer rdr("[1,2]");
        FAIL();
    }
    catch (unity::InvalidArgumentException const&)
    {
    }

    try
    {
        // invalid JSON
        CategoryRenderer rdr("{");
        FAIL();
    }
    catch (unity::InvalidArgumentException const&)
    {
    }

    try
    {
        CategoryRenderer rdr("{}");
    }
    catch (...)
    {
        FAIL();
    }
}
