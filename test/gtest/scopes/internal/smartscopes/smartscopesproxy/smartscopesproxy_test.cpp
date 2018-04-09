/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchMetadata.h>

#include "../RaiiServer.h"

#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

namespace
{

class smartscopesproxytest : public Test
{
public:
    smartscopesproxytest()
        : server_(FAKE_SSS_PATH)
        , reg_id_("SSRegistryTest")
        , scope_id_("SmartScopeTest")
        , ss_config_("")
    {
        // set the LANGUAGE env var
        ::setenv("LANGUAGE", "test_TEST", 1);

        // Instantiate runtime
        rt_ = RuntimeImpl::create(reg_id_, SS_RUNTIME_PATH);

        // Get registry config
        RegistryConfig reg_conf(reg_id_, rt_->registry_configfile());
        std::string mw_kind = reg_conf.mw_kind();
        std::string mw_configfile = reg_conf.mw_configfile();

        ss_config_ = SSConfig(rt_->ss_configfile());

        // Get middleware handles from runtimes
        mw_ = rt_->factory()->find(reg_id_, mw_kind);

        // Instantiate a SS registry and scope objects
        reg_ = SSRegistryObject::SPtr(new SSRegistryObject(mw_, ss_config_, mw_->get_scope_endpoint(), false));
        scope_ = SSScopeObject::UPtr(new SSScopeObject(scope_id_, mw_, reg_));

        // Add objects to the middlewares
        mw_->add_registry_object(rt_->registry_identity(), reg_);
        mw_->add_dflt_scope_object(std::move(scope_));
    }

    void reset_reg()
    {
        reg_.reset(new SSRegistryObject(mw_, ss_config_, mw_->get_scope_endpoint(), false));
    }

