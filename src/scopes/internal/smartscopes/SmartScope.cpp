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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 *              Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/SmartScope.h>

#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/SearchReply.h>

#include <iostream>

using namespace unity::scopes;
using namespace unity::scopes::internal::smartscopes;

SmartQuery::SmartQuery(std::string const& scope_id, SSRegistryObject::SPtr reg, CannedQuery const& query, SearchMetadata const& hints)
    : SearchQueryBase(query, hints)
    , scope_id_(scope_id)
    , query_(query)
    , ss_client_(reg->get_ssclient())
    , base_url_(reg->get_base_url(scope_id))
    , hints_(hints)
{
}

SmartQuery::~SmartQuery()
{
}

void SmartQuery::cancelled()
{
    if (search_handle_ != nullptr)
    {
        search_handle_->cancel_search();
    }
}

void SmartQuery::run(SearchReplyProxy const& reply)
{
    static const std::string no_net_hint("no-internet");
    if (hints_.contains_hint(no_net_hint))
    {
        auto var = hints_[no_net_hint];
        if (var.which() == Variant::Type::Bool && var.get_bool())
        {
            std::cout << "SmartQuery(): networking disabled for remote scope " << scope_id_ << ", skipping" << std::endl;
            return;
        }
    }

    struct FiltersHandler
    {
        FiltersHandler(SearchReplyProxy reply, std::string const& scope_id):
            reply(reply),
            scope_id(scope_id),
            has_filters(false),
            has_state(false)
        {}

        void push() {
            if (has_filters && has_state) {
                try
                {
                    reply->push(filters, state);
                }
                catch (std::exception const& e)
                {
                    std::cerr << "SmartScope::run(): Failed to register filters for scope '" << scope_id << "': "
                        << e.what() << std::endl;
                }
            }
        }

        void set(Filters const& f) {
            filters = f;
            has_filters = true;
            push();
        }

        void set(FilterState const& s) {
            state = s;
            has_state = true;
            push();
        }

        SearchReplyProxy reply;
        std::string scope_id;
        bool has_filters;
        bool has_state;
        Filters filters;
        FilterState state;
    } filters_data(reply, scope_id_);

    SearchReplyHandler handler;
    handler.filters_handler = [&filters_data](Filters const &filters) {
        filters_data.set(filters);
    };
    handler.filter_state_handler = [&filters_data](FilterState const& state) {
        filters_data.set(state);
    };
    handler.category_handler = [this, reply](std::shared_ptr<SearchCategory> const& category) {
        const CategoryRenderer rdr(category->renderer_template);
        try
        {
            reply->register_category(category->id, category->title, category->icon, rdr);
        }
        catch (std::exception const& e)
        {
            std::cerr << "SmartScope: failed to register category: \"" << category->id
                << "\" for scope \"" << scope_id_ << "\" and query: \""
                << query_.query_string() << std::endl;
        }
    };
    handler.result_handler = [this, reply](SearchResult const& result) {
        auto cat = reply->lookup_category(result.category_id);
        if (cat)
        {
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
        else
        {
            std::cerr << "SmartScope: result for query: \"" << scope_id_ << "\": \"" << query_.query_string()
                      << "\" returned an invalid cat_id. Skipping result." << std::endl;
        }
    };
    handler.departments_handler = [this, reply](std::shared_ptr<DepartmentInfo> const& deptinfo) {
        try
        {
            Department::SCPtr root = create_department(deptinfo);
            reply->register_departments(root);
        }
        catch (std::exception const& e)
        {
            std::cerr << "SmartScope::run(): Failed to register departments for scope '" << scope_id_ << "': "
                      << e.what() << std::endl;
        }
    };

    ///! TODO: country (+location data)
    int query_id = 0;
    std::string session_id;
    std::string agent;
    auto const metadata = search_metadata();
    if (metadata.contains_hint("session-id") && metadata["session-id"].which() == Variant::String)
    {
        session_id = metadata["session-id"].get_string();
    }
    else
    {
        session_id = "session_id_missing";
        std::cout << "SmartScope: missing or invalid session id for \"" << scope_id_ << "\": \"" << query_.query_string() << "\"" << std::endl;
    }
    if (metadata.contains_hint("query-id") && metadata["query-id"].which() == Variant::Int)
    {
        query_id = metadata["query-id"].get_int();
    }
    else
    {
        std::cout << "SmartScope: missing or invalid query id for \"" << scope_id_ << "\": \"" << query_.query_string() << "\"" << std::endl;
    }
    if (metadata.contains_hint("user-agent") && metadata["user-agent"].which() == Variant::String)
    {
        agent = metadata["user-agent"].get_string();
    }

    search_handle_ = ss_client_->search(handler, base_url_, query_.query_string(), query_.department_id(), session_id, query_id, hints_.form_factor(),
            settings(), query_.filter_state().serialize(), hints_.locale(), "", agent, hints_.cardinality());
    search_handle_->wait();

    std::cout << "SmartScope: query for \"" << scope_id_ << "\": \"" << query_.query_string() << "\" complete" << std::endl;
}

Department::SCPtr SmartQuery::create_department(std::shared_ptr<DepartmentInfo const> const& deptinfo)
{
    CannedQuery const query = CannedQuery::from_uri(deptinfo->canned_query);
    Department::SPtr dep = Department::create(query, deptinfo->label);
    if (!deptinfo->alternate_label.empty())
    {
        dep->set_alternate_label(deptinfo->alternate_label);
    }
    dep->set_has_subdepartments(deptinfo->has_subdepartments);
    for (auto const& d: deptinfo->subdepartments)
    {
        dep->add_subdepartment(create_department(d));
    }
    return dep;
}

SmartPreview::SmartPreview(std::string const& scope_id, SSRegistryObject::SPtr reg, Result const& result, ActionMetadata const& hints)
    : PreviewQueryBase(result, hints)
    , scope_id_(scope_id)
    , result_(result)
    , ss_client_(reg->get_ssclient())
    , base_url_(reg->get_base_url(scope_id))
    , hints_(hints)
{
}

SmartPreview::~SmartPreview()
{
}

void SmartPreview::cancelled()
{
    preview_handle_->cancel_preview();
}

void SmartPreview::run(PreviewReplyProxy const& reply)
{
    PreviewReplyHandler handler;
    handler.widget_handler = [reply](std::string const& widget_json) {
        reply->push({PreviewWidget(widget_json)});
    };
    handler.columns_handler = [reply](PreviewHandle::Columns const &columns) {
        if (columns.size() > 0)
        {
            // register layout
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
    };

    ///! TODO: country (+location data)
    std::string session_id;
    std::string agent;
    auto const metadata = action_metadata();
    if (metadata.contains_hint("session-id") && metadata["session-id"].which() == Variant::String)
    {
        session_id = metadata["session-id"].get_string();
    }
    else
    {
        session_id = "session_id_missing";
        std::cout << "SmartScope: missing or invalid session id for \"" << scope_id_ << "\" preview: \"" << result().uri() << "\"" << std::endl;
    }

    if (metadata.contains_hint("user-agent") && metadata["user-agent"].which() == Variant::String)
    {
        agent = metadata["user-agent"].get_string();
    }

    preview_handle_ = ss_client_->preview(handler, base_url_, result_["result_json"].get_string(), session_id, hints_.form_factor(), 0, settings(),
            hints_.locale(), "", agent);

    preview_handle_->wait();

    std::cout << "SmartScope: preview for \"" << scope_id_ << "\": \"" << result().uri() << "\" complete" << std::endl;
}

SmartActivation::SmartActivation(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id)
    : ActivationQueryBase(result, metadata, widget_id, action_id)
{
}

ActivationResponse SmartActivation::activate()
{
    ///! TODO: need SSS endpoint for this
    return ActivationResponse(ActivationResponse::Status::NotHandled);
}

NullActivation::NullActivation(Result const& result, ActionMetadata const& metadata)
    : ActivationQueryBase(result, metadata)
{
}

ActivationResponse NullActivation::activate()
{
    return ActivationResponse(ActivationResponse::Status::NotHandled);
}

SmartScope::SmartScope(SSRegistryObject::SPtr reg)
    : reg_(reg)
{
}

SearchQueryBase::UPtr SmartScope::search(std::string const& scope_id, CannedQuery const& q, SearchMetadata const& hints)
{
    SearchQueryBase::UPtr query(new SmartQuery(scope_id, reg_, q, hints));
    std::cout << "SmartScope: created query for \"" << scope_id << "\": \"" << q.query_string() << "\"" << std::endl;
    return query;
}

QueryBase::UPtr SmartScope::preview(std::string const& scope_id, Result const& result, ActionMetadata const& hints)
{
    QueryBase::UPtr preview(new SmartPreview(scope_id, reg_, result, hints));
    std::cout << "SmartScope: created preview for \"" << scope_id << "\": \"" << result.uri() << "\"" << std::endl;
    return preview;
}

ActivationQueryBase::UPtr SmartScope::activate(std::string const&, Result const& result, ActionMetadata const& metadata)
{
    ActivationQueryBase::UPtr activation(new NullActivation(result, metadata));
    return activation;
}

ActivationQueryBase::UPtr SmartScope::perform_action(std::string const& scope_id, Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id)
{
    ActivationQueryBase::UPtr activation(new SmartActivation(result, metadata, widget_id, action_id));
    std::cout << "SmartScope: created activation for \"" << scope_id << "\": \"" << result.uri() << "\"" << std::endl;
    return activation;
}
