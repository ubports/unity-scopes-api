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

#ifndef UNITY_API_SCOPES_CATEGORREGISTRYY_H
#define UNITY_API_SCOPES_CATEGORYREGISTRY_H

#include <unity/util/NonCopyable.h>
#include <scopes/Category.h>
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

class CategoryRegistry: private util::NonCopyable
{
public:
    CategoryRegistry() = default;

    Category::SCPtr add_category(std::string const &id, std::string const &renderer);
    Category::SCPtr find_category(std::string const& id) const;

private:
    std::map<std::string, Category::SCPtr> categories_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
