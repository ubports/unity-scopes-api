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

#include <scopes/internal/smartscopes/JsonCppNode.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <memory>

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

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

TEST_F( JsonNodeTest, flat_values )
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

TEST_F( JsonNodeTest, array_values )
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

    EXPECT_FALSE(node->has_node(2));
    EXPECT_THROW(node->get_node(2), unity::Exception);
}

TEST_F( JsonNodeTest, nested_values )
{
    std::string json_string =
        R"({
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
} // namespace
