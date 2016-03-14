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
 *              Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/JsonCppNode.h>

#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

void JsonCppNode::init_from_string(string const& json_string)
{
    if (json_string.empty())
    {
        return;
    }

    gobj_ptr<JsonParser> parser(json_parser_new());
    GError *err = nullptr;
    if (!json_parser_load_from_data(parser.get(), json_string.c_str(), json_string.size(), &err))
    {
        string msg = string("JsonCppNode(); parse error: ") + err->message;
        g_error_free(err);
        throw unity::ResourceException(msg);
    }
    root_.reset(json_node_copy(json_parser_get_root(parser.get())));
}

JsonCppNode::JsonCppNode()
    : root_(json_node_alloc(), json_node_free)
{
    json_node_init_null(root_.get());
}

JsonCppNode::JsonCppNode(string const& json_string)
    : JsonCppNode()
{
    init_from_string(json_string);
}

JsonCppNode::JsonCppNode(JsonNode* node)
    : root_(json_node_copy(node), json_node_free)
{
}

namespace
{

// Allocates a new node for each value. Caller is responsible for deallocating the root node.

JsonNode* variant_to_node(Variant const& var)
{
    unique_ptr<JsonNode, decltype(&json_node_free)> node_guard(json_node_alloc(), json_node_free);
    auto node = node_guard.get();
    switch (var.which())
    {
        case Variant::Type::Null:
            json_node_init_null(node);
            break;
        case Variant::Type::Array:
        {
            unique_ptr<JsonArray, decltype(&json_array_unref)> array(json_array_new(), json_array_unref);
            for (auto const& v : var.get_array())
            {
                json_array_add_element(array.get(), variant_to_node(v));
            }
            json_node_init_array(node, array.get());
            break;
        }
        case Variant::Type::Dict:
        {
            unique_ptr<JsonObject, decltype(&json_object_unref)> object(json_object_new(), json_object_unref);
            for (auto const& v : var.get_dict())
            {
                json_object_set_member(object.get(), v.first.c_str(), variant_to_node(v.second));
            }
            json_node_set_object(node, object.get());
            break;
        }
        case Variant::Type::String:
            json_node_init_string(node, var.get_string().c_str());
            break;
        case Variant::Type::Int:
            json_node_init_int(node, int64_t(var.get_int()));
            break;
        case Variant::Type::Int64:
            json_node_init_int(node, var.get_int64_t());
            break;
        case Variant::Type::Double:
            json_node_init_double(node, var.get_double());
            break;
        case Variant::Type::Bool:
        {
            json_node_init_boolean(node, var.get_bool());
            break;
        }
        default:
        {
            string msg = "JsonCppNode::from_variant(): unsupported variant type "
                         + to_string(static_cast<int>(var.which()));
            throw unity::LogicException(msg);
        }
    }
    return node_guard.release();
}

}  // namespace

