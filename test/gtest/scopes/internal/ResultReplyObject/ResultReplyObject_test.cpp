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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

class DummyReceiver : public SearchListenerBase
{
public:
    DummyReceiver(std::function<void(Department::SCPtr const&)> departments_push_func)
    {
        departments_push_func_ = departments_push_func;
    }

    void push(CategorisedResult) override {}

    void push(Department::SCPtr const& parent) override
    {
        departments_push_func_(parent);
    }

    void finished(CompletionDetails const& /*details*/) override {}

    std::function<void(Department::SCPtr const&)> departments_push_func_;
};

TEST(ResultReplyObject, departments_push)
{
    // valid department data
    {
        Department::SCPtr received_parent;

        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_parent](Department::SCPtr const& parent)
                {
                    received_parent = parent;
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        CannedQuery const query1("scope-foo", "", "dep1");
        CannedQuery const query2("scope-foo", "", "dep2");

        Department::SPtr parent = Department::create("dep1", query1, "Dep1");
        Department::SPtr dep2 = Department::create("dep2", query2, "Dep2");
        Department::SPtr dep3 = Department::create("dep3", query2, "Dep3");
        dep2->set_subdepartments({dep3});
        parent->set_subdepartments({dep2});
        reply.process_data(internal::DepartmentImpl::serialize_departments(parent));

        EXPECT_EQ("dep1", received_parent->id());
    }

    // invalid department data
    {
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([](Department::SCPtr const&) {});
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        CannedQuery const query1("scope-foo", "", "dep1");

        Department::SPtr parent = Department::create("dep1", query1, "Dep1");

        // hierarchy too shallow
        {
            auto var = internal::DepartmentImpl::serialize_departments(parent);
            EXPECT_THROW(reply.process_data(var), unity::LogicException);
        }
    }
}
