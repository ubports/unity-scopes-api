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

#include <scopes/internal/smartscopes/JsonCppParser.h>
#include <sstream>

using namespace unity::api::scopes::internal::smartscopes;

JsonCppParser::JsonCppParser( std::string json_string )
{
  read_json( json_string );
}

JsonCppParser::~JsonCppParser()
{

}

void JsonCppParser::clear_json()
{
  root_.clear();
}

bool JsonCppParser::read_json( const std::string& json_string )
{
  clear_json();

  if( !reader_.parse( json_string, root_ ) )
  {
    std::cout << "Failed to parse configuration\n" << reader_.getFormattedErrorMessages();
    return false;
  }

  return true;
}

bool JsonCppParser::write_json( std::string& json_string )
{
  if( root_.empty() )
  {
    return false;
  }

  json_string = writer_.write( root_ );
  return true;
}

bool JsonCppParser::get_value( const std::vector< std::string >& value_path, std::string& value )
{
  Json::Value value_node = get_value_path( value_path );

  if( !value_node || value_node.isArray() || value_node.isObject() )
  {
    return false;
  }

  std::string value_str = writer_.write( value_node );
  value = value_str.substr( 0, value_str.size() - 1 );
  return true;
}

bool JsonCppParser::get_array( const std::vector< std::string >& array_path, std::vector< std::string >& array )
{
  Json::Value array_node = get_value_path( array_path );

  if( !array_node || !array_node.isArray() )
  {
    return false;
  }

  for( const Json::Value& value_node : array_node )
  {
    std::string value_str = writer_.write( value_node );
    array.push_back( value_str.substr( 0, value_str.size() - 1 ) );
  }

  return true;
}

bool JsonCppParser::set_value( const std::vector< std::string >& value_path, const std::string& value )
{
  Json::Value value_node = set_value_path( value_path );

  value_node = value;
  return true;
}

bool JsonCppParser::set_array( const std::vector< std::string >& array_path, const std::vector< std::string >& array )
{
  Json::Value array_node = set_value_path( array_path );

  array_node.clear();
  for( const std::string& value : array )
  {
    array_node.append( value );
  }

  return true;
}

Json::Value JsonCppParser::get_value_path( const std::vector< std::string >& value_path )
{
  Json::Value value_node;
  for( const std::string& node : value_path )
  {
    value_node = value_node.get( node, nullptr );
    if( !value_node )
    {
      break;
    }
  }

  return value_node;
}

Json::Value JsonCppParser::set_value_path( const std::vector< std::string >& value_path )
{
  Json::Value value_node;
  for( const std::string& node : value_path )
  {
    value_node = value_node[node];
  }

  return value_node;
}
