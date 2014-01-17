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

#include <unity/scopes/internal/ResultImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/Result.h>
#include <sstream>
#include <cassert>

namespace unity
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

void ResultImpl::store(Result const& other, bool intercept_activation)
{
    if (this == other.p.get())
    {
        throw InvalidArgumentException("Result:: cannot store self");
    }
    if (intercept_activation)
    {
        set_intercept_activation();
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

void ResultImpl::set_intercept_activation()
{
    flags_ |= Flags::InterceptActivation;

    // clear the origin scope name, ReplyObject with set it anew with correct scope name (i.e. this scope);
    // this is needed to support the case where aggregator scope just passes the original result
    // upstream - in that case we want the original scope to receive activation.
    origin_.clear();
}

bool ResultImpl::find_stored_result(std::function<bool(Flags)> const& cmp_func,
                                    std::function<void(VariantMap const&)> const& found_func,
                                    std::function<void(VariantMap const&)> const& not_found_func) const
{
    if (stored_result_ == nullptr)
        return false;

    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, it is direct activation in the shell
    bool found = false;
    VariantMap stored = *stored_result_;
    while (!found)
    {
        auto it = stored.find("internal");
        if (it == stored.end())
        {
            throw LogicException("ResultImpl::find_stored_result(): Invalid structure of stored result, missing 'internal");
        }
        const VariantMap internal_var = it->second.get_dict();
        auto intit = internal_var.find("flags");
        const Flags flags = (intit != internal_var.end() ? static_cast<Flags>(intit->second.get_int()) : Flags::ActivationNotHandled);
        if (cmp_func(flags))
        {
            found_func(stored);
            found = true;
        }
        else
        {
            not_found_func(stored);
            // nested stored result?
            intit = internal_var.find("result");
            if (intit == internal_var.end())
                break; // reached the most inner result and it doesn't match, so break the loop
            stored = intit->second.get_dict();
        }
    }
    return found;
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
    return !find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; },
                [](VariantMap const&) {},  // do nothing if matches
                [](VariantMap const&) {}); // do nothing if doesn't match
}

std::string ResultImpl::activation_scope_name() const
{
    if ((flags_ & Flags::InterceptActivation) || stored_result_ == nullptr)
    {
        return origin_;
    }

    const auto get_origin = [](VariantMap const& var) -> std::string {
        auto it = var.find("internal");
        if (it != var.end())
        {
            auto intvar = it->second.get_dict();
            it = intvar.find("origin");
            if (it != intvar.end())
            {
                return it->second.get_string();
            }
            throw unity::LogicException("Result::activation_scope_name(): 'origin' element missing");
        }
        throw unity::LogicException("Result::activation_scope_name(): 'internal' element missing");
    };

    std::string target;
    std::string most_inner; // the most inner origin for nested results (from aggregator scopes)
    // visit stored results recursively,
    // check if any of them intercepts activation;
    // if not, return the most inner origin
    if (find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; }, // condition
                [&target, &get_origin](VariantMap const& var) {                        // if found
                    target = get_origin(var);
                },
                [&most_inner, &get_origin](VariantMap const& var) {                    // if not found
                    most_inner = get_origin(var);
                })
       )
    {
        return target;
    }
    return most_inner;
}

VariantMap ResultImpl::activation_target() const
{
    if ((flags_ & Flags::InterceptActivation) || stored_result_ == nullptr)
    {
        return serialize();
    }

    VariantMap res;
    VariantMap most_inner;
    // visit stored results recursively,
    // check if any of them intercepts activation, if so, return it.;
    // if not, return the most inner result.
    if (find_stored_result(
                [](Flags f) -> bool { return (f & Flags::InterceptActivation) != 0; }, // condition
                [&res](VariantMap const& var) {                                        // if found
                    res = var;
                },
                [&most_inner](VariantMap const& var) {                                 // if not found
                    most_inner = var;
                })
       )
    {
        return res;
    }
    return most_inner;
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

} // namespace unity
