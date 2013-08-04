/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/ScopeBase.h>

#define EXPORT __attribute__ ((visibility("default")))

class EXPORT MyScope : public unity::api::scopes::ScopeBase
{
public:
    MyScope() {}

    virtual int start(unity::api::scopes::RegistryProxy::SPtr const&) override { return VERSION; }
    virtual void stop() override {}
    virtual void run() override {}
    virtual void query(std::string const&, unity::api::scopes::ReplyProxy::SPtr const&) override {}

protected:
    virtual ~MyScope() {}
};

EXPORT
unity::api::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_API_SCOPE_CREATE_FUNCTION()
{
    return new MyScope;
}

EXPORT
void
// cppcheck-suppress unusedFunction
UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope)
{
    delete scope;
}
