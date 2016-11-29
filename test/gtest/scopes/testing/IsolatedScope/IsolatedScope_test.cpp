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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <unity/scopes/testing/InProcessBenchmark.h>
#include <unity/scopes/testing/OutOfProcessBenchmark.h>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/Result.h>
#include <unity/scopes/testing/ScopeMetadataBuilder.h>
#include <unity/scopes/testing/Statistics.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

#include <unity/scopes/testing/MockRegistry.h>
#include <unity/scopes/testing/MockPreviewReply.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/MockScope.h>
#include <unity/scopes/testing/MockQueryCtrl.h>

#include <unity/scopes/CategoryRenderer.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "scope.h"

namespace
{
typedef unity::scopes::testing::TypedScopeFixture<testing::Scope> TestScopeFixture;

static const std::string scope_id{"does.not.exist.scope"};
static const std::string scope_query_string{"does.not.exist.scope.query_string"};

static const std::string default_locale{"C"};
static const std::string default_form_factor{"SuperDuperPhablet"};
}

TEST(Category, construction_passes_on_arguments)
{
    const std::string id{"does.not.exist.id"};
    const std::string title{"does.not.exist.title"};
    const std::string icon{"does.not.exist.icon"};
    const unity::scopes::CategoryRenderer renderer{};

    unity::scopes::testing::Category category
    {
        id,
        title,
        icon,
        renderer
    };

    EXPECT_EQ(id, category.id());
    EXPECT_EQ(title, category.title());
    EXPECT_EQ(icon, category.icon());
    EXPECT_EQ(renderer, category.renderer_template());
}

TEST(ScopeMetadataBuilder, construct_minimal)
{
    unity::scopes::testing::ScopeMetadataBuilder builder;
    builder.scope_id(scope_id)
        .proxy(std::make_shared<unity::scopes::testing::MockScope>())
        .display_name("display_name")
        .description("description")
        .author("author");
    unity::scopes::ScopeMetadata metadata = builder();
    EXPECT_EQ(scope_id, metadata.scope_id());
    EXPECT_EQ("display_name", metadata.display_name());
    EXPECT_EQ("description", metadata.description());
    EXPECT_EQ("author", metadata.author());
}

TEST(ScopeMetadataBuilder, construct_full)
{
    unity::scopes::VariantMap appearance_attrs;
    appearance_attrs.emplace("background", unity::scopes::Variant("white"));

    unity::scopes::VariantMap a_setting;
    a_setting.emplace("number", unity::scopes::Variant(42));
    unity::scopes::VariantArray settings_defs;
    settings_defs.push_back(unity::scopes::Variant(a_setting));

    std::vector<std::string> child_scope_ids{ "joe", "mary" };
    std::set<std::string> keywords{ "people", "things" };

    unity::scopes::testing::ScopeMetadataBuilder builder;
    builder.scope_id(scope_id)
        .proxy(std::make_shared<unity::scopes::testing::MockScope>())
        .display_name("display_name")
        .description("description")
        .author("author")
        .art(std::string("art"))
        .icon(std::string("icon"))
        .search_hint(std::string("search_hint"))
        .hot_key(std::string("hot_key"))
        .invisible(true)
        .appearance_attributes(appearance_attrs)
        .scope_directory(std::string("scope_dir"))
        .results_ttl_type(unity::scopes::ScopeMetadata::ResultsTtlType::Large)
        .settings_definitions(settings_defs)
        .location_data_needed(true)
        .child_scope_ids(child_scope_ids)
        .version(42)
        .keywords(keywords)
        .is_aggregator(true);
    unity::scopes::ScopeMetadata metadata = builder();
    EXPECT_EQ(scope_id, metadata.scope_id());
    EXPECT_EQ("display_name", metadata.display_name());
    EXPECT_EQ("description", metadata.description());
    EXPECT_EQ("author", metadata.author());
    EXPECT_EQ("art", metadata.art());
    EXPECT_EQ("icon", metadata.icon());
    EXPECT_EQ("search_hint", metadata.search_hint());
    EXPECT_EQ("hot_key", metadata.hot_key());
    EXPECT_TRUE(metadata.invisible());
    EXPECT_EQ(appearance_attrs, metadata.appearance_attributes());
    EXPECT_EQ("scope_dir", metadata.scope_directory());
    EXPECT_EQ(unity::scopes::ScopeMetadata::ResultsTtlType::Large, metadata.results_ttl_type());
    EXPECT_EQ(settings_defs, metadata.settings_definitions());
    EXPECT_TRUE(metadata.location_data_needed());
    EXPECT_EQ(child_scope_ids, metadata.child_scope_ids());
    EXPECT_EQ(42, metadata.version());
    EXPECT_EQ(keywords, metadata.keywords());
    EXPECT_TRUE(metadata.is_aggregator());
}

TEST(ScopeMetadataBuilder, construction_in_case_of_missing_mandatory_arguments_aborts)
{
    unity::scopes::testing::ScopeMetadataBuilder builder;
    builder.scope_id(scope_id)
           .display_name("does.not.exist.display_name")
           .description("does.not.exist.description");

    EXPECT_EXIT(builder(), ::testing::KilledBySignal(SIGABRT), ".*");
}

