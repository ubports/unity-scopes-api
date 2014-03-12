/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
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

#include <unity/scopes/ScopeMetadata.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

//! @cond

ScopeMetadata::ScopeMetadata(unique_ptr<internal::ScopeMetadataImpl> impl) :
    p(move(impl))
{
    // The only way to create a ScopeMetadata instance is to first intialize
    // a ScopeMetadataImpl, fill it in, and then call this constructor.
    // We check here that all mandatory fields are present. If not
    // we have an internal logic error.
    assert(!p->scope_id().empty());
    assert(p->proxy());
    assert(!p->display_name().empty());
    assert(!p->description().empty());
}

ScopeMetadata::ScopeMetadata(ScopeMetadata const& other) :
    p(new internal::ScopeMetadataImpl(*(other.p)))
{
}

ScopeMetadata::ScopeMetadata(ScopeMetadata&&) = default;

ScopeMetadata::~ScopeMetadata()
{
}

ScopeMetadata& ScopeMetadata::operator=(ScopeMetadata const& other)
{
    if (this != &other)
    {
        p.reset(new internal::ScopeMetadataImpl(*(other.p)));
    }
    return *this;
}

ScopeMetadata& ScopeMetadata::operator=(ScopeMetadata&&) = default;

string ScopeMetadata::scope_id() const
{
    return p->scope_id();
}

ScopeProxy ScopeMetadata::proxy() const
{
    return p->proxy();
}

string ScopeMetadata::display_name() const
{
    return p->display_name();
}

string ScopeMetadata::description() const
{
    return p->description();
}

string ScopeMetadata::author() const
{
    return p->author();
}

string ScopeMetadata::art() const
{
    return p->art();
}

string ScopeMetadata::icon() const
{
    return p->icon();
}

string ScopeMetadata::search_hint() const
{
    return p->search_hint();
}

string ScopeMetadata::hot_key() const
{
    return p->hot_key();
}

bool ScopeMetadata::invisible() const
{
    return p->invisible();
}

VariantMap ScopeMetadata::serialize() const
{
    return p->serialize();
}

//! @endcond

} // namespace scopes

} // namespace unity
