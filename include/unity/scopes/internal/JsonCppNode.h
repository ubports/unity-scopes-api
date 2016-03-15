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

#pragma once

#include <unity/scopes/internal/JsonNodeInterface.h>

#include <unity/scopes/internal/gobj_memory.h>
#include <unity/scopes/Variant.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <json-glib/json-glib.h>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

namespace internal
{

class JsonCppNode : public JsonNodeInterface
{
public:
    JsonCppNode();
    JsonCppNode(std::string const& json_string);
    explicit JsonCppNode(JsonNode* root);
    explicit JsonCppNode(Variant const& var);
    ~JsonCppNode() = default;

    void clear() override;
    void read_json(std::string const& json_string) override;
    std::string to_json_string() const override;
    Variant to_variant() const override;

    int size() const override;
    std::vector<std::string> member_names() const override;
    NodeType type() const override;

    std::string as_string() const override;
    int64_t as_int() const override;
    double as_double() const override;
    bool as_bool() const override;

    bool has_node(std::string const& node_name) const override;

    JsonNodeInterface::SPtr get_node() const override;
    JsonNodeInterface::SPtr get_node(std::string const& node_name) const override;
    JsonNodeInterface::SPtr get_node(unsigned int node_index) const override;

private:
    void init_from_string(std::string const& jscon_string);

    typedef std::unique_ptr<JsonNode, decltype(&json_node_free)> NodePtr;
    NodePtr root_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
