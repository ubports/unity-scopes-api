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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma once

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/FilterGroup.h>
#include <unity/scopes/FilterOption.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

namespace unity
{

namespace scopes
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& q, SearchMetadata const& metadata) :
        SearchQueryBase(q, metadata)
    {
    }

    virtual void cancelled() override {}

    virtual void run(SearchReplyProxy const& reply) override
    {
        Filters filters;
        OptionSelectorFilter::SPtr filter = OptionSelectorFilter::create("f1", "Choose an option", FilterGroup::create("g1", "Group"), false);
        filter->add_option("o1", "Option 1");
        filter->add_option("o2", "Option 2");
        filters.push_back(filter);
        auto active_opts = filter->active_options(query().filter_state());

        if (query().query_string() == "test")
        {
            reply->push(filters);
        }
        else
        {
            // use the deprecated push call
            reply->push(filters, query().filter_state());
        }

        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        if (active_opts.size() == 1)
        {
            res.set_uri("option " + (*(active_opts.begin()))->id() + " active");
        }
        else
        {
            res.set_uri("no options active");
        }
        res.set_dnd_uri("dnd_uri");
        reply->push(res);
    }
};

class TestScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const &q, SearchMetadata const &metadata) override
    {
        return SearchQueryBase::UPtr(new TestQuery(q, metadata));
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const &) override
    {
        return nullptr;
    }
};

} // namespace scopes

} // namespace unity