JsonCppNode::JsonCppNode(const Variant& var)
    : root_(variant_to_node(var), json_node_free)
{
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace
{

Variant node_to_variant(JsonNode* node);  // Mutual recursion, need forward declaration.

extern "C"
void add_to_array(JsonArray* /* array */, guint /* index */, JsonNode* element_node, gpointer user_data)
{
    VariantArray* va = reinterpret_cast<VariantArray*>(user_data);
    va->push_back(node_to_variant(element_node));
}

extern "C"
void add_to_map(JsonObject* /* object */, gchar const* member_name, JsonNode* member_node, gpointer user_data)
{
    VariantMap* vm = reinterpret_cast<VariantMap*>(user_data);
    (*vm)[member_name] = node_to_variant(member_node);
}

Variant node_to_variant(JsonNode* node)
{
    switch (json_node_get_node_type(node))
    {
        case JSON_NODE_NULL:
            return Variant::null();
        case JSON_NODE_ARRAY:
        {
            VariantArray va;
            auto array = json_node_get_array(node);
            json_array_foreach_element(array, add_to_array, &va);
            return Variant(va);
        }
        case JSON_NODE_OBJECT:
        {
            VariantMap vm;
            auto object = json_node_get_object(node);
            json_object_foreach_member(object, add_to_map, &vm);
            return Variant(vm);
        }
        case JSON_NODE_VALUE:
        {
            switch (json_node_get_value_type(node))
            {
                case G_TYPE_STRING:
                    return Variant(string(json_node_get_string(node)));
                case G_TYPE_INT64:
                {
                    // HACK: If the value fits into a 32-bit int, return an Int
                    //       variant. Otherwise, return an Int64 variant.
                    int64_t val64 = json_node_get_int(node);
                    if (val64 < INT32_MIN || val64 > INT32_MAX)
                    {
                        return Variant(val64);
                    }
                    int32_t val32 = val64;
                    return Variant(val32);
                }
                case G_TYPE_DOUBLE:
                    return Variant(json_node_get_double(node));
                case G_TYPE_BOOLEAN:
                    return Variant(bool(json_node_get_boolean(node)));
                default:
                    abort();  // Impossible
            }
        }
        default:
            abort();  // Impossible
    }
}

}  // namespace

Variant JsonCppNode::to_variant() const
{
    return node_to_variant(root_.get());
}

void JsonCppNode::clear()
{
    json_node_init_null(root_.get());
}

void JsonCppNode::read_json(string const& json_string)
{
    init_from_string(json_string);
}

string JsonCppNode::to_json_string() const
{
    gobj_ptr<JsonGenerator> generator(json_generator_new());
    json_generator_set_root(generator.get(), root_.get());

    gsize len;
    unique_ptr<gchar, decltype(&g_free)> data(json_generator_to_data(generator.get(), &len), g_free);
    string result(data.get(), len);
    result += '\n';
    return result;
}

int JsonCppNode::size() const
{
    switch (json_node_get_node_type(root_.get()))
    {
        case JSON_NODE_ARRAY:
        {
            auto array = json_node_get_array(root_.get());
            return json_array_get_length(array);
        }
        case JSON_NODE_OBJECT:
        {
            auto object = json_node_get_object(root_.get());
            return json_object_get_size(object);
        }
        default:
            return 0;
    }
}

namespace
{

extern "C"
void add_name(JsonObject* /* object */, gchar const* member_name, JsonNode* /* member_node */, gpointer user_data)
{
    vector<string>* names = reinterpret_cast<vector<string>*>(user_data);
    names->push_back(member_name);
}

}

vector<string> JsonCppNode::member_names() const
{
    if (!JSON_NODE_HOLDS_OBJECT(root_.get()))
    {
        throw unity::LogicException("Root node is not an object");
    }

    vector<string> names;
    auto object = json_node_get_object(root_.get());
    json_object_foreach_member(object, add_name, &names);
    return names;
}

JsonNodeInterface::NodeType JsonCppNode::type() const
{
    switch (json_node_get_node_type(root_.get()))
    {
        case JSON_NODE_NULL:
            return Null;
        case JSON_NODE_ARRAY:
            return Array;
        case JSON_NODE_OBJECT:
            return Object;
        case JSON_NODE_VALUE:
        {
            switch (json_node_get_value_type(root_.get()))
            {
                case G_TYPE_STRING:
                    return String;
                case G_TYPE_INT64:
                    return Int;
                case G_TYPE_DOUBLE:
                    return Real;
                case G_TYPE_BOOLEAN:
                    return Bool;
                default:
                    abort();  // Impossible
            }
        }
        default:
            abort();  // Impossible
    }

    abort();  // NOTREACHED
}

string JsonCppNode::as_string() const
{
    if (!JSON_NODE_HOLDS_VALUE(root_.get()) || json_node_get_value_type(root_.get()) != G_TYPE_STRING)
    {
        throw unity::LogicException("Node does not contain a string value");
    }

    return string(json_node_get_string(root_.get()));
}

int64_t JsonCppNode::as_int() const
{
    if (!JSON_NODE_HOLDS_VALUE(root_.get()) || json_node_get_value_type(root_.get()) != G_TYPE_INT64)
    {
        throw unity::LogicException("Node does not contain an int value");
    }

    return json_node_get_int(root_.get());
}

double JsonCppNode::as_double() const
{
    if (!JSON_NODE_HOLDS_VALUE(root_.get()) || json_node_get_value_type(root_.get()) != G_TYPE_DOUBLE)
    {
        throw unity::LogicException("Node does not contain a double value");
    }

    return json_node_get_double(root_.get());
}

bool JsonCppNode::as_bool() const
{
    if (!JSON_NODE_HOLDS_VALUE(root_.get()) || json_node_get_value_type(root_.get()) != G_TYPE_BOOLEAN)
    {
        throw unity::LogicException("Node does not contain a bool value");
    }

    return json_node_get_boolean(root_.get());
}

#pragma GCC diagnostic pop

bool JsonCppNode::has_node(string const& node_name) const
{
    if (JSON_NODE_HOLDS_NULL(root_.get()))
    {
        throw unity::LogicException("Current node is empty");
    }

    if (!JSON_NODE_HOLDS_OBJECT(root_.get()))
    {
        throw unity::LogicException("Root node is not an object");
    }

    return json_object_has_member(json_node_get_object(root_.get()), node_name.c_str());
}

JsonNodeInterface::SPtr JsonCppNode::get_node() const
{
    if (JSON_NODE_HOLDS_NULL(root_.get()))
    {
        throw unity::LogicException("Current node is empty");
    }

    return make_shared<JsonCppNode>(root_.get());
}

JsonNodeInterface::SPtr JsonCppNode::get_node(string const& node_name) const
{
    if (!has_node(node_name))
    {
        throw unity::LogicException("Node " + node_name + " does not exist");
    }

    auto object = json_node_get_object(root_.get());
    auto member = json_object_get_member(object, node_name.c_str());
    return make_shared<JsonCppNode>(member);
}

JsonNodeInterface::SPtr JsonCppNode::get_node(unsigned int node_index) const
{
    if (JSON_NODE_HOLDS_NULL(root_.get()))
    {
        throw unity::LogicException("Current node is empty");
    }

    if (!JSON_NODE_HOLDS_ARRAY(root_.get()))
    {
        throw unity::LogicException("Root node is not an array");
    }

    auto array = json_node_get_array(root_.get());
    auto size = json_array_get_length(array);
    if (node_index >= size)
    {
        throw unity::LogicException("Invalid array index: " + to_string(node_index));
    }

    auto element = json_array_get_element(array, node_index);
    return make_shared<JsonCppNode>(element);
}
