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
#include <scopes/Variant.h>

namespace unity
{

namespace api
{

namespace scopes
{

class Category;

namespace internal
{

class ResultItemImpl
{
public:
    ResultItemImpl(std::shared_ptr<Category> category);

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_icon(std::string const& icon);
    void set_dnd_uri(std::string const& dnd_uri);
    void set_renderer_hint(std::string const& name, Variant const& value);

    std::string get_uri() const;
    std::string get_title() const;
    std::string get_icon() const;
    std::string get_dnd_uri() const;

    const VariantMap to_variant() const;
    void from_variant(VariantMap const& var);

private:
    std::shared_ptr<Category> category;
    std::string uri;
    std::string title;
    std::string icon;
    std::string dnd_uri;
    VariantMap render_hints;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity


#endif
