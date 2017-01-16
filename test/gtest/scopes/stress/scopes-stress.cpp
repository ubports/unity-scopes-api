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

// You may also include individual headers if you prefer.
#include <unity-scopes.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <boost/filesystem.hpp>
#include <condition_variable>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <cassert>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;

class Receiver : public SearchListenerBase
{
public:
    Receiver(const std::string& scope_id)
        : query_complete_(false),
          push_result_count_(0),
          scope_id_(scope_id)
    {
    }

    virtual void push(Department::SCPtr const& /* parent */) override
    {
    }

    virtual void push(Category::SCPtr const& /* category */) override
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
        push_result_count_++;
    }

    virtual void push(experimental::Annotation /* annotation */) override
    {
    }

    void push(Filters const& /* filters */, FilterState const& /* filter_state */) override
    {
    }

    virtual void finished(CompletionDetails const& details) override
    {
        if (details.status() == CompletionDetails::Error)
        {
            cerr << "query failed: " << details.message() << endl;
        }
        unique_lock<decltype(mutex_)> lock(mutex_);
        query_complete_ = true;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this] { return this->query_complete_; });
    }

    int result_count() const
    {
        return push_result_count_;
    }

    std::string scope_id() const
    {
        return scope_id_;
    }

private:
    bool query_complete_;
    int push_result_count_ = 0;
    mutex mutex_;
    condition_variable condvar_;
    std::string scope_id_;
};

TEST(stress, queryThreeScopesInParallel)
{
    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    auto scopes_meta = r->list();
    EXPECT_EQ(3u, scopes_meta.size());

    const std::list<std::string> scopes {"scope1", "scope2", "scope3"};

    for (int i = 0; i<40; i++)
    {
        cout << "RUN #" << i << endl;
        list<shared_ptr<Receiver>> replies;
        for (auto scope_id: scopes)
        {
            auto meta = r->get_metadata(scope_id);
            shared_ptr<Receiver> reply(new Receiver(scope_id));
            replies.push_back(reply);

            FilterState filter_state;
            SearchMetadata metadata("C", "phone");
            auto ctrl = meta.proxy()->search("", "", filter_state, metadata, reply);
        }

        cout << "waiting for replies to finish" << endl;
        for (auto reply: replies)
        {
            reply->wait_until_finished();
            cout << "finished query for scope " << reply->scope_id() << endl;
            EXPECT_EQ(20, reply->result_count());
        }
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;
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
        rc = RUN_ALL_TESTS();

        kill(rpid, SIGTERM);
        waitpid(rpid, nullptr, 0);
    }
    else
    {
        perror("Failed to fork:");
    }

    return rc;
}
