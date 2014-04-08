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

#include <unity/scopes/Runtime.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <signal.h>
#include <unistd.h>

using namespace unity::scopes;

class Receiver : public SearchListenerBase
{
public:
    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(ListenerBase::Reason /* reason */, std::string const& /* error_message */) override
    {
    }
};

TEST(Registry, metadata)
{
    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    auto meta = r->get_metadata("testscopeA");
    EXPECT_EQ("testscopeA", meta.scope_id());
    EXPECT_EQ("Canonical Ltd.", meta.author());
    EXPECT_EQ("scope-A.DisplayName", meta.display_name());
    EXPECT_EQ("scope-A.Description", meta.description());
    EXPECT_EQ("/foo/scope-A.Art", meta.art());
    EXPECT_EQ("/foo/scope-A.Icon", meta.icon());
    EXPECT_EQ("scope-A.HotKey", meta.hot_key());
    EXPECT_EQ("scope-A.SearchHint", meta.search_hint());
    EXPECT_EQ(TEST_RUNTIME_PATH "/scopes/testscopeA", meta.scope_directory());

    const char *bart = TEST_RUNTIME_PATH "/scopes/testscopeB/data/scope-B.Art";
    const char *bicon = TEST_RUNTIME_PATH "/scopes/testscopeB/data/scope-B.Icon";

    meta = r->get_metadata("testscopeB");
    EXPECT_EQ("testscopeB", meta.scope_id());
    EXPECT_EQ("Canonical Ltd.", meta.author());
    EXPECT_EQ("scope-B.DisplayName", meta.display_name());
    EXPECT_EQ("scope-B.Description", meta.description());
    EXPECT_EQ(bart, meta.art());
    EXPECT_EQ(bicon, meta.icon());
    EXPECT_EQ("scope-B.HotKey", meta.hot_key());
    EXPECT_EQ("scope-B.SearchHint", meta.search_hint());
    EXPECT_EQ(TEST_RUNTIME_PATH "/scopes/testscopeB", meta.scope_directory());

    auto sp = meta.proxy();

    SearchListenerBase::SPtr reply(new Receiver);
    SearchMetadata metadata("C", "desktop");

    // search would fail if testscopeB can't be executed
    try
    {
        auto ctrl = sp->search("foo", metadata, reply);
    }
    catch (...)
    {
        FAIL();
    }
}

bool wait_for_registry()
{
    const int num_retries = 10;
    const boost::filesystem::path path("/tmp/RegistryTest");
    for (int i = 0; i<num_retries; i++)
    {
        if (boost::filesystem::exists(path))
        {
            boost::system::error_code error;
            auto st = boost::filesystem::status(path, error).type();
            return st == boost::filesystem::file_type::socket_file;
        }
        sleep(1);
    }
    return false;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [Registry test]", TEST_RUNTIME_FILE, nullptr};
        if (execv(TEST_REGISTRY_PATH "/scoperegistry", const_cast<char* const*>(args)) < 0)
        {
            perror("Error starting scoperegistry:");
        }
        return 1;
    }
    else if (rpid > 0)
    {
        // FIXME: remove this once we have async queries and can set arbitrary timeout when calling registry
        EXPECT_TRUE(wait_for_registry());
        auto rc = RUN_ALL_TESTS();
        kill(rpid, SIGTERM);
        return rc;
    }
    else
    {
        perror("Failed to fork:");
    }
    return 1;
}
