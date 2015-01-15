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

#pragma once

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ReplyObjectBase : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ReplyObjectBase);

    virtual void push(VariantMap const& result) noexcept = 0;
    virtual void finished(CompletionDetails const& details) noexcept = 0;
    virtual void info(OperationInfo const& op_info) noexcept = 0;
};

} // namespace internal

} // namespace scopes

} // namespace unity
