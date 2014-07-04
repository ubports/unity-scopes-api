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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/ActionMetadata.h>

#include <unity/scopes/internal/ActionMetadataImpl.h>

namespace unity
{

namespace scopes
{

ActionMetadata::ActionMetadata(std::string const& locale, std::string const& form_factor)
    : p(new internal::ActionMetadataImpl(locale, form_factor))
{
}

/// @cond
ActionMetadata::ActionMetadata(internal::ActionMetadataImpl *impl)
    : p(impl)
{
}

ActionMetadata::ActionMetadata(ActionMetadata const& other)
    : p(new internal::ActionMetadataImpl(*(other.p.get())))
{
}

ActionMetadata::ActionMetadata(ActionMetadata&&) = default;

ActionMetadata::~ActionMetadata()
{
}

ActionMetadata& ActionMetadata::operator=(ActionMetadata const& other)
{
    if (this != &other)
    {
        p.reset(new internal::ActionMetadataImpl(*(other.p.get())));
    }
    return *this;
}

ActionMetadata& ActionMetadata::operator=(ActionMetadata&&) = default;


VariantMap ActionMetadata::serialize() const
{
    return p->serialize();
}

/// @endcond

std::string ActionMetadata::locale() const
{
    return p->locale();
}

std::string ActionMetadata::form_factor() const
{
    return p->form_factor();
}

void ActionMetadata::set_scope_data(Variant const& data)
{
    p->set_scope_data(data);
}

Variant ActionMetadata::scope_data() const
{
    return p->scope_data();
}

} // namespace scopes

} // namespace unity
