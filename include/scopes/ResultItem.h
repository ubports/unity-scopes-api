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

#ifndef UNITY_API_SCOPES_RESULTITEM_H
#define UNITY_API_SCOPES_RESULTITEM_H

#include <scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

class Result;

namespace internal
{
class ResultItemImpl;
}

/**
   \brief ResultItem encapsulates the basic attributes of any result
   returned by the Scope. The basic attributes (uri, title, icon, dnd_uri) must not be empty before
   calling Reply::push.
*/
class UNITY_API ResultItem
{
public:
    /**
       \brief Creates a ResultItem that is a copy of another ResultItem.
    */
    ResultItem(ResultItem const& other);

    /**
       \brief Destructor.
    */
    virtual ~ResultItem();

    ResultItem& operator=(ResultItem const& other);
    ResultItem(ResultItem&&);
    ResultItem& operator=(ResultItem&&);

    void store(ResultItem const& other);
    bool has_stored_result() const;
    ResultItem retrieve() const;

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_art(std::string const& image);
    void set_dnd_uri(std::string const& dnd_uri);
    void add_metadata(std::string const& key, Variant const& value);

    std::string uri() const;
    std::string title() const;
    std::string art() const;
    std::string dnd_uri() const;

    /**
       \brief Returns a dictionary of all attributes of this ResultItem instance.
       \return dictionary of all base attributes and custom attributes set with add_metadata call.
    */
    VariantMap serialize() const;

private:
    explicit ResultItem(const VariantMap &variant_map);
    ResultItem(internal::ResultItemImpl* impl);

    std::shared_ptr<internal::ResultItemImpl> p;

    friend class internal::ResultItemImpl;
    friend class Result;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
