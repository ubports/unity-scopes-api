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

#include <unity/scopes/internal/CategoryRendererImpl.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/util/FileIO.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal

{

//! @cond

CategoryRendererImpl::CategoryRendererImpl(std::string const& json_text)
    : data_(json_text)
{
    try
    {
        const internal::JsonCppNode node(json_text);
        if (node.type() != internal::JsonNodeInterface::NodeType::Object)
        {
            throw unity::InvalidArgumentException("CategoryRenderer(): invalid JSON definition, template is not a dictionary");
        }
    }
    catch (unity::ResourceException const&)
    {
        throw unity::InvalidArgumentException("CategoryRenderer(): invalid JSON definition");
    }
}

CategoryRenderer CategoryRendererImpl::from_file(std::string const& path)
{
    try
    {
        const std::string contents = unity::util::read_text_file(path);
        return CategoryRenderer(contents);
    }
    catch (...)
    {
        throw ResourceException("Category::from_file(): cannot parse renderer");
    }
}

std::string CategoryRendererImpl::data() const
{
    return data_;
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace unity