    ~smartscopesproxytest()
    {
        rt_->destroy();
        // zmq shutdown is asynchronous and, if we don't wait,
        // the next RuntimeImpl instance that is created may
        // fail because the zmq endpoints may not have been unlinked
        // in the filesystem yet.
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

protected:
    RaiiServer server_;

    std::string reg_id_;
    std::string scope_id_;

    RuntimeImpl::UPtr rt_;

    SSConfig ss_config_;

    MiddlewareBase::SPtr mw_;

    SSRegistryObject::SPtr reg_;
    SSScopeObject::UPtr scope_;
};

TEST_F(smartscopesproxytest, ss_registry)
{
    // non-existent scope (direct)
    EXPECT_THROW(reg_->get_metadata("dummy.scope.5"), NotFoundException);
    EXPECT_THROW(reg_->locate("dummy.scope.5"), NotFoundException);

    {
        // list scopes (direct)
        MetadataMap scopes = reg_->list();
        EXPECT_EQ(4u, scopes.size());

        // visible scope (direct)
        ScopeMetadata meta = reg_->get_metadata("dummy.scope");
        EXPECT_EQ("dummy.scope", meta.scope_id());
        EXPECT_EQ("Dummy Demo Scope", meta.display_name());
        EXPECT_EQ("Dummy demo scope.", meta.description());
        EXPECT_EQ("Mr.Fake", meta.author());
        EXPECT_EQ("icon", meta.icon());
        EXPECT_FALSE(meta.invisible());

        meta = reg_->get_metadata("dummy.scope.3");
        EXPECT_EQ("dummy.scope.3", meta.scope_id());
        EXPECT_EQ("Dummy Demo Scope 3", meta.display_name());
        EXPECT_EQ("Dummy demo scope 3.", meta.description());
        EXPECT_EQ("Mr.Fake", meta.author());
        EXPECT_FALSE(meta.invisible());
        EXPECT_EQ(4u, meta.settings_definitions().size());
        EXPECT_EQ("unitTemp", meta.settings_definitions()[1].get_dict()["id"].get_string());
        EXPECT_EQ("Temperature Units", meta.settings_definitions()[1].get_dict()["displayName"].get_string());
        EXPECT_EQ("list", meta.settings_definitions()[1].get_dict()["type"].get_string());
        EXPECT_EQ(1, meta.settings_definitions()[1].get_dict()["defaultValue"].get_int());
        EXPECT_EQ(2u, meta.settings_definitions()[1].get_dict()["displayValues"].get_array().size());
        EXPECT_EQ("Celsius", meta.settings_definitions()[1].get_dict()["displayValues"].get_array()[0].get_string());
        EXPECT_EQ("Fahrenheit", meta.settings_definitions()[1].get_dict()["displayValues"].get_array()[1].get_string());
        ASSERT_EQ(4u, meta.keywords().size());
        EXPECT_NE(meta.keywords().end(), meta.keywords().find("music"));
        EXPECT_NE(meta.keywords().end(), meta.keywords().find("video"));
        EXPECT_NE(meta.keywords().end(), meta.keywords().find("news"));
        EXPECT_NE(meta.keywords().end(), meta.keywords().find("games"));
    }

    // non-existent scope (via mw)
    MWRegistryProxy mw_reg = mw_->registry_proxy();
    EXPECT_THROW(mw_reg->get_metadata("dummy.scope.5"), NotFoundException);
    EXPECT_THROW(mw_reg->locate("dummy.scope.5"), NotFoundException);

    {
        // list scopes (via mw)
        MetadataMap scopes = mw_reg->list();
        EXPECT_EQ(4u, scopes.size());

        // visible scope (via mw)
        ScopeMetadata meta = mw_reg->get_metadata("dummy.scope");
        EXPECT_EQ("dummy.scope", meta.scope_id());
        EXPECT_EQ("Dummy Demo Scope", meta.display_name());
        EXPECT_EQ("Dummy demo scope.", meta.description());
        EXPECT_EQ("Mr.Fake", meta.author());
        EXPECT_EQ("icon", meta.icon());
        EXPECT_FALSE(meta.invisible());

        meta = mw_reg->get_metadata("dummy.scope.3");
        EXPECT_EQ("dummy.scope.3", meta.scope_id());
        EXPECT_EQ("Dummy Demo Scope 3", meta.display_name());
        EXPECT_EQ("Dummy demo scope 3.", meta.description());
        EXPECT_EQ("Mr.Fake", meta.author());
        EXPECT_FALSE(meta.invisible());
        EXPECT_EQ(4u, meta.settings_definitions().size());
        EXPECT_EQ("unitTemp", meta.settings_definitions()[1].get_dict()["id"].get_string());
        EXPECT_EQ("Temperature Units", meta.settings_definitions()[1].get_dict()["displayName"].get_string());
        EXPECT_EQ("list", meta.settings_definitions()[1].get_dict()["type"].get_string());
        EXPECT_EQ(1, meta.settings_definitions()[1].get_dict()["defaultValue"].get_int());
        EXPECT_EQ(2u, meta.settings_definitions()[1].get_dict()["displayValues"].get_array().size());
        EXPECT_EQ("Celsius", meta.settings_definitions()[1].get_dict()["displayValues"].get_array()[0].get_string());
        EXPECT_EQ("Fahrenheit", meta.settings_definitions()[1].get_dict()["displayValues"].get_array()[1].get_string());
    }
}

TEST_F(smartscopesproxytest, ss_registry_locale)
{
    // set an invalid LANGUAGE env var (should return 0 scopes)
    ::setenv("LANGUAGE", "test_FAIL", 1);
    reset_reg();
    EXPECT_EQ(0u, reg_->list().size());

    // set an empty LANGUAGE env var (should return 2 scopes)
    ::setenv("LANGUAGE", "", 1);
    reset_reg();
    EXPECT_EQ(4u, reg_->list().size());

    // set a valid LANGUAGE env var (should return 2 scopes)
    ::setenv("LANGUAGE", "test_TEST", 1);
    reset_reg();
    EXPECT_EQ(4u, reg_->list().size());

    // set a colon only LANGUAGE env var (should return 2 scopes)
    ::setenv("LANGUAGE", ":", 1);
    reset_reg();
    EXPECT_EQ(4u, reg_->list().size());

    // set a colon seperated LANGUAGE env var (first valid - should return 2 scopes)
    ::setenv("LANGUAGE", "test_TEST:test_FAIL", 1);
    reset_reg();
    EXPECT_EQ(4u, reg_->list().size());

    // set a colon seperated LANGUAGE env var (first invalid - should return 0 scopes)
    ::setenv("LANGUAGE", "test_FAIL:test_TEST", 1);
    reset_reg();
    EXPECT_EQ(0u, reg_->list().size());
}

class Receiver : public SearchListenerBase
{
public:
    virtual void push(CategorisedResult result) override
    {
        if (count_ == 0)
        {
            EXPECT_EQ("URI", result.uri());
            EXPECT_EQ("Stuff", result.title());
            EXPECT_EQ("https://dash.ubuntu.com/imgs/amazon.png", result.art());
            EXPECT_EQ("", result.dnd_uri());
            EXPECT_EQ("cat1", result.category()->id());
            EXPECT_EQ("Category 1", result.category()->title());
            EXPECT_EQ("", result.category()->icon());
            EXPECT_EQ("{}", result.category()->renderer_template().data());
        }
        else if (count_ == 1)
        {
            EXPECT_EQ("URI2", result.uri());
            EXPECT_EQ("Things", result.title());
            EXPECT_EQ("", result.art());
            EXPECT_EQ("", result.dnd_uri());
            EXPECT_EQ("cat1", result.category()->id());
            EXPECT_EQ("Category 1", result.category()->title());
            EXPECT_EQ("", result.category()->icon());
            EXPECT_EQ("{}", result.category()->renderer_template().data());
        }

        count_++;
        last_result_ = std::make_shared<Result>(result);
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        EXPECT_EQ(2, count_);

        // signal wait_until_finished
        std::unique_lock<std::mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

    std::shared_ptr<Result> last_result()
    {
        return last_result_;
    }

private:
    int count_ = 0;
    bool query_complete_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::shared_ptr<Result> last_result_;
};

TEST_F(smartscopesproxytest, search)
{
    auto reply = std::make_shared<Receiver>();

    ScopeMetadata meta = reg_->get_metadata("dummy.scope");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();

    // If the following search URIs are built incorrectly we will not get any results back
    reply = std::make_shared<Receiver>();

    meta = reg_->get_metadata("dummy.scope.3");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();

    // Now try a different remote scope
    reply = std::make_shared<Receiver>();

    meta = reg_->get_metadata("dummy.scope.4");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();
}

TEST_F(smartscopesproxytest, consecutive_queries)
{
    ScopeMetadata meta = reg_->get_metadata("dummy.scope");
    std::vector<std::shared_ptr<Receiver>> replies;

    const int iterations = 100;

    for (int i = 0; i < iterations; ++i)
    {
        replies.push_back(std::make_shared<Receiver>());
        meta.proxy()->search("search_string", SearchMetadata("en", "phone"), replies.back());
    }

    for (int i = 0; i < iterations; ++i)
    {
        replies[i]->wait_until_finished();
    }
}

class PreviewerWithCols : public PreviewListenerBase
{
public:
    virtual void push(PreviewWidgetList const& widget_list) override
    {
        widget_list_.insert(widget_list_.end(), widget_list.begin(), widget_list.end());
        widget_pushes_++;
    }

