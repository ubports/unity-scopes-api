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

#include <scopes/internal/smartscopes/JsonCppParser.h>

#include <gtest/gtest.h>
#include <memory>

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

class JsonParserTest : public Test
{
public:
  JsonParserTest() : json_parser_( new JsonCppParser() ) {}

protected:
  JsonParserInterface::SPtr json_parser_;
};

TEST_F( JsonParserTest, basic )
{
  std::string json_string = R"({
    "firstName": "John",
    "age": 25,
    "human": true,
    "address": {
        "city": "New York"
    },
    "phoneNumbers": [
        "1234",
        "5678"
    ]
  })";

  std::string value;
  std::vector< std::string > array;

  EXPECT_TRUE( json_parser_->read_json( json_string ) );

  EXPECT_TRUE( json_parser_->get_value( {"firstName"}, value ) );
  EXPECT_EQ( "John", value );

  EXPECT_FALSE( json_parser_->get_value( {"lastName"}, value ) );
  EXPECT_EQ( "", value );
  EXPECT_TRUE( json_parser_->set_value( {"lastName"}, "Smith" ) );
  EXPECT_TRUE( json_parser_->get_value( {"lastName"}, value ) );
  EXPECT_EQ( "Smith", value );

  EXPECT_TRUE( json_parser_->get_value( {"age"}, value ) );
  EXPECT_EQ( "25", value );

  EXPECT_TRUE( json_parser_->get_value( {"human"}, value ) );
  EXPECT_EQ( "true", value );

  EXPECT_TRUE( json_parser_->get_value( {"address","city"}, value ) );
  EXPECT_EQ( "New York", value );

  EXPECT_TRUE( json_parser_->get_array( {"phoneNumbers"}, array ) );
  EXPECT_EQ( "1234", array[0] );
  EXPECT_EQ( "5678", array[1] );

  array.push_back( "9101112" );
  EXPECT_TRUE( json_parser_->set_array( {"phoneNumbers"}, array ) );
  EXPECT_TRUE( json_parser_->get_array( {"phoneNumbers"}, array ) );
  EXPECT_EQ( "1234", array[0] );
  EXPECT_EQ( "5678", array[1] );
  EXPECT_EQ( "9101112", array[2] );
}

} // namespace
