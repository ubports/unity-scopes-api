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

#include <unity/scopes/internal/JsonUtils.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

Variant json_to_variant(std::string const& json)
{
    auto node = std::make_shared<JsonCppNode>(json);
    return json_to_variant(node);
}

Variant json_to_variant(JsonNodeInterface::SCPtr node)
{
    switch (node->type())
    {
        case JsonNodeInterface::NodeType::Null:
            return Variant::null();
        case JsonNodeInterface::NodeType::Array:
            {
                VariantArray arr;
                for (auto i=0; i<node->size(); ++i)
                {
                    arr.push_back(json_to_variant(node->get_node(i)));
                }
                return Variant(arr);
            }
        case JsonNodeInterface::NodeType::Object:
            {
                VariantMap var;
                for (auto m: node->member_names())
                {
                    var[m] = json_to_variant(node->get_node(m));
                }
                return Variant(var);
            }
        case JsonNodeInterface::NodeType::String:
            return Variant(node->as_string());
        case JsonNodeInterface::NodeType::Int:
        case JsonNodeInterface::NodeType::UInt: //FIXME: potentially unsafe
            return Variant(node->as_int());
        case JsonNodeInterface::NodeType::Real:
            return Variant(node->as_double());
        case JsonNodeInterface::NodeType::Bool:
            return Variant(node->as_bool());
        default:
            throw unity::LogicException("json_to_variant(): unsupported json type " + node->type());
            break;
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
