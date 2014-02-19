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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "PusherScope.h"

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/Query.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;

class PusherQuery : public SearchQuery
{
public:
    PusherQuery(int cardinality)
        : cardinality_(cardinality)
    {
cerr << "PusherQuery(" << cardinality << ")" << endl;
    }

    virtual void cancelled() override
    {
cerr << "Query cancelled" << endl;
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
cerr << "pusher query run" << endl;
        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");

        // We push 100 results. If cardinality_ is less than 100
        // we check that last valid push and the ones following
        // it return false.
        for (int i = 1; i <= 100; ++i)
        {
            bool b = reply->push(res);
            if (cardinality_ < 100)
            {
                EXPECT_EQ(i < cardinality_ ? true : false, b);
            }
        }
    }

private:
    int cardinality_;
};

int PusherScope::start(string const&, RegistryProxy const &)
{
cerr << "PusherScope start" << endl;
    return VERSION;
}

void PusherScope::stop()
{
}

void PusherScope::run()
{
cerr << "PusherScope run" << endl;
}

QueryBase::UPtr PusherScope::create_query(Query const& /* query */, SearchMetadata const& md)
{
cerr << "creating pusher query" << endl;
    return QueryBase::UPtr(new PusherQuery(md.cardinality()));
}
