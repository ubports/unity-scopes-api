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

#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/Query.h>
#include <unity/scopes/Department.h>

#include "scope.h"

#include <thread>

namespace testing
{

class ActivationShowingDash : public unity::scopes::ActivationBase
{
public:
    ActivationShowingDash()
    {
    }

    unity::scopes::ActivationResponse activate() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        return unity::scopes::ActivationResponse{unity::scopes::ActivationResponse::ShowDash};
    }
};

class LongRunningActivation : public unity::scopes::ActivationBase
{
public:
    LongRunningActivation()
    {
    }

    unity::scopes::ActivationResponse activate() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        return unity::scopes::ActivationResponse{unity::scopes::ActivationResponse::ShowDash};
    }
};

class Query : public unity::scopes::SearchQuery
{
public:
    Query(unity::scopes::Query const& query)
        : query_(query)
    {
    }

    void cancelled() override
    {
    }

    void run(unity::scopes::SearchReplyProxy const& reply) override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        unity::scopes::Department dep("news", query_, "News");
        dep.set_subdepartments({{"subdep1", query_, "Europe"},{"subdep2", query_, "US"}});
        reply->register_departments({dep}, "news");

        auto cat = reply->register_category("cat1", "Category 1", "");
        unity::scopes::CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);

        unity::scopes::Query query("scope-A", "foo", "dep1");
        unity::scopes::Annotation annotation(unity::scopes::Annotation::Type::Link);
        annotation.add_link("Link1", query);
        reply->register_annotation(annotation);
    }

private:
    unity::scopes::Query query_;
};

class Preview : public unity::scopes::PreviewQuery
{
public:
    void cancelled() override
    {
    }

    void run(unity::scopes::PreviewReplyProxy const& reply) override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        unity::scopes::PreviewWidgetList widgets;
        widgets.emplace_back(unity::scopes::PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        widgets.emplace_back(unity::scopes::PreviewWidget(R"({"id": "id", "type": "image", "art": "screenshot-url"})"));
        reply->push(widgets);
        reply->push("author", unity::scopes::Variant("Foo"));
        reply->push("rating", unity::scopes::Variant("Bar"));
    }
};

} // namespace testing

int testing::Scope::start(std::string const&, unity::scopes::RegistryProxy const &)
{
    return VERSION;
}

void testing::Scope::stop()
{
}

void testing::Scope::run()
{
}

unity::scopes::SearchQuery::UPtr testing::Scope::create_query(
        unity::scopes::Query const& query,
        unity::scopes::SearchMetadata const &)
{
    return unity::scopes::SearchQuery::UPtr(new testing::Query(query));
}

unity::scopes::ActivationBase::UPtr testing::Scope::activate(
        unity::scopes::Result const&,
        unity::scopes::ActionMetadata const&)
{
    return unity::scopes::ActivationBase::UPtr{new testing::ActivationShowingDash()};
}

unity::scopes::ActivationBase::UPtr testing::Scope::perform_action(
        unity::scopes::Result const&,
        unity::scopes::ActionMetadata const&,
        std::string const&,
        std::string const&)
{
    return unity::scopes::ActivationBase::UPtr{new testing::LongRunningActivation()};
}

unity::scopes::PreviewQuery::UPtr testing::Scope::preview(
        unity::scopes::Result const&,
        unity::scopes::ActionMetadata const &)
{
    return unity::scopes::PreviewQuery::UPtr(new testing::Preview());
}
