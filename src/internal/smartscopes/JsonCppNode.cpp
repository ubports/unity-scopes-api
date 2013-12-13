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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <scopes/internal/smartscopes/JsonCppNode.h>
#include <unity/UnityExceptions.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal::smartscopes;

JsonCppNode::JsonCppNode(const std::string& json_string)
{
    if (!json_string.empty())
    {
        read_json(json_string);
    }
}

JsonCppNode::JsonCppNode(const Json::Value& root)
    : root_(root)
{
}

JsonCppNode::~JsonCppNode()
{

}

void JsonCppNode::clear()
{
    root_.clear();
}

void JsonCppNode::read_json(const std::string& json_string)
{
    clear();

    if (!reader_.parse(json_string, root_))
    {
        throw unity::ResourceException("Failed to parse json string: " + reader_.getFormattedErrorMessages());
    }
}

int JsonCppNode::size() const
{
    return root_.size();
}

JsonNodeInterface::NodeType JsonCppNode::type() const
{
    switch (root_.type())
    {
        case Json::nullValue:
            return Null;
        case Json::arrayValue:
            return Array;
        case Json::objectValue:
            return Object;
        case Json::stringValue:
            return String;
        case Json::intValue:
            return Int;
        case Json::uintValue:
            return UInt;
        case Json::realValue:
            return Real;
        case Json::booleanValue:
            return Bool;
    }

    return Null;
}

std::string JsonCppNode::as_string() const
{
    if (!root_.isConvertibleTo(Json::stringValue))
    {
        throw unity::LogicException("Node does not contain a string value");
    }

    return root_.asString();
}

int JsonCppNode::as_int() const
{
    if (!root_.isConvertibleTo(Json::intValue))
    {
        throw unity::LogicException("Node does not contain an int value");
    }

    return root_.asInt();
}

uint JsonCppNode::as_uint() const
{
    if (!root_.isConvertibleTo(Json::uintValue))
    {
        throw unity::LogicException("Node does not contain a uint value");
    }

    return root_.asUInt();
}

double JsonCppNode::as_double() const
{
    if (!root_.isConvertibleTo(Json::realValue))
    {
        throw unity::LogicException("Node does not contain a double value");
    }

    return root_.asDouble();
}

bool JsonCppNode::as_bool() const
{
    if (!root_.isConvertibleTo(Json::booleanValue))
    {
        throw unity::LogicException("Node does not contain a bool value");
    }

    return root_.asBool();
}

bool JsonCppNode::has_node(const std::string& node_name) const
{
    return root_.isMember(node_name);
}

bool JsonCppNode::has_node(uint node_index) const
{
    if (root_.type() != Json::arrayValue || node_index >= root_.size())
    {
        return false;
    }

    return true;
}

JsonNodeInterface::SPtr JsonCppNode::get_node(const std::string& node_name) const
{
    const Json::Value& value_node = root_[node_name];

    if (!value_node)
    {
        throw unity::LogicException("Node " + node_name + " does not exist");
    }

    return std::make_shared <JsonCppNode> (value_node);
}

JsonNodeInterface::SPtr JsonCppNode::get_node(uint node_index) const
{
    if (root_.type() != Json::arrayValue)
    {
        throw unity::LogicException("Root node is not an array");
    }

    const Json::Value& value_node = root_[node_index];

    if (!value_node)
    {
        throw unity::LogicException("Node " + std::to_string(node_index) + " does not exist");
    }

    return std::make_shared <JsonCppNode> (value_node);
}
