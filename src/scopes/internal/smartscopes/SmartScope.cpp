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

    ///! TODO: session_id, query_id, country (+location data)
    search_handle_ = ss_client_->search(base_url_, query_.query_string(), query_.department_id(), "session_id", 0, hints_.form_factor(), settings(), query_.filter_state().serialize(), hints_.locale(), "", hints_.cardinality());

    SearchRequestResults results = search_handle_->get_search_results();
    std::map<std::string, Category::SCPtr> categories;

    if (std::get<0>(results)) // are there any departments?
    {
        try
        {
            auto const& deptinfo = std::get<0>(results);
            Department::SCPtr root = create_department(deptinfo);
            reply->register_departments(root);
        }
        catch (std::exception const& e)
        {
            std::cerr << "SmartScope::run(): Failed to register departments for scope '" << scope_id_ << "': "
                      << e.what() << std::endl;
        }
    }

    auto const& filters = std::get<1>(results);
    if (!filters.empty())
    {
        auto const& filter_state = std::get<2>(results);
        try
        {
            reply->push(filters, filter_state);
        }
        catch (std::exception const& e)
        {
            std::cerr << "SmartScope::run(): Failed to register filters for scope '" << scope_id_ << "': "
                      << e.what() << std::endl;
        }
    }

    for (auto& result : std::get<3>(results))
    {
        if (!result.category)
        {
            std::cerr << "SmartScope: result for query: \"" << scope_id_ << "\": \"" << query_.query_string()
                      << "\" returned an invalid cat_id. Skipping result." << std::endl;
            continue;
        }

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
    ///! TODO: session_id, widgets_api_version, country (+location data)
    preview_handle_ = ss_client_->preview(base_url_, result_["result_json"].get_string(), "session_id", hints_.form_factor(), 0, settings(), hints_.locale(), "");

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
