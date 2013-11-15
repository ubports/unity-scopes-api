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
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#ifndef UNITY_API_SCOPES_CATEGORYIMPL_H
#define UNITY_API_SCOPES_CATEGORYIMPL_H

#include <unity/util/NonCopyable.h>
#include <scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class UNITY_API CategoryImpl : private util::NonCopyable
{
public:
    CategoryImpl(VariantMap const& variant_map);
    CategoryImpl(std::string const& id, std::string const& title, std::string const &icon, std::string const& renderer_template);
    std::string const& id() const;
    std::shared_ptr<VariantMap> variant_map() const;

private:
    std::string id_;
    std::string title_;
    std::string icon_;
    std::string renderer_template_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
