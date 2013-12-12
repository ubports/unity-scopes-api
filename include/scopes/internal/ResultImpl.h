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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#ifndef UNITY_API_SCOPES_RESULTITEMIMPL_H
#define UNITY_API_SCOPES_RESULTITEMIMPL_H

#include <string>
#include <memory>
#include <unordered_set>
#include <scopes/Variant.h>

namespace unity
{

namespace api
{

namespace scopes
{

class Result;

namespace internal
{

class ResultImpl
{
public:
    ResultImpl();
    ResultImpl(VariantMap const& variant_map);
    ResultImpl(ResultImpl const& other);
    ResultImpl& operator=(ResultImpl const& other);

    virtual ~ResultImpl();

    void store(Result const& other);
    bool has_stored_result() const;
    Result retrieve() const;

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_art(std::string const& image);
    void set_dnd_uri(std::string const& dnd_uri);
    void add_metadata(std::string const& key, Variant const& value);
    Variant& operator[](std::string const& key);
    Variant const& operator[](std::string const& key) const;

    std::string uri() const;
    std::string title() const;
    std::string art() const;
    std::string dnd_uri() const;
    bool has_metadata(std::string const& key) const;
    Variant const& metadata(std::string const& key) const;

    VariantMap serialize() const;

protected:
    virtual void serialize_internal(VariantMap& var) const;

private:
    void deserialize(VariantMap const& var);
    static void throw_on_non_string(std::string const& name, Variant::Type vtype);
    static void throw_on_empty(std::string const& name, Variant const& value);
    static const std::unordered_set<std::string> standard_attrs;

    Variant uri_;
    Variant title_;
    Variant art_;
    Variant dnd_uri_;
    std::shared_ptr<VariantMap> metadata_;
    std::shared_ptr<VariantMap> stored_result_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity


#endif
