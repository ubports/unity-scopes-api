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

#include <scopes/internal/ResultItemImpl.h>
#include <unity/UnityExceptions.h>
#include <scopes/Category.h>
#include <scopes/ResultItem.h>
#include <sstream>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

const std::unordered_set<std::string> ResultItemImpl::standard_attrs = {"uri", "title", "icon", "dnd_uri", "cat_id"};

ResultItemImpl::ResultItemImpl(Category::SCPtr category)
    : category_(category)
{
    if (category_ == nullptr)
    {
        throw InvalidArgumentException("ResultItem: null category");
    }
}

ResultItemImpl::ResultItemImpl(Category::SCPtr category, const VariantMap& variant_map)
    : ResultItemImpl(category)
{
    deserialize(variant_map);
}

ResultItemImpl::ResultItemImpl(ResultItemImpl const& other)
    : uri_(other.uri_),
    title_(other.title_),
    icon_(other.icon_),
    dnd_uri_(other.dnd_uri_),
    category_(other.category_)
{
    if (other.metadata_)
    {
        metadata_ = std::make_shared<VariantMap>(*(other.metadata_));
    }
    if (other.stored_result_)
    {
        stored_result_ = std::make_shared<VariantMap>(*other.stored_result_);
    }
}

ResultItemImpl& ResultItemImpl::operator=(ResultItemImpl const& other)
{
    if (this != &other)
    {
        uri_ = other.uri_;
        title_ = other.title_;
        icon_ = other.icon_;
        dnd_uri_ = other.dnd_uri_;
        if (other.metadata_)
        {
            metadata_ = std::make_shared<VariantMap>(*(other.metadata_));
        }
        if (other.stored_result_)
        {
            stored_result_ = std::make_shared<VariantMap>(*other.stored_result_);
        }
        category_ = other.category_;
    }
    return *this;
}

void ResultItemImpl::store(ResultItem const& other)
{
    if (this == other.p.get())
    {
        throw InvalidArgumentException("ResultItem:: cannot store self");
    }
    stored_result_.reset(new VariantMap(other.serialize()));
}

bool ResultItemImpl::has_stored_result() const
{
    return stored_result_ != nullptr;
}

ResultItem ResultItemImpl::retrieve() const
{
    if (stored_result_ == nullptr)
    {
        throw InvalidArgumentException("ResultItem: no result has been stored");
    }
    return ResultItem(category_, *stored_result_); //FIXME: this category from the outer result!
}

void ResultItemImpl::set_uri(std::string const& uri)
{
    uri_ = uri;
}

void ResultItemImpl::set_title(std::string const& title)
{
    title_ = title;
}

void ResultItemImpl::set_icon(std::string const& icon)
{
    icon_ = icon;
}

void ResultItemImpl::set_dnd_uri(std::string const& dnd_uri)
{
    dnd_uri_ = dnd_uri;
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

std::string ResultItemImpl::icon() const
{
    return icon_;
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
    throw_on_empty("icon", icon_);
    throw_on_empty("dnd_uri", dnd_uri_);

    VariantMap var;
    var["uri"] = uri_;
    var["title"] = title_;
    var["icon"] = icon_;
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

    VariantMap outer;
    outer["attrs"] = var;
    if (stored_result_)
    {
        VariantMap stored_result_var;
        stored_result_var["result"] = *stored_result_;
        outer["internal"] = stored_result_var;
    }
    return outer;
}

void ResultItemImpl::deserialize(VariantMap const& var)
{
    // check for ["internal"]["result"] dict which holds stored result.
    auto it = var.find("internal");
    if (it != var.end())
    {
        it = it->second.get_dict().find("result");
        if (it != var.end())
        {
            stored_result_.reset(new VariantMap(it->second.get_dict()));
        }
    }

    // check for ["attrs"] dict which holds all attributes
    it = var.find("attrs");
    if (it == var.end())
    {
        throw InvalidArgumentException("Invalid variant structure");
    }

    const VariantMap attrs = it->second.get_dict();

    it = attrs.find("uri");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'uri'");
    uri_ = it->second.get_string();

    it = attrs.find("title");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'title'");
    title_ = it->second.get_string();

    it = attrs.find("icon");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'icon'");
    icon_ = it->second.get_string();

    it = attrs.find("dnd_uri");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'dnd_uri'");
    dnd_uri_ = it->second.get_string();

    // cat_id is not really used as it's provided by category instance pointer,
    // but the check is here for consistency and to make sure a valid attrsiant
    // is passed.
    it = attrs.find("cat_id");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'cat_id'");

    if (attrs.size() > standard_attrs.size())
    {
        for (auto const& kv: attrs)
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