TEST_F(TestScopeFixture,
       creating_a_search_query_and_checking_for_pushed_results_works)
{
    using namespace ::testing;

    const std::string id{"does.not.exist.id"};
    const std::string title{"does.not.exist.title"};
    const std::string icon{"does.not.exist.icon"};
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    EXPECT_CALL(reply, register_departments(_)).Times(1);
    EXPECT_CALL(reply, register_category(_, _, _, _))
            .Times(1)
            .WillOnce(
                Return(
                    unity::scopes::Category::SCPtr
                    {
                        new unity::scopes::testing::Category
                        {
                            id, title, icon, renderer
                        }
                    }));
    EXPECT_CALL(reply, push(Matcher<unity::scopes::experimental::Annotation const&>(_)))
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(_)))
            .Times(1)
            .WillOnce(Return(true));

    unity::scopes::SearchReplyProxy search_reply_proxy
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    unity::scopes::CannedQuery query{scope_id};
    query.set_query_string(scope_query_string);

    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(search_reply_proxy);
}

TEST_F(TestScopeFixture, performing_an_aggregating_query_works)
{
    using namespace ::testing;

    const std::string aggregator_query{"aggregator test"};

    const std::string id{"does.not.exist.id"};
    const std::string title{"does.not.exist.title"};
    const std::string icon{"does.not.exist.icon"};
    const std::string display_name{"does.not.exist.display_name"};
    const std::string description{"does.not.exist.description"};
    const std::string author{"does.not.exist.author"};
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockQueryCtrl> queryctrl;
    unity::scopes::QueryCtrlProxy queryctrl_proxy(
        &queryctrl, [](unity::scopes::QueryCtrl*) {});

    auto push_childscope_results =
        [queryctrl_proxy](Unused, Unused, Unused, Unused, unity::scopes::SearchListenerBase::SPtr const& listener) -> unity::scopes::QueryCtrlProxy {
        unity::scopes::Category::SCPtr cat(
            new unity::scopes::testing::Category(
                "id", "title", "icon", unity::scopes::CategoryRenderer()));
        listener->push(cat);

        unity::scopes::CategorisedResult result(cat);
        result.set_uri("uri");
        result.set_title("title");
        result.set_art("art");
        result.set_dnd_uri("dnd_uri");
        listener->push(result);

        listener->finished(unity::scopes::CompletionDetails(unity::scopes::CompletionDetails::OK));
        return queryctrl_proxy;
    };

    NiceMock<unity::scopes::testing::MockScope> childscope;
    EXPECT_CALL(childscope, search(aggregator_query, _, _, _, _))
        .WillOnce(Invoke(push_childscope_results));
    unity::scopes::ScopeProxy childscope_proxy(
        &childscope, [](unity::scopes::Scope*) {});

    EXPECT_CALL(registry, get_metadata("child"))
        .WillOnce(Return(
                      unity::scopes::testing::ScopeMetadataBuilder()
                      .scope_id("child")
                      .proxy(childscope_proxy)
                      .display_name(display_name)
                      .description(description)
                      .author(author)
                      ()));

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    EXPECT_CALL(reply, register_category(_, _, _, _))
            .Times(1)
            .WillOnce(
                Return(
                    unity::scopes::Category::SCPtr
                    {
                        new unity::scopes::testing::Category
                        {
                            id, title, icon, renderer
                        }
                    }));
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(_)))
            .Times(1)
            .WillOnce(Return(true));

    unity::scopes::SearchReplyProxy search_reply_proxy(
        &reply, [](unity::scopes::SearchReply*) {});

    unity::scopes::CannedQuery query(scope_id);
    query.set_query_string(aggregator_query);
    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    //search_query->set_metadata(meta_data);
    search_query->run(search_reply_proxy);
}

TEST_F(TestScopeFixture, previewing_a_result_works)
{
    using namespace ::testing;

    NiceMock<unity::scopes::testing::MockPreviewReply> reply;
    EXPECT_CALL(reply, push(A<unity::scopes::PreviewWidgetList const&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push("author", unity::scopes::Variant("Foo")))
        .WillOnce(Return(true));
    EXPECT_CALL(reply, push("rating", unity::scopes::Variant("Bar")))
        .WillOnce(Return(true));

    unity::scopes::ActionMetadata metadata(default_locale, default_form_factor);
    unity::scopes::testing::Result result;
    unity::scopes::PreviewReplyProxy proxy(&reply, [](unity::scopes::PreviewReply*) {});

    auto previewer = scope->preview(result, metadata);
    ASSERT_NE(nullptr, previewer);
    previewer->run(proxy);
}

TEST_F(TestScopeFixture, activating_a_result_works)
{
    using namespace ::testing;

    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};
    unity::scopes::testing::Result result;
    auto activation = scope->activate(result, meta_data);

    EXPECT_NE(nullptr, activation);

    EXPECT_EQ(unity::scopes::ActivationResponse::ShowDash,
              activation->activate().status());
}

TEST_F(TestScopeFixture, performing_an_action_works)
{
    using namespace ::testing;

    static const std::string action_id{"action"};
    static const std::string widget_id{"widget"};

    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};
    unity::scopes::testing::Result result;

    auto activation = scope->perform_action(
                result,
                meta_data,
                widget_id,
                action_id);

    EXPECT_NE(nullptr, activation);
    EXPECT_EQ(unity::scopes::ActivationResponse::ShowDash,
              activation->activate().status());
}
