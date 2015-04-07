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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#pragma once

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/testing/ScopeMetadataBuilder.h>
#include <unity/scopes/testing/MockScope.h>

namespace unity
{

namespace scopes
{

class TestScope : public ScopeBase
{
public:
    SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override
    {
        return nullptr;
    }

    PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;
    }

    ChildScopeList find_child_scopes() const override
    {
        // 1st TestScope::find_child_scopes() returns: "A,B,C"
        if (i == 0)
        {
            ChildScopeList list;
            list.push_back({"ScopeA", empty_metadata()});
            list.push_back({"ScopeB", empty_metadata()});
            list.push_back({"ScopeC", empty_metadata()});
            ++i;
            return list;
        }
        // 2nd TestScope::find_child_scopes() returns: "D,A,B,C,E"
        else if (i == 1)
        {
            ChildScopeList list;
            list.push_back({"ScopeD", empty_metadata()});
            list.push_back({"ScopeA", empty_metadata()});
            list.push_back({"ScopeB", empty_metadata()});
            list.push_back({"ScopeC", empty_metadata()});
            list.push_back({"ScopeE", empty_metadata()});
            ++i;
            return list;
        }
        // 3rd+ TestScope::find_child_scopes() returns: D,A,B
        else
        {
            ChildScopeList list;
            list.push_back({"ScopeD", empty_metadata()});
            list.push_back({"ScopeA", empty_metadata()});
            list.push_back({"ScopeB", empty_metadata()});
            return list;
        }
    }

    ScopeMetadata empty_metadata() const
    {
        unity::scopes::testing::ScopeMetadataBuilder builder;
        builder.scope_id(" ").proxy(std::make_shared<unity::scopes::testing::MockScope>(" ", " "))
               .display_name(" ").description(" ").author(" ");
        return builder();
    }

private:
    mutable int i = 0;
};

} // namespace scopes

} // namespace unity
