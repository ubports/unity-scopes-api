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

#include <unity/scopes/FilterBase.h>
#include <unity/scopes/internal/FilterBaseImpl.h>

namespace unity
{

namespace scopes
{

/// @cond
FilterBase::FilterBase(internal::FilterBaseImpl *pimpl)
    : p(pimpl)
{
}

VariantMap FilterBase::serialize() const
{
    return p->serialize();
}

FilterBase::~FilterBase() = default;
/// @endcond

void FilterBase::set_display_hints(int hints)
{
    p->set_display_hints(hints);
}

std::string FilterBase::id() const
{
    return p->id();
}

int FilterBase::display_hints() const
{
    return p->display_hints();
}

std::string FilterBase::filter_type() const
{
    return p->filter_type();
}

} // namespace scopes

} // namespace unity
