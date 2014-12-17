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

    ChildScopeList child_scopes() const override
    {
        if (i == 0)
        {
            ChildScopeList list;
            list.push_back({"ScopeA", true});
            list.push_back({"ScopeB", false});
            list.push_back({"ScopeC", false});
            ++i;
            return list;
        }
        else
        {
            ChildScopeList list;
            list.push_back({"ScopeA", true});
            list.push_back({"ScopeB", false});
            list.push_back({"ScopeC", false});
            list.push_back({"ScopeD", false});
            list.push_back({"ScopeE", true});
            return list;
        }
    }

private:
    mutable int i = 0;
};

} // namespace scopes

} // namespace unity
