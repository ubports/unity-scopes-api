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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONPARSERINTERFACE_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONPARSERINTERFACE_H

#include <unity/util/DefinesPtrs.h>

#include <string>
#include <vector>

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

class JsonNodeInterface
{
public:
    UNITY_DEFINES_PTRS(JsonNodeInterface);

    JsonNodeInterface() = default;
    virtual ~JsonNodeInterface() = default;

    virtual void clear_json() = 0;

    virtual bool read_json( const std::string& json_string ) = 0;
    virtual bool write_json( std::string& json_string ) = 0;

    virtual int size() = 0;

    virtual bool get_value( const std::vector<std::string>& value_path, std::string& value ) = 0;
    virtual bool get_value( int value_index, std::string& value ) = 0;
    virtual bool get_node( const std::vector<std::string>& node_path, JsonNodeInterface::SPtr& node ) = 0;
    virtual bool get_node( int node_path, JsonNodeInterface::SPtr& node ) = 0;

    virtual bool set_value( const std::vector<std::string>& value_path, const std::string& value ) = 0;
    virtual bool set_value( int value_index, const std::string& value ) = 0;
    virtual bool set_node( const std::vector<std::string>& node_path, const JsonNodeInterface::SPtr& node ) = 0;
    virtual bool set_node( int node_path, const JsonNodeInterface::SPtr& node ) = 0;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONPARSERINTERFACE_H
