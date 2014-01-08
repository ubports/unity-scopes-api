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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 */

#include <scopes/Category.h>
#include <scopes/CategorisedResult.h>
#include <scopes/Reply.h>
#include <scopes/Runtime.h>
#include <scopes/ScopeBase.h>

using namespace std;
using namespace unity::api::scopes;

class TestQuery : public QueryBase
{
public:
    virtual void cancelled() override
    {
    }
    virtual void run(ReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);
    }
};

class TestScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const &) override
    {
        return VERSION;
    }

    virtual void stop() override
    {
    }

    virtual void run() override
    {
    }

    virtual QueryBase::UPtr create_query(string const &, VariantMap const &) override
    {
        return QueryBase::UPtr(new TestQuery());
    }

    virtual QueryBase::UPtr preview(Result const&, VariantMap const &) override
    {
        return nullptr;
    }
};

int main(int, char **argv) {
    auto rt = Runtime::create_scope_runtime("TestScope", argv[1]);
    TestScope scope;
    rt->run_scope(&scope);
    return 0;
}
