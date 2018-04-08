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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/SearchReply.h>

using namespace unity::scopes;
using namespace unity::scopes::internal::smartscopes;

SmartQuery::SmartQuery(std::string const& scope_id, SSRegistryObject::SPtr reg, CannedQuery const& query, SearchMetadata const& hints)
    : SearchQueryBase(query, hints)
    , scope_id_(scope_id)
    , query_(query)
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
    struct FiltersHandler
    {
        FiltersHandler(SearchReplyProxy reply, std::string const& scope_id):
            reply(reply),
            scope_id(scope_id),
            has_filters(false),
            has_state(false)
        {}

        void push() {
            if (has_filters && has_state)
            {
                reply->push(filters, state);
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
    handler.filters_handler = [this, &filters_data](Filters const &filters) {
        try
        {
            filters_data.set(filters);
        }
        catch (std::exception const& e)
        {
        }
    };
    handler.filter_state_handler = [this, &filters_data](FilterState const& state) {
        try
        {
            filters_data.set(state);
        }
        catch (std::exception const& e)
        {
        }
    };
    handler.category_handler = [this, reply](std::shared_ptr<SearchCategory> const& category) {
        const CategoryRenderer rdr(category->renderer_template);
        try
        {
            reply->register_category(category->id, category->title, category->icon, rdr);
        }
        catch (std::exception const&)
        {
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
        }
    };


    ///! TODO: detailed location data
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
    }
    if (metadata.contains_hint("query-id") && metadata["query-id"].which() == Variant::Int)
    {
        query_id = metadata["query-id"].get_int();
    }
    else
    {
    }
    if (metadata.contains_hint("user-agent") && metadata["user-agent"].which() == Variant::String)
    {
        agent = metadata["user-agent"].get_string();
    }

    LocationInfo loc;
    if (metadata.has_location())
    {
        loc.has_location = true;
        auto location = metadata.location();
        if (location.has_country_code())
        {
            loc.country_code = location.country_code();
        }
        loc.longitude = location.longitude();
        loc.latitude = location.latitude();
    }
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

    ///! TODO: country (+location data) - Preview metadata does not currently contain location data
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
    }

    if (metadata.contains_hint("user-agent") && metadata["user-agent"].which() == Variant::String)
    {
        agent = metadata["user-agent"].get_string();
    }
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
    return query;
}

QueryBase::UPtr SmartScope::preview(std::string const& scope_id, Result const& result, ActionMetadata const& hints)
{
    QueryBase::UPtr preview(new SmartPreview(scope_id, reg_, result, hints));
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
    return activation;
}

ActivationQueryBase::UPtr SmartScope::activate_result_action(std::string const& scope_id, Result const& result, ActionMetadata const& metadata, std::string const& action_id)
{
    // NOTE: there is no endpoint for it really, SmartActivation is a no-op (not-handled)
    ActivationQueryBase::UPtr activation(new SmartActivation(result, metadata, "", action_id));
    return activation;
}
