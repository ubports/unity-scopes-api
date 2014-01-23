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

#ifndef UNITY_SCOPES_INTERNAL_RESULTREPLYOBJECT_H
#define UNITY_SCOPES_INTERNAL_RESULTREPLYOBJECT_H

#include <unity/scopes/internal/ReplyObject.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/SearchListener.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

class ResultReplyObject : public ReplyObject
{
public:
    ResultReplyObject(SearchListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name);
    virtual ~ResultReplyObject();

    virtual void process_data(VariantMap const& data) override;

private:
    SearchListener::SPtr const receiver_;
    std::shared_ptr<CategoryRegistry> cat_registry_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
