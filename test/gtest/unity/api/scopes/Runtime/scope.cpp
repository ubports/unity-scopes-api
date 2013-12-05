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

#include <iostream>

#include <scopes/Runtime.h>
#include <scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::api::scopes;

class TestQuery : public QueryBase
{
public:
    virtual void cancelled() override
    {
    }
    virtual void run(ReplyProxy const&) override
    {
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
        QueryBase::UPtr query(new TestQuery());
        return query;
    }
};

int main(int, char **argv) {
    auto rt = Runtime::create(argv[1]);
    TestScope scope;
    try {
        rt->run_scope("TestScope", &scope);
    } catch (unity::Exception const &e) {
        cerr << e.to_string() << std::endl;
    }
    return 0;
}
