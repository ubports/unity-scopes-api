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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONCPPPARSER_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONCPPPARSER_H

#include <scopes/internal/smartscopes/JsonParserInterface.h>

#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class JsonCppNode : public JsonNodeInterface
{
public:
    explicit JsonCppNode( const std::string& json_string = "" );
    explicit JsonCppNode( const Json::Value& root );
    ~JsonCppNode();

    void clear_json() override;

    bool read_json( const std::string& json_string ) override;
    bool write_json( std::string& json_string ) override;

    int size() override;

    bool get_value( const std::vector<std::string>& value_path, std::string& value ) override;
    bool get_value( int value_index, std::string& value ) override;
    bool get_node( const std::vector<std::string>& node_path, JsonNodeInterface::SPtr& node ) override;
    bool get_node( int node_index, JsonNodeInterface::SPtr& node ) override;

    bool set_value( const std::vector<std::string>& value_path, const std::string& value ) override;
    bool set_value( int value_index, const std::string& value ) override;
    bool set_node( const std::vector<std::string>& node_path, const JsonNodeInterface::SPtr& node ) override;
    bool set_node( int node_index, const JsonNodeInterface::SPtr& node ) override;

private:
    const Json::Value& get_value_path( const std::vector<std::string>& value_path );
    Json::Value& set_value_path( const std::vector<std::string>& value_path );

    std::string node_to_string( const Json::Value& node );

private:
    Json::Value root_;
    Json::Reader reader_;
    Json::FastWriter writer_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONCPPPARSER_H
