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

#include <unity/scopes/PreviewQueryBase.h>

#include <unity/scopes/internal/PreviewQueryBaseImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

/// @cond
PreviewQueryBase::PreviewQueryBase(Result const& result, ActionMetadata const& metadata) :
    QueryBase(new internal::PreviewQueryBaseImpl(result, metadata))
{
}

PreviewQueryBase::~PreviewQueryBase()
{
}
/// @endcond

Result PreviewQueryBase::result() const
{
    return fwd()->result();
}

ActionMetadata PreviewQueryBase::action_metadata() const
{
    return fwd()->action_metadata();
}

internal::PreviewQueryBaseImpl* PreviewQueryBase::fwd() const
{
    return dynamic_cast<internal::PreviewQueryBaseImpl*>(p.get());
}

} // namespace scopes

} // namespace unity
