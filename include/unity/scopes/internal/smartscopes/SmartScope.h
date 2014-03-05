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

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/PreviewReply.h>
#include <unity/scopes/internal/SearchReply.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ScopeBase.h>

#include <iostream>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartQuery : public SearchQueryBase
{
public:
    SmartQuery(std::string const& scope_id, SSRegistryObject::SPtr reg, CannedQuery const& query, SearchMetadata const& hints)
        : scope_id_(scope_id)
        , query_(query)
    {
        SmartScopesClient::SPtr ss_client = reg->get_ssclient();
        std::string base_url = reg->get_base_url(scope_id_);

        ///! TODO: session_id, query_id, locale, country
        search_handle_ = ss_client->search(base_url, query_.query_string(), "session_id", 0, hints.form_factor(), "", "", hints.cardinality());
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
            res["result_json"] = result.json;

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
    CannedQuery query_;
    SearchHandle::UPtr search_handle_;
};

class SmartPreview : public PreviewQueryBase
{
public:
    SmartPreview(std::string const& scope_id, SSRegistryObject::SPtr reg, Result const& result, ActionMetadata const& hints)
        : scope_id_(scope_id)
        , result_(result)
    {
        SmartScopesClient::SPtr ss_client = reg->get_ssclient();
        std::string base_url = reg->get_base_url(scope_id_);

        ///! TODO: session_id, widgets_api_version, locale, country
        preview_handle_ = ss_client->preview(base_url, result_["result_json"].get_string(), "session_id", hints.form_factor(), 0, "", "");
    }

    ~SmartPreview()
    {
    }

    virtual void cancelled() override
    {
        preview_handle_->cancel_preview();
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        auto results = preview_handle_->get_preview_results();
        PreviewHandle::Columns columns = results.first;
        PreviewHandle::Widgets widgets = results.second;

        // register layout
        if (columns.size() > 0)
        {
            ColumnLayoutList layout_list;

            for (auto& column : columns)
            {
                ColumnLayout layout(column.size());
                for (auto& widget_lo : column)
                {
                    layout.add_column(widget_lo);
                }

                layout_list.emplace_back(layout);
            }

            reply->register_layout(layout_list);
        }

        // push wigdets
        PreviewWidgetList widget_list;

        for (auto& widget_json : widgets)
        {
            widget_list.emplace_back(PreviewWidget(widget_json));
        }

        reply->push(widget_list);

        std::cout << "SmartScope: preview for \"" << scope_id_ << "\": \"" << result_.uri() << "\" complete" << std::endl;
    }

private:
    std::string scope_id_;
    Result result_;
    PreviewHandle::UPtr preview_handle_;
};

class NullActivation : public ActivationQueryBase
{
    ActivationResponse activate() override
    {
        return ActivationResponse(ActivationResponse::Status::NotHandled);
    }
};

class SmartActivation : public ActivationQueryBase
{
    ActivationResponse activate() override
    {
        ///! TODO: need SSS endpoint for this
        return ActivationResponse(ActivationResponse::Status::NotHandled);
    }
};

class SmartScope
{
public:
    SmartScope(SSRegistryObject::SPtr reg)
        : reg_(reg)
    {
    }

    QueryBase::UPtr search(std::string const& scope_id, CannedQuery const& q, SearchMetadata const& hints)
    {
        QueryBase::UPtr query(new SmartQuery(scope_id, reg_, q, hints));
        std::cout << "SmartScope: created query for \"" << scope_id << "\": \"" << q.query_string() << "\"" << std::endl;
        return query;
    }

    QueryBase::UPtr preview(std::string const& scope_id, Result const& result, ActionMetadata const& hints)
    {
        QueryBase::UPtr preview(new SmartPreview(scope_id, reg_, result, hints));
        std::cout << "SmartScope: created preview for \"" << scope_id << "\": \"" << result.uri() << "\"" << std::endl;
        return preview;
    }

    ActivationQueryBase::UPtr activate(std::string const&, Result const&, ActionMetadata const&)
    {
        ActivationQueryBase::UPtr activation(new NullActivation());
        return activation;
    }

    ActivationQueryBase::UPtr perform_action(std::string const& scope_id, Result const& result, ActionMetadata const& /*hints*/, std::string const& /*widget_id*/, std::string const& /*action_id*/)
    {
        ActivationQueryBase::UPtr activation(new SmartActivation());
        std::cout << "SmartScope: created activation for \"" << scope_id << "\": \"" << result.uri() << "\"" << std::endl;
        return activation;
    }

private:
    SSRegistryObject::SPtr reg_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity
