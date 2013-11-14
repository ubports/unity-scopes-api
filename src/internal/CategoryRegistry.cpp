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

#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

Category::SCPtr CategoryRegistry::add_category(std::string const &id, std::string const &renderer)
{
    if (categories_.find(id) != categories_.end())
    {
        std::ostringstream s;
        s << "Category " << id << " already defined";
        throw InvalidArgumentException(s.str());
    }
    auto cat = std::make_shared<Category>(id, renderer);

}

Category::SCPtr CategoryRegistry::find_category(std::string const& id) const
{
    auto it = categories_.find(id);
    if (it != categories_.end())
    {
        return it->second;
    }
    return nullptr;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity


