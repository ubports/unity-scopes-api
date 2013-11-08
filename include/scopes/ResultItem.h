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

#ifndef UNITY_API_SCOPES_RESULTITEM_H
#define UNITY_API_SCOPES_RESULTITEM_H

#include <unity/util/NonCopyable.h>
#include <scopes/Variant.h>
#include <scopes/Category.h>
#include <string>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    class ResultItemImpl;
    class ReplyImpl;
    class ReplyObject;
}

class UNITY_API ResultItem : private util::NonCopyable
{
public:
    ResultItem(Category::SPtr category);

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_icon(std::string const& icon);
    void set_dnd_uri(std::string const& dnd_uri);
    void set_renderer_hint(std::string const& name, Variant const& value);

    std::string get_uri() const;
    std::string get_title() const;
    std::string get_icon() const;
    std::string get_dnd_uri() const;

private:
    std::shared_ptr<internal::ResultItemImpl> p;

    friend class internal::ResultItemImpl;
    friend class internal::ReplyObject;
    friend class internal::ReplyImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
