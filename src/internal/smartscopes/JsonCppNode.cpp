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
#include <sstream>

using namespace unity::api::scopes::internal::smartscopes;

JsonCppNode::JsonCppNode( std::string json_string )
{
  if( !json_string.empty() )
  {
    read_json( json_string );
  }
}

JsonCppNode::JsonCppNode( const Json::Value& root )
    : root_( root )
{
}

JsonCppNode::~JsonCppNode()
{

}

void JsonCppNode::clear_json()
{
  root_.clear();
}

bool JsonCppNode::read_json( const std::string& json_string )
{
  clear_json();

  if( !reader_.parse( json_string, root_ ) )
  {
    std::cout << "Failed to parse configuration\n" << reader_.getFormattedErrorMessages();
    return false;
  }

  return true;
}

bool JsonCppNode::write_json( std::string& json_string )
{
  if( root_.empty() )
  {
    return false;
  }

  json_string = writer_.write( root_ );
  return true;
}

int JsonCppNode::size()
{
  return root_.size();
}

bool JsonCppNode::get_value( const std::vector< std::string >& value_path, std::string& value )
{
  const Json::Value& value_node = get_value_path( value_path );

  if( !value_node || value_node.isArray() || value_node.isObject() )
  {
    value = "";
    return false;
  }

  value = node_to_string( value_node );
  return true;
}

bool JsonCppNode::get_value( int value_index, std::string& value )
{
  if( root_.type() != Json::arrayValue )
  {
    return false;
  }

  const Json::Value& value_node = root_[value_index];

  if( !value_node || value_node.isArray() || value_node.isObject() )
  {
    value = "";
    return false;
  }

  value = node_to_string( value_node );
  return true;
}

bool JsonCppNode::get_node( const std::vector< std::string >& node_path, JsonNodeInterface::SPtr& node )
{
  const Json::Value& value_node = get_value_path( node_path );

  if( !value_node || ( !value_node.isArray() && !value_node.isObject() ) )
  {
    return false;
  }

  node = std::make_shared < JsonCppNode > ( value_node );
  return true;
}

bool JsonCppNode::get_node( int node_index, JsonNodeInterface::SPtr& node )
{
  if( root_.type() != Json::arrayValue )
  {
    return false;
  }

  const Json::Value& value_node = root_[node_index];

  if( !value_node || ( !value_node.isArray() && !value_node.isObject() ) )
  {
    return false;
  }

  node = std::make_shared < JsonCppNode > ( value_node );
  return true;
}

bool JsonCppNode::set_value( const std::vector< std::string >& value_path, const std::string& value )
{
  Json::Value& value_node = set_value_path( value_path );

  value_node = value;
  return true;
}

bool JsonCppNode::set_value( int value_index, const std::string& value )
{
  Json::Value& value_node = root_[value_index];

  value_node = value;
  return true;
}

bool JsonCppNode::set_node( const std::vector< std::string >& node_path, const JsonNodeInterface::SPtr& node )
{
  Json::Value& value_node = set_value_path( node_path );

  value_node = static_cast< JsonCppNode* >( node.get() )->root_;
  return true;
}

bool JsonCppNode::set_node( int node_index, const JsonNodeInterface::SPtr& node )
{
  Json::Value& value_node = root_[node_index];

  value_node = static_cast< JsonCppNode* >( node.get() )->root_;
  return true;
}

const Json::Value& JsonCppNode::get_value_path( const std::vector< std::string >& value_path )
{
  const Json::Value* value_node = &root_;
  for( const std::string& node : value_path )
  {
    value_node = &( *value_node )[node];
  }

  return *value_node;
}

Json::Value& JsonCppNode::set_value_path( const std::vector< std::string >& value_path )
{
  Json::Value* value_node = &root_;
  for( const std::string& node : value_path )
  {
    value_node = &( *value_node )[node];
  }

  return *value_node;
}

std::string JsonCppNode::node_to_string( const Json::Value& node )
{
  std::string value = writer_.write( node );

  value = value.substr( 0, value.size() - 1 );
  if( node.isString() )
  {
    value = value.substr( 1, value.size() - 2 );
  }

  return value;
}
