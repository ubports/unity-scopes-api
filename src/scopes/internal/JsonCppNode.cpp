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

#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/UnityExceptions.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include <sstream>

using namespace unity::scopes;
using namespace unity::scopes::internal;

JsonCppNode::JsonCppNode(std::string const& json_string)
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

JsonCppNode::JsonCppNode(const Variant& var)
{
    root_ = from_variant(var);
}

JsonCppNode::~JsonCppNode()
{
}

Json::Value JsonCppNode::from_variant(Variant const& var)
{
    switch (var.which())
    {
        case Variant::Type::Int:
            return Json::Value(var.get_int());
        case Variant::Type::Bool:
            return Json::Value(var.get_bool());
        case Variant::Type::String:
            return Json::Value(var.get_string());
        case Variant::Type::Double:
            return Json::Value(var.get_double());
        case Variant::Type::Dict:
            {
                Json::Value val(Json::ValueType::objectValue);
                for (auto const& v: var.get_dict())
                {
                    val[v.first] = from_variant(v.second);
                }
                return val;
            }
        case Variant::Type::Array:
            {
                Json::Value val(Json::ValueType::arrayValue);
                for (auto const& v: var.get_array())
                {
                    val.append(from_variant(v));
                }
                return val;
            }
        case Variant::Type::Null:
            return Json::Value(Json::ValueType::nullValue);
        default:
            {
                std::ostringstream s;
                s << "json_to_variant(): unsupported json type ";
                s << static_cast<int>(var.which());
                throw unity::LogicException(s.str());
            }
    }
}

Variant JsonCppNode::to_variant(Json::Value const& value)
{
    switch (value.type())
    {
        case Json::ValueType::nullValue:
            return Variant::null();
        case Json::ValueType::arrayValue:
            {
                VariantArray arr;
                for (unsigned int i=0; i<value.size(); ++i)
                {
                    arr.push_back(to_variant(value[i]));
                }
                return Variant(arr);
            }
        case Json::ValueType::objectValue:
            {
                VariantMap var;
                for (auto const& m: value.getMemberNames())
                {
                    var[m] = to_variant(value[m]);
                }
                return Variant(var);
            }
        case Json::ValueType::stringValue:
            return Variant(value.asString());
        case Json::ValueType::intValue:
        case Json::ValueType::uintValue:
            return Variant(value.asInt()); // this can throw std::runtime_error from jsoncpp if unsigned int to int conversion is not possible
        case Json::ValueType::realValue:
            return Variant(value.asDouble());
        case Json::ValueType::booleanValue:
            return Variant(value.asBool());
        default:
            {
                std::ostringstream s;
                s << "json_to_variant(): unsupported json type ";
                s << static_cast<int>(value.type());
                throw unity::LogicException(s.str());
            }
    }
}

Variant JsonCppNode::to_variant() const
{
    return to_variant(root_);
}

void JsonCppNode::clear()
{
    root_.clear();
}

void JsonCppNode::read_json(std::string const& json_string)
{
    Json::Reader reader;
    clear();

    if (!reader.parse(json_string, root_))
    {
        throw unity::ResourceException("Failed to parse json string: " + reader.getFormattedErrorMessages());
    }
}

std::string JsonCppNode::to_json_string() const
{
    Json::FastWriter writer;
    return writer.write(root_);
}

int JsonCppNode::size() const
{
    return root_.size();
}

std::vector<std::string> JsonCppNode::member_names() const
{
    if (root_.type() != Json::objectValue)
    {
        throw unity::LogicException("Root node is not an object");
    }
    return root_.getMemberNames();
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
        default:
            break;
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

unsigned int JsonCppNode::as_uint() const
{
    if (!root_.isConvertibleTo(Json::uintValue))
    {
        throw unity::LogicException("Node does not contain a unsigned int value");
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

bool JsonCppNode::has_node(std::string const& node_name) const
{
    if (!root_)
    {
        throw unity::LogicException("Current node is empty");
    }

    return root_.isMember(node_name);
}

JsonNodeInterface::SPtr JsonCppNode::get_node() const
{
    if (!root_)
    {
        throw unity::LogicException("Current node is empty");
    }

    return std::make_shared<JsonCppNode>(root_);
}

JsonNodeInterface::SPtr JsonCppNode::get_node(std::string const& node_name) const
{
    if (!root_.isMember(node_name))
    {
        throw unity::LogicException("Node " + node_name + " does not exist");
    }

    const Json::Value& value_node = root_[node_name];
    return std::make_shared<JsonCppNode>(value_node);
}

JsonNodeInterface::SPtr JsonCppNode::get_node(unsigned int node_index) const
{
    if (root_.type() != Json::arrayValue)
    {
        throw unity::LogicException("Root node is not an array");
    }
    else if (node_index >= root_.size())
    {
        throw unity::LogicException("Invalid array index");
    }

    const Json::Value& value_node = root_[node_index];

    if (!value_node)
    {
        throw unity::LogicException("Node " + std::to_string(node_index) + " does not exist");
    }

    return std::make_shared <JsonCppNode> (value_node);
}
