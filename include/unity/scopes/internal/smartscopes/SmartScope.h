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
  SmartQuery(std::string const& id, SSRegistryObject::SPtr reg, std::string const& query) :
    scope_id_(id),
    query_(query)
  {
    SmartScopesClient::SPtr ss_client = reg->get_ssclient();
    std::string base_url = reg->get_base_url(scope_id_);

    search_handle_ = ss_client->search(base_url, query_, "session_id", 0,
                                       "platform", "en", "US", "0", "0", 10);
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

    for( auto& result : results )
    {
      if (categories.find(result.category->id) == end(categories))
      {
        CategoryRenderer rdr(result.category->renderer_template);
        Category::SCPtr cat = reply->register_category(result.category->id, result.category->title,
                                                       result.category->icon, rdr);
        categories[result.category->id] = cat;
      }

      Category::SCPtr cat = categories[result.category->id];
      CategorisedResult res(cat);
      res.set_uri(result.uri);
      res.set_title(result.title);
      res.set_art(result.art);
      res.set_dnd_uri(result.dnd_uri);
      reply->push(res);
    }

    std::cout << "SmartScope: query for \"" << scope_id_ << "\": \"" << query_ << "\" complete" << std::endl;
  }

private:
  std::string scope_id_;
  std::string query_;
  SearchHandle::UPtr search_handle_;
};

class SmartScope
{
public:
  SmartScope(SSRegistryObject::SPtr reg) :
    reg_(reg)
  {
  }

  QueryBase::UPtr create_query(std::string const& id, std::string const& q, VariantMap const&)
  {
    QueryBase::UPtr query(new SmartQuery(id, reg_, q));
    std::cout << "SmartScope: created query for \"" << id << "\": \"" << q << "\"" << std::endl;
    return query;
  }

private:
  SSRegistryObject::SPtr reg_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity
