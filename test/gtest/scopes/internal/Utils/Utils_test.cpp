/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/internal/Utils.h>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace std;

TEST(Utils, uncamelcase)
{
    EXPECT_EQ("", uncamelcase(""));
    EXPECT_EQ("foo-bar", uncamelcase("FooBar"));
    EXPECT_EQ("foo-bar", uncamelcase("foo-bar"));
    EXPECT_EQ("foo_bar", uncamelcase("foo_bar"));
    EXPECT_EQ("foo-bar", uncamelcase("fooBAR"));
    EXPECT_EQ("foo-bar", uncamelcase("fooBAr"));
    EXPECT_EQ("foo-bar", uncamelcase("foo-Bar"));
}

TEST(Utils, convert_to)
{
    {
        Variant out;
        EXPECT_FALSE(convert_to<int>("foo", out));
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<int>("10", out));
        EXPECT_EQ(10, out.get_int());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<string>("foo", out));
        EXPECT_EQ("foo", out.get_string());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<double>("10.0", out));
        EXPECT_FLOAT_EQ(10.0f, out.get_double());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<bool>("true", out));
        EXPECT_TRUE(out.get_bool());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<bool>("false", out));
        EXPECT_FALSE(out.get_bool());
    }
}

void empty_dir(string const& dir)
{
    namespace fs = boost::filesystem;
    try
    {
        for (fs::directory_iterator end, it(dir); it != end; ++it)
        {
            remove_all(it->path());
        }
    }
    catch (...)
    {
    }
}

int get_perms(string const& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        string msg = "get_perms(): cannot stat " + path + ": errno = " + to_string(errno);
        throw runtime_error(msg);
    }
    return st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
}

TEST(Utils, make_directories)
{
    using namespace std;

    string test_root = TEST_DIR "/dir";

    empty_dir(test_root);
    rmdir(test_root.c_str());  // First time around, make sure that test_root doesn't exist.

    make_directories(test_root, 0700);
    EXPECT_EQ(0700, get_perms(test_root));

    // Do it a second time, to test that it works whether test_root exists or not.
    make_directories(test_root, 0700);
    EXPECT_EQ(0700, get_perms(test_root));

    // Change permissions on test_root and make a dir underneath it.
    chmod(test_root.c_str(), 0777);
    string child = test_root + "/child";
    int old_umask = umask(0000);
    make_directories(child, 0555);
    umask(old_umask);

    // test_root permissions must still be the same, child must have perm 0555.
    EXPECT_EQ(0777, get_perms(test_root));
    EXPECT_EQ(0555, get_perms(child));

    // Remove permission from child dir.
    chmod(child.c_str(), 0000);

    // Check that we fail silently trying to create the grandchild.
    string grandchild = child + "/grandchild";
    make_directories(grandchild, 0555);

    chmod(child.c_str(), 0777);  // Don't leave the dir behind without permissions.
}
