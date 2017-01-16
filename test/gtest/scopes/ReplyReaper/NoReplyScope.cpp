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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "NoReplyScope.h"

#include <unity/scopes/ScopeBase.h>

#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;

namespace
{

class Query : public SearchQueryBase
{
public:
    Query(CannedQuery const& query, SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
    {
    }

    virtual void run(SearchReplyProxy const&) override
    {
        this_thread::sleep_for(chrono::seconds(3));
    }

    virtual void cancelled() override
    {
    }
};

}  // namespace

SearchQueryBase::UPtr NoReplyScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    return SearchQueryBase::UPtr(new Query(query, metadata));
}

PreviewQueryBase::UPtr NoReplyScope::preview(Result const&, ActionMetadata const&)
{
    return nullptr;  // Not called
}
