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
#include <cassert>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ResultImpl::ResultImpl()
    : flags_(Flags::ActivationNotHandled)
{
}

ResultImpl::ResultImpl(VariantMap const& variant_map)
    : flags_(Flags::ActivationNotHandled)
{
    deserialize(variant_map);
}

ResultImpl::ResultImpl(ResultImpl const& other)
    : attrs_(other.attrs_),
      origin_(other.origin_),
      flags_(other.flags_)
{
    if (other.stored_result_)
    {
        stored_result_ = std::make_shared<VariantMap>(*other.stored_result_);
    }
}

ResultImpl& ResultImpl::operator=(ResultImpl const& other)
{
    if (this != &other)
    {
        attrs_ = other.attrs_;
        flags_ = other.flags_;
        origin_ = other.origin_;
        if (other.stored_result_)
        {
            stored_result_ = std::make_shared<VariantMap>(*other.stored_result_);
        }
    }
    return *this;
}

void ResultImpl::store(Result const& other, bool intercept_preview_req)
{
    if (this == other.p.get())
    {
        throw InvalidArgumentException("Result:: cannot store self");
    }
    if (intercept_preview_req)
    {
        flags_ |= Flags::InterceptPreview;
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

void ResultImpl::set_origin(std::string const& scope_name)
{
    origin_ = scope_name;
}

void ResultImpl::set_uri(std::string const& uri)
{
    attrs_["uri"] = uri;
}

void ResultImpl::set_title(std::string const& title)
{
    attrs_["title"] = title;
}

void ResultImpl::set_art(std::string const& art)
{
    attrs_["art"] = art;
}

void ResultImpl::set_dnd_uri(std::string const& dnd_uri)
{
    attrs_["dnd_uri"] = dnd_uri;
}

void ResultImpl::intercept_activation()
{
    flags_ |= Flags::InterceptActivation;
    origin_.clear(); //clear the origin scope name, ReplyObject with set it anew with correct scope
}

bool ResultImpl::find_stored_result(std::function<bool(Flags)> const& cmp_func, std::function<void(VariantMap const&)> const& found_func) const
{
    if (stored_result_ == nullptr)
        return false;

    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, it is direct activation in the shell
    for (VariantMap stored = *stored_result_;;)
    {
        auto it = stored.find("internal");
        if (it == stored.end())
        {
            throw LogicException("Invalid structure of stored result, missing 'internal");
        }
        const VariantMap internal_var = it->second.get_dict();
        auto intit = internal_var.find("flags");
        const Flags flags = (intit != internal_var.end() ? static_cast<Flags>(intit->second.get_int()) : Flags::ActivationNotHandled);
        if (cmp_func(flags))
        {
            found_func(stored);
            return true;
        }

        // nested stored result?
        intit = internal_var.find("result");
        if (intit == internal_var.end())
            break;
        stored = intit->second.get_dict();
    }
    return false;
}

bool ResultImpl::direct_activation() const
{
    if (flags_ & Flags::InterceptActivation)
    {
        return false;
    }

    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, it is direct activation in the shell
    if (find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; },
                [](VariantMap const&) {})
       )
    {
        return false;
    }
    return true;
}

std::string ResultImpl::activation_scope_name() const
{
    if (flags_ & Flags::InterceptActivation)
    {
        return origin_;
    }

    std::string target;
    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, it is direct activation in the shell
    if (find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; },
                [&target](VariantMap const& var) {
                    auto it = var.find("internal");
                    if (it != var.end())
                    {
                        it = it->second.get_dict().find("origin");
                        target = it->second.get_string();
                    }
                })
       )
    {
        assert(target.empty());
        return target;
    }
    throw LogicException("No activation target for result with uri '" + uri() + "', it should be activated directly");
}

VariantMap ResultImpl::activation_target() const
{
    if (flags_ & Flags::InterceptActivation)
    {
        return serialize();
    }

    VariantMap res;
    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, it is direct activation in the shell
    if (find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; },
                [&res](VariantMap const& var) {
                    res = var;
                })
       )
    {
        return res;
    }
    throw LogicException("No activation target for result with uri '" + uri() + "', it should be activated directly");
}

int ResultImpl::flags() const
{
    return flags_;
}

Variant& ResultImpl::operator[](std::string const& key)
{
    return attrs_[key];
}

Variant const& ResultImpl::operator[](std::string const& key) const
{
    return value(key);
}

std::string ResultImpl::uri() const noexcept
{
    auto const it = attrs_.find("uri");
    if (it != attrs_.end() && it->second.which() == Variant::Type::String)
    {
        return it->second.get_string();
    }
    return "";
}

std::string ResultImpl::title() const noexcept
{
    auto const it = attrs_.find("title");
    if (it != attrs_.end() && it->second.which() == Variant::Type::String)
    {
        return it->second.get_string();
    }
    return "";
}

std::string ResultImpl::art() const noexcept
{
    auto const it = attrs_.find("art");
    if (it != attrs_.end() && it->second.which() == Variant::Type::String)
    {
        return it->second.get_string();
    }
    return "";
}

std::string ResultImpl::dnd_uri() const noexcept
{
    auto const it = attrs_.find("dnd_uri");
    if (it != attrs_.end() && it->second.which() == Variant::Type::String)
    {
        return it->second.get_string();
    }
    return "";
}

std::string ResultImpl::origin() const noexcept
{
    return origin_;
}

bool ResultImpl::contains(std::string const& key) const
{
    return attrs_.find(key) != attrs_.end();
}

Variant const& ResultImpl::value(std::string const& key) const
{
    auto const& it = attrs_.find(key);
    if (it != attrs_.end())
    {
        return it->second;
    }
    std::ostringstream s;
    s << "Result::value(): requested key " << key << " doesn't exist";
    throw InvalidArgumentException(s.str());
}

void ResultImpl::throw_on_non_string(std::string const& name, Variant::Type vtype) const
{
    if (vtype != Variant::Type::String)
    {
        throw InvalidArgumentException("ResultItem: wrong type of attribute: " + name);
    }
}

void ResultImpl::throw_on_empty(std::string const& name) const
{
    auto const it = attrs_.find(name);
    if (it == attrs_.end())
    {
        throw InvalidArgumentException("ResultItem: missing required attribute: " + name);
    }
    throw_on_non_string(name, it->second.which());
}

void ResultImpl::serialize_internal(VariantMap& var) const
{
    if (flags_ != 0)
    {
        var["flags"] = flags_;
    }
    if (!origin_.empty())
    {
        var["origin"] = origin_;
    }
    if (stored_result_)
    {
        var["result"] = *stored_result_;
    }
}

VariantMap ResultImpl::serialize() const
{
    throw_on_empty("uri");
    throw_on_empty("dnd_uri");

    VariantMap outer;
    outer["attrs"] = Variant(attrs_);

    VariantMap intvar;
    serialize_internal(intvar);
    outer["internal"] = std::move(intvar);

    return outer;
}

void ResultImpl::deserialize(VariantMap const& var)
{
    // check for ["internal"]["result"] dict which holds stored result.
    auto it = var.find("internal");
    if (it == var.end())
    {
        throw InvalidArgumentException("Missing 'internal' element'");
    }
    else
    {
        auto const& resvar = it->second.get_dict();
        it = resvar.find("flags");
        if (it != resvar.end())
        {
            flags_ = it->second.get_int();
        }
        it = resvar.find("origin");
        if (it != resvar.end())
        {
            origin_ = it->second.get_string();
        }
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

    it = attrs.find("dnd_uri");
    if (it == attrs.end())
        throw InvalidArgumentException("Missing 'dnd_uri'");

    for (auto const& kv: attrs)
    {
        this->operator[](kv.first) = kv.second;
    }
}

Result ResultImpl::create_result(VariantMap const& variant_map)
{
    return Result(variant_map);
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
