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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/internal/ActivationQueryBaseImpl.h>

namespace unity
{

namespace scopes
{

/// @cond
ActivationQueryBase::ActivationQueryBase() :
    QueryBase(),
    p(new internal::ActivationQueryBaseImpl())
{
}

ActivationQueryBase::~ActivationQueryBase()
{
}
/// @endcond

void ActivationQueryBase::cancelled()
{
    //default implementation does nothing
}

ActivationResponse ActivationQueryBase::activate()
{
    return p->activate();
}

} // namespace scopes

} // namespace unity
