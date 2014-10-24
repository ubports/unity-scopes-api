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

// You may also include individual headers if you prefer.
#include <unity-scopes.h>
#include <thread>
#include <chrono>

#include <sstream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(string const& scope_id, CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata),
        scope_id_(scope_id)
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        CategoryRenderer rdr;
        auto cat = reply->register_category("cat1", "Category 1", "", rdr);
        for (int i = 0; i<20; i++)
        {
            std::stringstream s;
            s << i;
            CategorisedResult res(cat);
            res.set_uri("uri" + s.str());
            res.set_title("title");
            res.set_art("icon");
            res.set_dnd_uri("dnd_uri");
            if (!reply->push(res))
            {
                return;
            }
            this_thread::sleep_for(chrono::milliseconds(60));
        }
    }

private:
    string scope_id_;
};

class MyScope : public ScopeBase
{
public:
    virtual void start(string const& scope_id) override
    {
        scope_id_ = scope_id;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(scope_id_, q, metadata));
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        return nullptr;
    }

private:
    string scope_id_;
};

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
