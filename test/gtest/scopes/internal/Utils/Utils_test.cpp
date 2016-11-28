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

#include <unity/UnityExceptions.h>

#include <boost/filesystem.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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

TEST(Utils, uncamelcase_turkish)
{
    string old_locale;
    char const* p = getenv("LC_ALL");
    if (p)
    {
        old_locale = p;
    }
    setenv("LC_ALL", "tr_TR.UTF-8", 1);

    EXPECT_EQ("small-i", uncamelcase("smallI"));

    if (!old_locale.empty())
    {
        setenv("LC_ALL", old_locale.c_str(), 1);
    }
    else
    {
        unsetenv("LX_ALL");
    }
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

TEST(Utils, split_exec_args)
{
    // Test empty executable
    try
    {
        split_exec_args("test", "");
        FAIL();
    }
    catch (std::exception const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Invalid empty executable for scope: 'test'", e.what());
    }

    // Test invalid executable
    try
    {
        split_exec_args("test", "\"");
        FAIL();
    }
    catch (std::exception const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Invalid executable for scope: 'test'", e.what());
    }

    // Test argument splitting
    auto exec_args = split_exec_args("test", "/path\\ to/exec' 'file arg \"arg 2\" arg' '3 arg\\ 4");
    ASSERT_EQ(5u, exec_args.size());
    EXPECT_STREQ("\"/path to/exec file\"", exec_args[0].c_str());
    EXPECT_STREQ("arg", exec_args[1].c_str());
    EXPECT_STREQ("\"arg 2\"", exec_args[2].c_str());
    EXPECT_STREQ("\"arg 3\"", exec_args[3].c_str());
    EXPECT_STREQ("\"arg 4\"", exec_args[4].c_str());
}

TEST(Utils, convert_exec_rel_to_abs)
{
    // Test empty executable
    try
    {
        convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "");
        FAIL();
    }
    catch (std::exception const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Invalid empty executable for scope: 'test'", e.what());
    }

    // Test invalid executable
    try
    {
        convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "\"");
        FAIL();
    }
    catch (std::exception const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Invalid executable for scope: 'test'", e.what());
    }

    // Test nonexistent executable
    try
    {
        convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "noexec");
        FAIL();
    }
    catch (std::exception const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Nonexistent scope runner executable: 'noexec' for scope: 'test'", e.what());
    }

    // Test absolute executable
    auto abs_exec = convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "/bin/bash");
    EXPECT_STREQ("/bin/bash", abs_exec.c_str());

    // Test absolute executable w/ args
    abs_exec = convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "/bin/bash arg \"arg 2\" arg' '3 arg\\ 4");
    EXPECT_STREQ("/bin/bash arg \"arg 2\" \"arg 3\" \"arg 4\"", abs_exec.c_str());

    // Test relative executable (w/ ./)
    abs_exec = convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "./Utils_test");
    EXPECT_TRUE(boost::filesystem::exists(abs_exec));

    // Test relative executable (w/o ./)
    abs_exec = convert_exec_rel_to_abs("test", boost::filesystem::path(TEST_DIR), "Utils_test");
    EXPECT_TRUE(boost::filesystem::exists(abs_exec));
}
