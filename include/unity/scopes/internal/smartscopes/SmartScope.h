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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Query.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Query.h>
#include <unity/scopes/Annotation.h>

#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>

#include <iostream>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartQuery : public SearchQuery
{
public:
    SmartQuery(std::string const& id, SSRegistryObject::SPtr reg, Query const& query)
        : scope_id_(id)
        , query_(query)
    {
        SmartScopesClient::SPtr ss_client = reg->get_ssclient();
        std::string base_url = reg->get_base_url(scope_id_);

        ///! TODO: session_id, query_id, platform, locale, country, lat, long, limit
        search_handle_ = ss_client->search(base_url, query_.query_string(), "session_id", 0, "platform", "en", "US", "0", "0", 10);
    }

    ~SmartQuery() noexcept
    {
    }

    virtual void cancelled() override
    {
        search_handle_->cancel_search();
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        std::vector<SearchResult> results = search_handle_->get_search_results();
        std::map<std::string, Category::SCPtr> categories;

        for (auto& result : results)
        {
            Category::SCPtr cat;

            auto cat_it = categories.find(result.category->id);
            if (cat_it == end(categories))
            {
                CategoryRenderer rdr(result.category->renderer_template);
                cat = reply->register_category(result.category->id, result.category->title, result.category->icon, rdr);
                categories[result.category->id] = cat;
            }
            else
            {
                cat = cat_it->second;
            }

            CategorisedResult res(cat);
            res.set_uri(result.uri);
            res.set_title(result.title);
            res.set_art(result.art);
            res.set_dnd_uri(result.dnd_uri);

            auto other_params = result.other_params;
            for (auto& param : other_params)
            {
                res[param.first] = param.second->to_variant();
            }

            reply->push(res);
        }

        std::cout << "SmartScope: query for \"" << scope_id_ << "\": \"" << query_.query_string() << "\" complete" << std::endl;
    }

private:
    std::string scope_id_;
    Query query_;
    SearchHandle::UPtr search_handle_;
};

class SmartPreview : public PreviewQuery
{
public:
    SmartPreview(std::string const& id, SSRegistryObject::SPtr reg, Result const& result)
        : scope_id_(id)
        , result_(result)
    {
        SmartScopesClient::SPtr ss_client = reg->get_ssclient();
        std::string base_url = reg->get_base_url(scope_id_);

        //search_handle_ = ss_client->search(base_url, query_.query_string(), "session_id", 0, "platform", "en", "US", "0", "0", 10);
    }

    ~SmartPreview()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        (void)reply;
    }

private:
    std::string scope_id_;
    Result result_;
};

class SmartScope
{
public:
    SmartScope(SSRegistryObject::SPtr reg)
        : reg_(reg)
    {
    }

    QueryBase::UPtr create_query(std::string const& id, Query const& q, VariantMap const&)
    {
        QueryBase::UPtr query(new SmartQuery(id, reg_, q));
        std::cout << "SmartScope: created query for \"" << id << "\": \"" << q.query_string() << "\"" << std::endl;
        return query;
    }

    QueryBase::UPtr preview(std::string const& id, Result const& result, VariantMap const&)
    {
        QueryBase::UPtr preview(new SmartPreview(id, reg_, result));
        std::cout << "SmartScope: created preview for \"" << id << "\": \"" << result.uri() << "\"" << std::endl;
        return preview;
    }

    ActivationBase::UPtr activate(std::string const& id, Result const& result, VariantMap const& hints)
    {
        ///! TODO
        (void)id;
        (void)result;
        (void)hints;
        return ActivationBase::UPtr();
    }

    ActivationBase::UPtr activate_preview_action(std::string const& id, Result const& result, VariantMap const& hints, std::string const& action_id)
    {
        ///! TODO
        (void)id;
        (void)result;
        (void)hints;
        (void)action_id;
        return ActivationBase::UPtr();
    }

private:
    SSRegistryObject::SPtr reg_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity
