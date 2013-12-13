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

#include <scopes/internal/ResultImpl.h>
#include <unity/UnityExceptions.h>
#include <scopes/Result.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

const std::unordered_set<std::string> ResultImpl::standard_attrs = {"uri", "title", "art", "dnd_uri"};

ResultImpl::ResultImpl()
{
}

ResultImpl::ResultImpl(VariantMap const& variant_map)
{
    deserialize(variant_map);
}

ResultImpl::ResultImpl(ResultImpl const& other)
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

ResultImpl& ResultImpl::operator=(ResultImpl const& other)
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

ResultImpl::~ResultImpl()
{
}

void ResultImpl::store(Result const& other)
{
    if (this == other.p.get())
    {
        throw InvalidArgumentException("Result:: cannot store self");
    }
    stored_result_.reset(new VariantMap(other.serialize()));
}

bool ResultImpl::has_stored_result() const
{
    return stored_result_ != nullptr;
}

Result ResultImpl::retrieve() const
{
    if (stored_result_ == nullptr)
    {
        throw InvalidArgumentException("Result: no result has been stored");
    }
    return Result(*stored_result_);
}

void ResultImpl::set_uri(std::string const& uri)
{
    uri_ = uri;
}

void ResultImpl::set_title(std::string const& title)
{
    title_ = title;
}

void ResultImpl::set_art(std::string const& image)
{
    art_ = image;
}

void ResultImpl::set_dnd_uri(std::string const& dnd_uri)
{
    dnd_uri_ = dnd_uri;
}

void ResultImpl::add_metadata(std::string const& key, Variant const& value)
{
    if (!metadata_)
    {
        metadata_.reset(new VariantMap());
    }
    (*metadata_)[key] = value;
}

Variant& ResultImpl::operator[](std::string const& key)
{
    if (key == "uri")
        return uri_;
    if (key == "dnd_uri")
        return dnd_uri_;
    if (key == "title")
        return title_;
    if (key == "art")
        return art_;

    if (!metadata_)
    {
        metadata_.reset(new VariantMap());
    }
    return (*metadata_)[key];
}

Variant const& ResultImpl::operator[](std::string const& key) const
{
    if (key == "uri")
        return uri_;
    if (key == "dnd_uri")
        return dnd_uri_;
    if (key == "title")
        return title_;
    if (key == "art")
        return art_;

    return metadata(key);
}

std::string ResultImpl::uri() const noexcept
{
    if (uri_.which() == Variant::Type::String)
    {
        return uri_.get_string();
    }
    return "";
}

std::string ResultImpl::title() const
{
    return title_.get_string();
}

std::string ResultImpl::art() const
{
    return art_.get_string();
}

std::string ResultImpl::dnd_uri() const noexcept
{
    if (uri_.which() == Variant::Type::String)
    {
        return dnd_uri_.get_string();
    }
    return "";
}

bool ResultImpl::has_metadata(std::string const& key) const
{
    if (metadata_ != nullptr)
    {
        return metadata_->find(key) != metadata_->end();
    }
    return false;
}

Variant const& ResultImpl::metadata(std::string const& key) const
{
    if (metadata_ != nullptr)
    {
        auto const& it = metadata_->find(key);
        if (it != metadata_->end())
        {
            return it->second;
        }
    }
    std::ostringstream s;
    s << "Result::metadata(): requested key " << key << " doesn't exist";
    throw InvalidArgumentException(s.str());
}

void ResultImpl::throw_on_non_string(std::string const& name, Variant::Type vtype)
{
    if (vtype != Variant::Type::String)
    {
        throw InvalidArgumentException("ResultItem: wrong type of attribute: " + name);
    }
}

void ResultImpl::throw_on_empty(std::string const& name, Variant const& value)
{
    throw_on_non_string(name, value.which());
    if (value.get_string().empty())
    {
        throw InvalidArgumentException("ResultItem: missing required attribute: " + name);
    }
}

void ResultImpl::serialize_internal(VariantMap& var) const
{
    if (stored_result_)
    {
        var["result"] = *stored_result_;
    }
}

VariantMap ResultImpl::serialize() const
{
    throw_on_empty("uri", uri_);
    throw_on_empty("dnd_uri", dnd_uri_);

    VariantMap var;
    var["uri"] = uri_;
    var["dnd_uri"] = dnd_uri_;
    if (title_.which() != Variant::Type::Null)
    {
        var["title"] = title_;
    }
    if (art_.which() != Variant::Type::Null)
    {
        var["art"] = art_;
    }

    if (metadata_)
    {
        for (auto const& kv: *metadata_)
        {
            if (var.find(kv.first) != var.end())
            {
                throw InvalidArgumentException("ResultItemImpl::serialize(): Can't overwrite internal attribute: "
                                               + kv.first);
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

void ResultImpl::deserialize(VariantMap const& var)
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
    uri_ = it->second;

    it = attrs.find("title");
    if (it != attrs.end())
    {
        title_ = it->second;
    }

    it = attrs.find("art");
    if (it != attrs.end())
    {
        art_ = it->second;
    }

    it = attrs.find("dnd_uri");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'dnd_uri'");
    dnd_uri_ = it->second;

    for (auto const& kv: attrs)
    {
        if (standard_attrs.find(kv.first) == standard_attrs.end()) // skip standard attributes
        {
            add_metadata(kv.first, kv.second);
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
