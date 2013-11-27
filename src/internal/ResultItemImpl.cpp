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

#include <scopes/internal/ResultItemImpl.h>
#include <unity/UnityExceptions.h>
#include <scopes/Category.h>
#include <sstream>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

const std::unordered_set<std::string> ResultItemImpl::standard_attrs = {"uri", "title", "art", "dnd_uri", "cat_id"};

ResultItemImpl::ResultItemImpl(Category::SCPtr category)
{
    set_category(category);
}

ResultItemImpl::ResultItemImpl(Category::SCPtr category, const VariantMap& variant_map)
    : ResultItemImpl(category)
{
    deserialize(variant_map);
}

void ResultItemImpl::set_uri(std::string const& uri)
{
    uri_ = uri;
}

void ResultItemImpl::set_title(std::string const& title)
{
    title_ = title;
}

void ResultItemImpl::set_art(std::string const& image)
{
    art_ = image;
}

void ResultItemImpl::set_dnd_uri(std::string const& dnd_uri)
{
    dnd_uri_ = dnd_uri;
}

void ResultItemImpl::set_category(Category::SCPtr category)
{
    if (category == nullptr)
    {
        throw InvalidArgumentException("Category must not be null");
    }
    category_ = category;
}

void ResultItemImpl::add_metadata(std::string const& key, Variant const& value)
{
    if (!metadata_)
    {
        metadata_.reset(new VariantMap());
    }
    (*metadata_)[key] = value;
}

std::string ResultItemImpl::uri() const
{
    return uri_;
}

std::string ResultItemImpl::title() const
{
    return title_;
}

std::string ResultItemImpl::art() const
{
    return art_;
}

std::string ResultItemImpl::dnd_uri() const
{
    return dnd_uri_;
}

Category::SCPtr ResultItemImpl::category() const
{
    return category_;
}

void ResultItemImpl::throw_on_empty(std::string const& name, std::string const& value)
{
    if (value.empty())
    {
        std::ostringstream s;
        s << "Required attribute " << name << " is empty";
        throw InvalidArgumentException(s.str());
    }
}

VariantMap ResultItemImpl::serialize() const
{
    throw_on_empty("uri", uri_);
    throw_on_empty("title", title_);
    throw_on_empty("art", art_);
    throw_on_empty("dnd_uri", dnd_uri_);

    VariantMap var;
    var["uri"] = uri_;
    var["title"] = title_;
    var["art"] = art_;
    var["dnd_uri"] = dnd_uri_;
    var["cat_id"] = category_->id();

    if (metadata_)
    {
        for (auto const& kv: *metadata_)
        {
            if (var.find(kv.first) != var.end())
            {
                std::ostringstream s;
                s << "Can't overwrite internal attribute: " << kv.first;
                throw InvalidArgumentException(s.str());
            }
            var[kv.first] = kv.second;
        }
    }
    return var;
}

void ResultItemImpl::deserialize(VariantMap const& var)
{
    auto it = var.find("uri");
    if (it == var.end())
        throw InvalidArgumentException("Missing 'uri'");
    uri_ = it->second.get_string();

    it = var.find("title");
    if (it == var.end())
        throw InvalidArgumentException("Missing 'title'");
    title_ = it->second.get_string();

    it = var.find("art");
    if (it == var.end())
        throw InvalidArgumentException("Missing 'art'");
    art_ = it->second.get_string();

    it = var.find("dnd_uri");
    if (it == var.end())
        throw InvalidArgumentException("Missing 'dnd_uri'");
    dnd_uri_ = it->second.get_string();

    // cat_id is not really used as it's provided by category instance pointer,
    // but the check is here for consistency and to make sure a valid variant
    // is passed.
    it = var.find("cat_id");
    if (it == var.end())
        throw InvalidArgumentException("Missing 'cat_id'");

    if (var.size() > standard_attrs.size())
    {
        for (auto const& kv: var)
        {
            if (standard_attrs.find(kv.first) == standard_attrs.end()) // skip standard attributes
            {
                add_metadata(kv.first, kv.second);
            }
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
