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

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/SearchReply.h>
#include <unity/scopes/ScopeBase.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;

class PusherQuery : public SearchQueryBase
{
public:
    PusherQuery(int cardinality)
        : cardinality_(cardinality)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat1", "Category 1", "");

        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");

        // We push 100 results. we check that last valid push
        // and the ones following it return false.
        for (int i = 1; i <= 100; ++i)
        {
            EXPECT_EQ(i < cardinality_, reply->push(res));
        }
    }

private:
    int cardinality_;
};

int PusherScope::start(string const&, RegistryProxy const &)
{
    return VERSION;
}

void PusherScope::stop()
{
}

void PusherScope::run()
{
}

SearchQueryBase::UPtr PusherScope::search(CannedQuery const& /* query */, SearchMetadata const& md)
{
    return SearchQueryBase::UPtr(new PusherQuery(md.cardinality()));
}

PreviewQueryBase::UPtr PusherScope::preview(Result const& /* result */, ActionMetadata const& /* metadata */)
{
    abort();  // Not called
}
