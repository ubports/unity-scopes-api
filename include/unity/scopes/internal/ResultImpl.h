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

#ifndef UNITY_SCOPES_INTERNAL_RESULTITEMIMPL_H
#define UNITY_SCOPES_INTERNAL_RESULTITEMIMPL_H

#include <string>
#include <memory>
#include <functional>
#include <unity/scopes/Variant.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/internal/RuntimeImpl.h>

namespace unity
{

namespace scopes
{

class Result;

namespace internal
{
class ResultImpl
{
public:
    // activation and preview flags, used internally only.
    // some of them can potentially be OR'ed if we add more, so let's make them powers of 2
    enum Flags
    {
        ActivationNotHandled = 0, // direct activation
        InterceptActivation = 1
    };

    ResultImpl();
    ResultImpl(VariantMap const& variant_map);
    ResultImpl(ResultImpl const& other);
    ResultImpl& operator=(ResultImpl const& other);

    virtual ~ResultImpl() = default;

    void store(Result const& other, bool intercept_activation);
    bool has_stored_result() const;
    Result retrieve() const;
    void set_runtime(RuntimeImpl const* runtime);
    void set_origin(std::string const& scope_name);

    void set_uri(std::string const& uri);
    void set_title(std::string const& title);
    void set_art(std::string const& image);
    void set_dnd_uri(std::string const& dnd_uri);
    void set_intercept_activation();
    bool direct_activation() const;
    int flags() const;
    ScopeProxy activation_scope() const;
    VariantMap activation_target() const;
    Variant& operator[](std::string const& key);
    Variant const& operator[](std::string const& key) const;

    std::string uri() const noexcept;
    std::string title() const noexcept;
    std::string art() const noexcept;
    std::string dnd_uri() const noexcept;
    std::string origin() const noexcept;
    bool contains(std::string const& key) const;
    Variant const& value(std::string const& key) const;

    VariantMap serialize() const;

    // a public static method, so there's access to the private constructor
    // without the need to define too many classes as friends
    static Result create_result(VariantMap const&);

protected:
    virtual void serialize_internal(VariantMap& var) const;

    // find stored result whose flags give true in cmp_func, pass it to found_func and return;
    // non-matching results are passed to not_found_func.
    // this is done recursively as stored result can be nested.
    // return true if found, otherwise false.
    bool find_stored_result(std::function<bool(Flags)> const& cmp_func,
                            std::function<void(VariantMap const&)> const& found_func,
                            std::function<void(VariantMap const&)> const& not_found_func) const;

private:
    void deserialize(VariantMap const& var);
    void throw_on_non_string(std::string const& name, Variant::Type vtype) const;
    void throw_on_empty(std::string const& name) const;

    VariantMap attrs_;
    std::shared_ptr<VariantMap> stored_result_;
    std::string origin_;
    int flags_;
    RuntimeImpl const* runtime_;
};

} // namespace internal

} // namespace scopes

} // namespace unity


#endif
