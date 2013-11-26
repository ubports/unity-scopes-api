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

#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRendererImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

const std::string CategoryRenderer::DEFAULT = "{\"template\":{\"renderer\":\"grid\",\"overlay-mode\":null},"
    "\"components\":{\"title\":\"title\",\"art\": \"art\" }}";

const std::string CategoryRenderer::MUSIC_GRID = "{\"template\":{\"renderer\":\"grid\",\"overlay-mode\":null},"
    "\"components\":{\"title\":\"title\",\"subtitle\":\"subtitle\",\"art\":\"art\"}}";

CategoryRenderer::CategoryRenderer(std::string const& json_text)
    : p(new internal::CategoryRendererImpl(json_text))
{
}

CategoryRenderer CategoryRenderer::from_file(std::string const& path)
{
    return internal::CategoryRendererImpl::from_file(path);
}

std::string CategoryRenderer::data() const
{
    return p->data();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity
