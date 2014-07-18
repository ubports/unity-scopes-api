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

#ifndef TEST_TESTSCOPE_H
#define TEST_TESTSCOPE_H

#include <unity/scopes/ScopeBase.h>

using namespace std;
using namespace unity::scopes;

class TestScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override;
    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override;
};

#endif
