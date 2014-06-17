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

#include "SlowSearchScope.h"

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <mutex>
#include <thread>

using namespace std;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& /* reply */) override
    {
    }
};

}  // namespace

void SlowSearchScope::start(string const&, RegistryProxy const &)
{
}

void SlowSearchScope::stop()
{
}

void SlowSearchScope::run()
{
}

SearchQueryBase::UPtr SlowSearchScope::search(CannedQuery const&, SearchMetadata const &)
{
    this_thread::sleep_for(chrono::seconds(4));
    return SearchQueryBase::UPtr(new TestQuery());
}

PreviewQueryBase::UPtr SlowSearchScope::preview(Result const&, ActionMetadata const &)
{
    return nullptr;  // unused
}