    virtual void push(std::string const&, Variant const&) override
    {
        widget_pushes_++;
    }

    virtual void push(ColumnLayoutList const& column_list) override
    {
        ASSERT_EQ(3u, column_list.size());

        // column 1
        auto it = column_list.begin();
        ASSERT_EQ(1, it->number_of_columns());
        ASSERT_EQ(3u, it->column(0).size());
        EXPECT_EQ("widget_id_A", it->column(0)[0]);
        EXPECT_EQ("widget_id_B", it->column(0)[1]);
        EXPECT_EQ("widget_id_C", it->column(0)[2]);

        // column 2
        std::advance(it, 1);
        ASSERT_EQ(2, it->number_of_columns());
        ASSERT_EQ(1u, it->column(0).size());
        EXPECT_EQ("widget_id_A", it->column(0)[0]);

        ASSERT_EQ(2u, it->column(1).size());
        EXPECT_EQ("widget_id_B", it->column(1)[0]);
        EXPECT_EQ("widget_id_C", it->column(1)[1]);

        // column 3
        std::advance(it, 1);
        ASSERT_EQ(3, it->number_of_columns());
        ASSERT_EQ(1u, it->column(0).size());
        EXPECT_EQ("widget_id_A", it->column(0)[0]);

        ASSERT_EQ(1u, it->column(1).size());
        EXPECT_EQ("widget_id_B", it->column(1)[0]);

        ASSERT_EQ(1u, it->column(2).size());
        EXPECT_EQ("widget_id_C", it->column(2)[0]);

        col_pushes_++;
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(3u, widget_list_.size());

        // widget 1
        auto it = widget_list_.begin();
        EXPECT_EQ("widget_id_A", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("Widget A", it->attribute_values()["title"].get_string());
        EXPECT_EQ("First widget.", it->attribute_values()["text"].get_string());

        // widget 2
        std::advance(it, 1);
        EXPECT_EQ("widget_id_B", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("Widget B", it->attribute_values()["title"].get_string());
        EXPECT_EQ("Second widget.", it->attribute_values()["text"].get_string());

        // widget 3
        std::advance(it, 1);
        EXPECT_EQ("widget_id_C", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("Widget C", it->attribute_values()["title"].get_string());
        EXPECT_EQ("Third widget.", it->attribute_values()["text"].get_string());


        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        EXPECT_EQ(3, widget_pushes_);
        EXPECT_EQ(1, col_pushes_);

        // Signal wait_until_finished
        std::unique_lock<std::mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    int widget_pushes_ = 0;
    int col_pushes_ = 0;
    bool query_complete_ = false;
    PreviewWidgetList widget_list_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class PreviewerNoCols : public PreviewListenerBase
{
public:
    virtual void push(PreviewWidgetList const& widget_list) override
    {
        widget_list_.insert(widget_list_.end(), widget_list.begin(), widget_list.end());
        widget_pushes_++;
    }

    virtual void push(std::string const&, Variant const&) override
    {
        widget_pushes_++;
    }

    virtual void push(ColumnLayoutList const&) override
    {
        col_pushes_++;
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        EXPECT_EQ(2, widget_pushes_);
        EXPECT_EQ(0, col_pushes_);

        EXPECT_EQ(2u, widget_list_.size());

        // widget 1
        auto it = widget_list_.begin();
        EXPECT_EQ("widget_id_A", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("Widget A", it->attribute_values()["title"].get_string());
        EXPECT_EQ("First widget.", it->attribute_values()["text"].get_string());

        // widget 2
        std::advance(it, 1);
        EXPECT_EQ("widget_id_B", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("Widget B", it->attribute_values()["title"].get_string());
        EXPECT_EQ("Second widget.", it->attribute_values()["text"].get_string());

        // Signal wait_until_finished
        std::unique_lock<std::mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    int widget_pushes_ = 0;
    int col_pushes_ = 0;
    bool query_complete_ = false;
    std::mutex mutex_;
    std::condition_variable cond_;
    PreviewWidgetList widget_list_;
};

TEST_F(smartscopesproxytest, preview)
{
    auto reply = std::make_shared<Receiver>();

    ScopeMetadata meta = reg_->get_metadata("dummy.scope");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();

    auto result = reply->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    // with columns returned
    auto previewer_with_cols = std::make_shared<PreviewerWithCols>();

    meta.proxy()->preview(*(result.get()), ActionMetadata("en", "phone"), previewer_with_cols);
    previewer_with_cols->wait_until_finished();

    // without columns returned
    auto previewer_no_cols = std::make_shared<PreviewerNoCols>();

    meta.proxy()->preview(*(result.get()), ActionMetadata("en", "phone"), previewer_no_cols);
    previewer_no_cols->wait_until_finished();

    // If the following preview URIs are built incorrectly we will not get any results back
    reply = std::make_shared<Receiver>();

    meta = reg_->get_metadata("dummy.scope.3");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();

    result = reply->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    previewer_with_cols = std::make_shared<PreviewerWithCols>();

    meta.proxy()->preview(*(result.get()), ActionMetadata("en", "phone"), previewer_with_cols);
    previewer_with_cols->wait_until_finished();

    // Now try a different remote scope
    reply = std::make_shared<Receiver>();

    meta = reg_->get_metadata("dummy.scope.4");

    meta.proxy()->search("search_string", SearchMetadata("en", "phone"), reply);
    reply->wait_until_finished();

    result = reply->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    previewer_with_cols = std::make_shared<PreviewerWithCols>();

    meta.proxy()->preview(*(result.get()), ActionMetadata("en", "phone"), previewer_with_cols);
    previewer_with_cols->wait_until_finished();
}

} // namespace
