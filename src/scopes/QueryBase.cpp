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

#include <unity/scopes/QueryBase.h>

#include <unity/scopes/internal/QueryBaseImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

/// @cond
QueryBase::QueryBase() :
    p(new internal::QueryBaseImpl)
{
}

QueryBase::~QueryBase()
{
}

void QueryBase::cancel()
{
    p->cancel();    // Forward cancel to subqueries
    cancelled();    // Inform this query that it was cancelled
}
/// @endcond

void QueryBase::set_metadata(SearchMetadata const& metadata)
{
    p->set_metadata(metadata);
}

void QueryBase::set_department_id(std::string const& department_id)
{
    p->set_department_id(department_id);
}

std::string QueryBase::department_id() const
{
    return p->department_id();
}

bool QueryBase::valid() const
{
    return p->valid();
}

VariantMap QueryBase::settings() const
{
    return p->settings();
}

} // namespace scopes

} // namespace unity
