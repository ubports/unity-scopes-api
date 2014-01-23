/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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
#include <unity/scopes/Variant.h>

#include <gtest/gtest.h>
#include <memory>
#include <algorithm>

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace
{

class JsonNodeTest : public Test
{
public:
    JsonNodeTest()
        : root_node_(new JsonCppNode())
    {
    }

protected:
    JsonNodeInterface::SPtr root_node_;
};

TEST_F(JsonNodeTest, flat_values)
{
    std::string json_string = R"({
                              "firstName": "John",
                              "age": 25,
                              "human": true
                          })";

    std::string value_str;
    int value_int;
    bool value_bool;

    // parse json

    EXPECT_NO_THROW(root_node_->read_json(json_string));
    EXPECT_EQ(3, root_node_->size());

    auto members = root_node_->member_names();
    EXPECT_EQ(3, members.size());
    EXPECT_TRUE(std::find(members.begin(), members.end(), "firstName") != members.end());
    EXPECT_TRUE(std::find(members.begin(), members.end(), "age") != members.end());
    EXPECT_TRUE(std::find(members.begin(), members.end(), "human") != members.end());

    // get root value

    EXPECT_NO_THROW(value_str = root_node_->get_node("firstName")->as_string());
    EXPECT_EQ("John", value_str);

    EXPECT_NO_THROW(value_int = root_node_->get_node("age")->as_int());
    EXPECT_EQ(25, value_int);

    EXPECT_NO_THROW(value_bool = root_node_->get_node("human")->as_bool());
    EXPECT_EQ(true, value_bool);

    // try get invalid value

    EXPECT_FALSE(root_node_->has_node("lastName"));
    EXPECT_THROW(root_node_->get_node("lastName"), unity::Exception);
}

TEST_F(JsonNodeTest, array_values)
{
    std::string json_string = R"({
                              "phoneNumbers": [
                              "1234",
                              "5678"
                              ]
                          })";

    std::string value;
    JsonNodeInterface::SPtr node;

    // parse json

    EXPECT_NO_THROW(root_node_->read_json(json_string));
    EXPECT_EQ(1, root_node_->size());

    // get array values

    EXPECT_NO_THROW(node = root_node_->get_node("phoneNumbers"));
    EXPECT_EQ(2, node->size());

    EXPECT_NO_THROW(value = node->get_node(0)->as_string());
    EXPECT_EQ("1234", value);
    EXPECT_NO_THROW(value = node->get_node(1)->as_string());
    EXPECT_EQ("5678", value);

    // get invalid array value

    EXPECT_THROW(node->get_node(2), unity::Exception);
}

TEST_F(JsonNodeTest, nested_values)
{
    std::string json_string = R"({
                              "address": {
                              "city": "New York"
                          },
                              "phoneNumbers": [
                              {
                              "type": "home",
                              "number": "212 555-1234"
                          },
                              {
                              "type": "fax",
                              "number": "646 555-4567"
                          }
                              ]
                          })";

    std::string value;
    JsonNodeInterface::SPtr node;

    // parse json

    EXPECT_NO_THROW(root_node_->read_json(json_string));
    EXPECT_EQ(2, root_node_->size());

    // get nested value

    EXPECT_NO_THROW(node = root_node_->get_node("address"));
    EXPECT_EQ(1, node->size());

    EXPECT_NO_THROW(value = node->get_node("city")->as_string());
    EXPECT_EQ("New York", value);

    // get nested array values

    EXPECT_NO_THROW(node = root_node_->get_node("phoneNumbers"));
    EXPECT_EQ(2, node->size());

    EXPECT_NO_THROW(node = node->get_node(0));
    EXPECT_EQ(2, node->size());

    EXPECT_NO_THROW(value = node->get_node("type")->as_string());
    EXPECT_EQ("home", value);

    EXPECT_NO_THROW(value = node->get_node("number")->as_string());
    EXPECT_EQ("212 555-1234", value);
}

TEST_F(JsonNodeTest, from_variant)
{
    VariantArray va({Variant(1), Variant(2), Variant(true)});
    VariantMap vm;
    vm["foo"] = "bar";
    vm["baz"] = 1;
    vm["boo"] = 2.0f;
    vm["zee"] = true;
    vm["wee"] = Variant(va);

    Variant var(vm);
    JsonCppNode node(var);
    EXPECT_EQ("bar", node.get_node("foo")->as_string());
    EXPECT_EQ(1, node.get_node("baz")->as_int());
    EXPECT_TRUE(node.get_node("boo")->as_double() - 2.0f < 0.00001f);
    EXPECT_TRUE(node.get_node("zee")->as_bool());
    EXPECT_EQ(1, node.get_node("wee")->get_node(0)->as_int());
    EXPECT_EQ(2, node.get_node("wee")->get_node(1)->as_int());
    EXPECT_EQ(true, node.get_node("wee")->get_node(2)->as_bool());
}

TEST_F(JsonNodeTest, to_variant)
{
    std::string json_string = R"({"a":1, "b": 2.0, "c":null, "d":[1,true,"foo"]})";
    JsonCppNode node(json_string);
    auto var = node.to_variant();
    auto outer = var.get_dict();
    EXPECT_EQ(4, outer.size());
    EXPECT_EQ(1, outer["a"].get_int());
    EXPECT_TRUE(outer["b"].get_double() - 2.0f < 0.00001f);
    EXPECT_TRUE(outer["c"].is_null());
    auto arr = outer["d"].get_array();
    EXPECT_EQ(3, arr.size());
    EXPECT_EQ(1, arr[0].get_int());
    EXPECT_TRUE(arr[1].get_bool());
    EXPECT_EQ("foo", arr[2].get_string());
}

} // namespace
