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

// Force Version.h to use the specified version, so we get coverage on the version check.

#define UNITY_SCOPES_VERSION_MAJOR 666

#include "Counters.h"

#include <unity/scopes/internal/ScopeLoader.h>
#include <unity/scopes/CannedQuery.h>

#include <iostream>

class EXPORT MyScope : public unity::scopes::ScopeBase
{
public:
    MyScope() {}
    virtual ~MyScope() {}

    virtual int start(std::string const&, unity::scopes::RegistryProxy const&) override
    {
        inc_start();
        return VERSION;
    }

    virtual void stop() override
    {
        inc_stop();
    }

    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const&,
                                                             unity::scopes::SearchMetadata const&) override
    {
        return nullptr;
    }

    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const&,
                                                   unity::scopes::ActionMetadata const&) override
    {
        return nullptr;
    }
};

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        inc_create();
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        inc_destroy();
        delete scope_base;
    }

}
