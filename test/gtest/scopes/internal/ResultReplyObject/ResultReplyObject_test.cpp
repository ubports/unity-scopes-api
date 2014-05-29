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
    DummyReceiver(std::function<void(Department::SCPtr const&, Department::SCPtr const&)> departments_push_func)
    {
        departments_push_func_ = departments_push_func;
    };

    void push(CategorisedResult) override {}

    void push(Department::SCPtr const& parent, Department::SCPtr const& current_department)
    {
        departments_push_func_(parent, current_department);
    }

    void finished(Reason /* r */, std::string const& /* error_message */) override {}

    std::function<void(Department::SCPtr const&, Department::SCPtr const&)> departments_push_func_;
};

TEST(ResultReplyObject, departments_push)
{
    // valid department data
    {
        Department::SCPtr received_parent;
        Department::SCPtr received_current;

        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_parent, &received_current](Department::SCPtr const& parent, Department::SCPtr const& current_dep)
                {
                    received_parent = parent;
                    received_current = current_dep;
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
        reply.process_data(internal::DepartmentImpl::serialize_departments(parent, dep2));

        EXPECT_EQ("dep1", received_parent->id());
        EXPECT_EQ("dep2", received_current->id());
    }

    // invalid department data
    {
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([](Department::SCPtr const&, Department::SCPtr const&) {});
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        CannedQuery const query1("scope-foo", "", "dep1");

        Department::SPtr parent = Department::create("dep1", query1, "Dep1");
        Department::SPtr dep2 = Department::create("dep2", query1, "Dep2");
        Department::SPtr dep3 = Department::create("dep3", query1, "Dep3");
        dep2->set_subdepartments({dep3});
        parent->set_subdepartments({dep2});

        Department::SPtr current = Department::create("dep9", query1, "Dep9");

        // current department not in the parent's tree
        {
            auto var = internal::DepartmentImpl::serialize_departments(parent, current);
            var.erase("current_department");
            EXPECT_THROW(reply.process_data(var), unity::InvalidArgumentException);
        }
    }
}
