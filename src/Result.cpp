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

#include <scopes/Result.h>
#include <scopes/internal/ResultImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Result::Result(internal::ResultImpl* impl)
    : p(impl)
{
}

Result::Result(std::shared_ptr<internal::ResultImpl> impl)
    : p(impl)
{
}


Result::Result(const VariantMap &variant_map)
    : p(new internal::ResultImpl(variant_map))
{
}

Result::Result(Result const& other)
    : p(new internal::ResultImpl(*(other.p)))
{
}

Result::~Result()
{
}

Result& Result::operator=(Result const& other)
{
    if (this != &other)
    {
        p = std::make_shared<internal::ResultImpl>(*(other.p));
    }
    return *this;
}

Result::Result(Result&&) = default;

Result& Result::operator=(Result&&) = default;

void Result::store(Result const& other, bool intercept_preview_req)
{
    p->store(other, intercept_preview_req);
}

bool Result::has_stored_result() const
{
    return p->has_stored_result();
}

Result Result::retrieve() const
{
    return p->retrieve();
}

void Result::set_uri(std::string const& uri)
{
    p->set_uri(uri);
}

void Result::set_title(std::string const& title)
{
    p->set_title(title);
}

void Result::set_art(std::string const& image)
{
    p->set_art(image);
}

void Result::set_dnd_uri(std::string const& dnd_uri)
{
    p->set_dnd_uri(dnd_uri);
}

void Result::set_intercept_activation()
{
    p->set_intercept_activation();
}

bool Result::direct_activation() const
{
    return p->direct_activation();
}

std::string Result::activation_scope_name() const
{
    return p->activation_scope_name();
}

Variant& Result::operator[](std::string const& key)
{
    return (*p)[key];
}

Variant const& Result::operator[](std::string const& key) const
{
    // const_pointer_cast to force const [] operator of ResultImpl
    return (*(std::const_pointer_cast<internal::ResultImpl const>(p)))[key];
}

std::string Result::uri() const noexcept
{
    return p->uri();
}

std::string Result::title() const noexcept
{
    return p->title();
}

std::string Result::art() const noexcept
{
    return p->art();
}

std::string Result::dnd_uri() const noexcept
{
    return p->dnd_uri();
}

bool Result::contains(std::string const& key) const
{
    return p->contains(key);
}

Variant const& Result::value(std::string const& key) const
{
    return p->value(key);
}

VariantMap Result::serialize() const
{
    return p->serialize();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity
