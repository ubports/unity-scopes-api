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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/internal/SearchReply.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/ScopeBase.h>

#ifndef UNITY_SCOPES_TEST_SCOPE_H
#define UNITY_SCOPES_TEST_SCOPE_H

namespace unity
{

namespace scopes
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& q) :
        SearchQueryBase(),
        query_(q)
    {
    }

    virtual void cancelled() override {}

    virtual void run(SearchReplyProxy const& reply) override
    {
        Filters filters;
        auto filter = OptionSelectorFilter::create("f1", "Choose an option", false);
        filter->add_option("o1", "Option 1");
        filter->add_option("o2", "Option 2");
        filters.push_back(filter);
        auto active_opts = filter->active_options(query_.filter_state());
        reply->push(filters, query_.filter_state()); // send unmodified state back

        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);
    }

private:
    CannedQuery query_;
};

class TestScope : public ScopeBase
{
public:
    virtual int start(std::string const&, RegistryProxy const &) override
    {
        return VERSION;
    }

    virtual void stop() override {}
    virtual void run() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const &q, SearchMetadata const &) override
    {
        return SearchQueryBase::UPtr(new TestQuery(q));
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const &) override
    {
        return nullptr;
    }
};

} // namespace scopes

} // namespace unity

#endif
