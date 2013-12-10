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
        : root_node_( new JsonCppNode() )
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

    std::string value;
    JsonNodeInterface::SPtr node;

    // parse json

    EXPECT_TRUE( root_node_->read_json( json_string ) );
    EXPECT_EQ( 3, root_node_->size() );

    // get root value

    EXPECT_TRUE( root_node_->get_value( { "firstName" }, value ) );
    EXPECT_EQ( "John", value );

    EXPECT_TRUE( root_node_->get_value( { "age" }, value ) );
    EXPECT_EQ( "25", value );

    EXPECT_TRUE( root_node_->get_value( { "human" }, value ) );
    EXPECT_EQ( "true", value );

    // try get invalid value

    EXPECT_FALSE( root_node_->get_value( { "lastName" }, value ) );
    EXPECT_EQ( "", value );

    // add new root value

    EXPECT_TRUE( root_node_->set_value( { "lastName" }, "Smith" ) );
    EXPECT_EQ( 4, root_node_->size() );

    EXPECT_TRUE( root_node_->get_value( { "lastName" }, value ) );
    EXPECT_EQ( "Smith", value );
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

    EXPECT_TRUE( root_node_->read_json( json_string ) );
    EXPECT_EQ( 1, root_node_->size() );

    // get array values

    EXPECT_TRUE( root_node_->get_node( { "phoneNumbers" }, node ) );
    EXPECT_EQ( 2, node->size() );

    EXPECT_TRUE( node->get_value( 0, value ) );
    EXPECT_EQ( "1234", value );
    EXPECT_TRUE( node->get_value( 1, value ) );
    EXPECT_EQ( "5678", value );

    // get invalid array value

    EXPECT_FALSE( node->get_value( 2, value ) );
    EXPECT_EQ( "", value );

    // add new array value

    EXPECT_TRUE( node->set_value( 2, "9101112" ) );
    EXPECT_TRUE( root_node_->set_node( { "phoneNumbers" }, node ) );
    EXPECT_TRUE( root_node_->get_node( { "phoneNumbers" }, node ) );

    EXPECT_TRUE( node->get_value( 2, value ) );
    EXPECT_EQ( "9101112", value );
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

    EXPECT_TRUE( root_node_->read_json( json_string ) );
    EXPECT_EQ( 2, root_node_->size() );

    // get nested value

    EXPECT_TRUE( root_node_->get_value( { "address", "city" }, value ) );
    EXPECT_EQ( "New York", value );

    // get nested array values

    EXPECT_TRUE( root_node_->get_node( { "phoneNumbers" }, node ) );
    EXPECT_EQ( 2, node->size() );

    EXPECT_TRUE( node->get_node( 0, node ) );
    EXPECT_EQ( 2, node->size() );

    EXPECT_TRUE( node->get_value( { "type" }, value ) );
    EXPECT_EQ( "home", value );

    EXPECT_TRUE( node->get_value( { "number" }, value ) );
    EXPECT_EQ( "212 555-1234", value );
}
} // namespace
