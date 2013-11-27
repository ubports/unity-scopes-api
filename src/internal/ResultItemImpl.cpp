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

const std::unordered_set<std::string> ResultItemImpl::standard_attrs = {"uri", "title", "art", "dnd_uri"};

ResultItemImpl::ResultItemImpl()
{
}

ResultItemImpl::ResultItemImpl(VariantMap const& variant_map)
{
    deserialize(variant_map);
}

ResultItemImpl::ResultItemImpl(ResultItemImpl const& other)
    : uri_(other.uri_),
    title_(other.title_),
    art_(other.art_),
    dnd_uri_(other.dnd_uri_)
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
        art_ = other.art_;
        dnd_uri_ = other.dnd_uri_;
        if (other.metadata_)
        {
            metadata_ = std::make_shared<VariantMap>(*(other.metadata_));
        }
        if (other.stored_result_)
        {
            stored_result_ = std::make_shared<VariantMap>(*other.stored_result_);
        }
    }
    return *this;
}

ResultItemImpl::~ResultItemImpl()
{
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
    return ResultItem(*stored_result_);
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

void ResultItemImpl::throw_on_empty(std::string const& name, std::string const& value)
{
    if (value.empty())
    {
        std::ostringstream s;
        s << "Required attribute " << name << " is empty";
        throw InvalidArgumentException(s.str());
    }
}

void ResultItemImpl::serialize_internal(VariantMap& var) const
{
    if (stored_result_)
    {
        var["result"] = *stored_result_;
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

    VariantMap intvar;
    serialize_internal(intvar);
    outer["internal"] = intvar;

    return outer;
}

void ResultItemImpl::deserialize(VariantMap const& var)
{
    // check for ["internal"]["result"] dict which holds stored result.
    auto it = var.find("internal");
    if (it != var.end())
    {
        auto const& resvar = it->second.get_dict();
        it = resvar.find("result");
        if (it != resvar.end())
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

    it = attrs.find("art");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'art'");
    art_ = it->second.get_string();

    it = attrs.find("dnd_uri");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'dnd_uri'");
    dnd_uri_ = it->second.get_string();

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
