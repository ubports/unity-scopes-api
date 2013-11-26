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

#ifndef UNITY_API_SCOPES_CATEGORYRENDERER_H
#define UNITY_API_SCOPES_CATEGORYRENDERER_H

#include <string>
#include <memory>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    class CategoryRendererImpl;
}

/**
   \brief
*/
class UNITY_API CategoryRenderer
{
public:
    explicit CategoryRenderer(std::string const& json_text = DEFAULT);
    static CategoryRenderer from_file(std::string const& path);

    std::string data() const;

    static const std::string DEFAULT;
    static const std::string MUSIC_GRID;

private:
    std::shared_ptr<internal::CategoryRendererImpl> p;

    friend class internal::CategoryRendererImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
