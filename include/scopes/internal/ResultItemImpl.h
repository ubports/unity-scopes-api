/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
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
#include <scopes/Category.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ResultItemImpl
{
public:
    explicit ResultItemImpl(Category::SCPtr category);
    ResultItemImpl(Category::SCPtr category, VariantMap const& variant_map);
    ResultItemImpl(ResultItemImpl const& other) = default;
    ResultItemImpl& operator=(ResultItemImpl const& other) = default;

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_icon(std::string const& icon);
    void set_dnd_uri(std::string const& dnd_uri);
    void add_metadata(std::string const& key, Variant const& value);

    std::string uri() const;
    std::string title() const;
    std::string icon() const;
    std::string dnd_uri() const;
    Category::SCPtr category() const;

    std::shared_ptr<VariantMap> serialize() const;

private:
    void deserialize(VariantMap const& var);
    static void throw_on_empty(std::string const& name, std::string const& value);
    static const std::unordered_set<std::string> standard_attrs;

    std::string uri_;
    std::string title_;
    std::string icon_;
    std::string dnd_uri_;
    std::shared_ptr<VariantMap> metadata_;
    Category::SCPtr category_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity


#endif
