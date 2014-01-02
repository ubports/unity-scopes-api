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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONNODEINTERFACE_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONNODEINTERFACE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

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
    NONCOPYABLE(JsonNodeInterface);
    UNITY_DEFINES_PTRS(JsonNodeInterface);

    enum NodeType
    {
        Null, Array, Object, String, Int, UInt, Real, Bool
    };

    JsonNodeInterface() = default;
    virtual ~JsonNodeInterface() = default;

    virtual void clear() = 0;
    virtual void read_json(std::string const& json_string) = 0;

    virtual int size() const = 0;
    virtual NodeType type() const = 0;

    virtual std::string as_string() const = 0;
    virtual int as_int() const = 0;
    virtual uint as_uint() const = 0;
    virtual double as_double() const = 0;
    virtual bool as_bool() const = 0;

    virtual bool has_node(std::string const& node_name) const = 0;

    virtual JsonNodeInterface::SPtr get_node() const = 0;
    virtual JsonNodeInterface::SPtr get_node(std::string const& node_name) const = 0;
    virtual JsonNodeInterface::SPtr get_node(uint node_index) const = 0;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_JSONNODEINTERFACE_H
