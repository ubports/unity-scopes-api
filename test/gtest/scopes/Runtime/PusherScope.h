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

#ifndef TEST_PUSHERSCOPE_H
#define TEST_PUSHERSCOPE_H

#include <unity/scopes/ScopeBase.h>

class PusherScope : public unity::scopes::ScopeBase
{
public:
    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const&,
                                                        unity::scopes::SearchMetadata const&) override;
    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const&,
                                                          unity::scopes::ActionMetadata const&) override;
};

#endif
