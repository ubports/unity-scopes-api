/*
 * Copyright (C) 2016 Canonical Ltd
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

#include <unity/scopes/FilterGroup.h>
#include <unity/scopes/internal/FilterGroupImpl.h>

namespace unity
{

namespace scopes
{

FilterGroup::FilterGroup(internal::FilterGroupImpl *pimpl)
    : p(pimpl)
{
}

FilterGroup::SCPtr FilterGroup::create(std::string const& id, std::string const& label)
{
    return std::shared_ptr<FilterGroup>(new FilterGroup(new internal::FilterGroupImpl(id, label)));
}

std::string FilterGroup::label() const
{
    return p->label();
}

std::string FilterGroup::id() const
{
    return p->id();
}

FilterGroup::FilterGroup() = default;
FilterGroup::~FilterGroup() = default;

} // namespace scopes

} // namespace unity
