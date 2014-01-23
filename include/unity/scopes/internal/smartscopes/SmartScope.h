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
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Query.h>
#include <unity/scopes/Annotation.h>

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
    SmartQuery(std::string const& query) :
        query_(query)
    {
    }

    ~SmartQuery() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        CategoryRenderer rdr;
        auto cat = reply->register_category("cat1", "Category 1", "", rdr);
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("SmartScope: result 1 for query \"" + query_ + "\"");
        res.set_art("icon");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);

        Query q("SmartScope", query_, "");
        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("More...", q);
        reply->push(annotation);

        std::cout << "SmartScope: query \"" << query_ << "\" complete" << std::endl;
    }

private:
    std::string query_;
};

class SmartPreview : public PreviewQuery
{
public:
    SmartPreview(std::string const& uri) :
        uri_(uri)
    {
    }

    ~SmartPreview() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        widgets.emplace_back(PreviewWidget(R"({"type": "image", "art": "screenshot-url"})"));
        reply->push(widgets);
        reply->push("author", Variant("Foo"));
        reply->push("rating", Variant("4 blah"));
        std::cout << "SmartScope: preview for \"" << uri_ << "\" complete" << std::endl;
    }

private:
    std::string uri_;
};

class SmartScope
{
public:
    QueryBase::UPtr create_query(std::string const& id, std::string const& q, VariantMap const&)
    {
        QueryBase::UPtr query(new SmartQuery(q));
        std::cout << "SmartScope: created query: \"" << q << "\"" << std::endl;
        return query;
    }

    QueryBase::UPtr preview(Result const& result, VariantMap const&)
    {
        QueryBase::UPtr preview(new SmartPreview(result.uri()));
        std::cout << "SmartScope: created previewer: \"" << result.uri() << "\"" << std::endl;
        return preview;
    }
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity
