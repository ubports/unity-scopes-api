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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "TestScope.h"

using namespace std;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query, SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
        , query_(query)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        // pushing invalid departments (current departnent not present in the tree) should throw
        {
            Department::SPtr parent = Department::create("unknown1", query_, "All departments");
            Department::SPtr news_dep = Department::create("unknown2", query_, "News");
            news_dep->set_subdepartments({Department::create("unknown3", query_, "Europe")});
            parent->set_subdepartments({news_dep});
            EXPECT_THROW(reply->register_departments(parent), unity::LogicException);
        }

        // pushing invalid departments (just one level) should throw
        {
            Department::SPtr parent = Department::create("unknown1", query_, "All departments");
            EXPECT_THROW(reply->register_departments(parent), unity::LogicException);
        }

        // send test info for no internet connection (mid run)
        reply->info(OperationInfo{OperationInfo::NoInternet, "Partial results returned due to no internet connection."});

        Department::SPtr parent = Department::create("", query_, "All departments");
        Department::SPtr news_dep = Department::create("news", query_, "News");
        news_dep->set_subdepartments({Department::create("subdep1", query_, "Europe"), Department::create("subdep2", query_, "US")});
        parent->set_subdepartments({news_dep});
        reply->register_departments(parent);

        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);

        // send test info for no internet connection (mid run)
        reply->info(OperationInfo{OperationInfo::PoorInternet, "Partial results returned due to poor internet connection."});

        CannedQuery query("scope-A", "foo", "dep1");
        experimental::Annotation annotation(experimental::Annotation::Type::Link);
        annotation.add_link("Link1", query);
        reply->push(annotation);
    }

private:
    CannedQuery query_;
};

}  // namespace

class TestPreview : public PreviewQueryBase
{
public:
    TestPreview(Result const& result, ActionMetadata const& metadata)
        : PreviewQueryBase(result, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        // send test info for no location data (run start)
        reply->info(OperationInfo{OperationInfo::NoLocationData});

        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        widgets.emplace_back(PreviewWidget(R"({"id": "id", "type": "image", "art": "screenshot-url"})"));
        reply->push(widgets);
        reply->push("author", Variant("Foo"));
        reply->push("rating", Variant("Bar"));

        // send test info for no account data (run end)
        reply->info(OperationInfo{OperationInfo::InaccurateLocationData, "Partial results returned due to inaccurate location data."});
    }
};

SearchQueryBase::UPtr TestScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    return SearchQueryBase::UPtr(new TestQuery(query, metadata));
}

PreviewQueryBase::UPtr TestScope::preview(Result const& result, ActionMetadata const& metadata)
{
    return PreviewQueryBase::UPtr(new TestPreview(result, metadata));
}
