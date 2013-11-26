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

#include <scopes/internal/CategoryRendererImpl.h>
#include <scopes/CategoryRenderer.h>
#include <fstream>
#include <streambuf>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal

{

//! @cond

CategoryRendererImpl::CategoryRendererImpl(std::string const& json_text)
    : data_(json_text)
{
    //TODO: json validation
}

CategoryRenderer CategoryRendererImpl::from_file(std::string const& path)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return CategoryRenderer(contents);
}

std::string CategoryRendererImpl::data() const
{
    return data_;
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
