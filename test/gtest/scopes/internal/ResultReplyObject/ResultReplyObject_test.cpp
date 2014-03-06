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

#include <unity/scopes/internal/ResultReplyObject.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/CategorisedResult.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

class DummyReceiver : public SearchListenerBase
{
public:
    DummyReceiver(std::function<void(DepartmentList const&, std::string const&)> departments_push_func)
    {
        departments_push_func_ = departments_push_func;
    };

    void push(CategorisedResult) override {}

    void push(DepartmentList const& departments, std::string const& current_department_id)
    {
        departments_push_func_(departments, current_department_id);
    }

    void finished(Reason /* r */, std::string const& /* error_message */) override {}

    std::function<void(DepartmentList const&, std::string const&)> departments_push_func_;
};

TEST(ResultReplyObject, departments_push)
{
    // valid department data
    {
        DepartmentList received_deps;
        std::string received_current_dep_id;

        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_deps, &received_current_dep_id](DepartmentList const& departments, std::string const& current_dep)
                {
                    received_deps = departments;
                    received_current_dep_id = current_dep;
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        CannedQuery const query1("scope-foo", "", "dep1");
        CannedQuery const query2("scope-foo", "", "dep2");

        DepartmentList input_deps { Department(query1, "Dep1"), Department(query2, "Dep2") };
        std::string const input_current_dep("dep1");
        reply.process_data(internal::DepartmentImpl::serialize_departments(input_deps, input_current_dep));

        EXPECT_EQ(2, received_deps.size());
        EXPECT_EQ("dep1", received_deps.front().id());
        EXPECT_EQ("dep2", received_deps.back().id());
        EXPECT_EQ("dep1", received_current_dep_id);
    }

    // invalid department data
    {
        DepartmentList received_deps;
        std::string received_current_dep_id;

        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_deps, &received_current_dep_id](DepartmentList const& departments, std::string const& current_dep)
                {
                    received_deps = departments;
                    received_current_dep_id = current_dep;
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        CannedQuery const query1("scope-foo", "", "dep1");

        DepartmentList input_deps { Department(query1, "Dep1") };
        std::string const input_current_dep("dep1");

        // missing 'current_dep'
        {
            auto var = internal::DepartmentImpl::serialize_departments(input_deps, input_current_dep);
            var.erase("current_department");
            EXPECT_THROW(reply.process_data(var), unity::InvalidArgumentException);
        }
    }
}
