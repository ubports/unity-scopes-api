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

#ifndef UNITY_SCOPES_RESULT_H
#define UNITY_SCOPES_RESULT_H

#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{

class CategorisedResult;

namespace internal
{
class ResultImpl;
class ScopeImpl;
}

/**
   \brief Result encapsulates attributes of any result returned by the Scope.
   Result API provides convienience methods for some typical attributes (title,
   art) but scopes are free to add and use any custom attributes by means of []
   operator. The only required attribute is 'uri' and it must not be empty before
   calling Reply::push.
*/
class Result
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Result);
    /// @endcond

    /**
       \brief Creates a Result that is a copy of another Result.
    */
    Result(Result const& other);

    /**
       \brief Destructor.
    */
    virtual ~Result();

    /// @cond
    Result& operator=(Result const& other);
    Result(Result&&);
    Result& operator=(Result&&);
    /// @endcond

    /**
    \brief Stores a Result inside this Result instance.
    This method is meant to be used by aggregator scopes which want to modify results they receive, but want
    to keep a copy of the original result so that they can be correctly handled by the original scopes
    who created them when it comes to activation or previews.
    Scopes middleware will automatically pass the correct inner stored result to the activation or preview request handler
    of a scope which created it.
    \param other original result to store within this result
    \param intercept_activation whether this scope should receive activation and preview request
    */
    void store(Result const& other, bool intercept_activation = false);

    /**
    \brief Check if this Result instance has a stored result.
    \return true if there is a stored result
     */
    bool has_stored_result() const;

    /**
     \brief Get stored result.
     This method throws unity::InvalidArgumentException if no result has been stored in this Result instance.
     \return stored result
     */
    Result retrieve() const;

    /**
     \brief Set "uri" attribute of this result.
     */
    void set_uri(std::string const& uri);

    /**
     \brief Set "title" attribute of this result.

     Equivalent to calling result["title"] = title;
     */
    void set_title(std::string const& title);
    /**
     \brief Set "art" attribute of this result.

     Equivalent to calling result["art"] = image;
     */
    void set_art(std::string const& image);

    /**
     \brief Set "dnd_uri" attribute of this result.
     */
    void set_dnd_uri(std::string const& dnd_uri);

    /**
     \brief Indicates to the receiver that this scope should intercept activation request for this result.
     By default, scope receives preview requests for the results it creates but does not receive activation
     requests (they are handled directly by the shell).
     Intercepting activation implies intercepting preview requests as well; this is important for scopes which
     just forward results from other scopes and call set_intercept_activation() on them.
     A scope which sets intercept activation flag for a result should re-implement ScopeBase::activate()
     and provide an implementation of ActivationBase that handles actual activation.
     If not called, the result will be activated directly by the Unity shell whithout involving the scope,
     assuming appropriate uri schema handler is present on the system.
     */
    void set_intercept_activation();

    /**
     \brief Check if this result should be activated directly by the shell (scope doesn't handle activation of this result).
     \return true if this result needs to be activated directly
     */
    bool direct_activation() const;

    /**
     \brief Get proxy of a scope that handles activation and preview of this result.
     The proxy is only available when receiving this result from a scope, otherwise this method throws LogicException.
     Note that activation request should only be sent to a scope returned by this method if direct_activation() is false.
     \return scope proxy
     */
    ScopeProxy target_scope_proxy() const;

    /**
       \brief Returns reference of a Result attribute.
       This method can be used to read or initialize both standard ("uri", "title", "art", "dnd_uri")
       and custom metadata attributes. Referencing a non-existing attribute automatically creates
       it with a default value of Variant::Type::Null.
       \return reference of an attribute
     */
    Variant& operator[](std::string const& key);

    /**
       \brief Returns const reference of a Result attribute.
       This method can be used for read-only access to both standard ("uri", "title", "art", "dnd_uri")
       and custom metadata attributes. Referencing a non-existing attribute throws unity::InvalidArgumentException.
       \return const reference of an existing attribute
     */
    Variant const& operator[](std::string const& key) const;

    /**
     \brief Get "uri" property of this Result.
     This method returns empty string if this attribute is not of Variant::Type::String type (e.g. it was set with operator[]).
     \return the value of "uri" or empty string
     */
    std::string uri() const noexcept;

    /**
     \brief Get "title" property of this Result.
     This method returns empty string if this attribute is not of Variant::Type::String type (e.g. it was set with operator[]).
     \return the value of "title" or empty string
     */
    std::string title() const noexcept;

    /**
     \brief Get "art" property of this Result.
     This method returns empty string if this attribute is not of Variant::Type::String type (e.g. it was set with operator[]).
     \return the value of "art" or empty string
     */
    std::string art() const noexcept;

    /**
     \brief Get "dnd_uri" property of this Result.
     This method returns empty string if this attribute is not of Variant::Type::String type (e.g. it was set with operator[]).
     \return the value of "dnd_uri" or empty string
     */
    std::string dnd_uri() const noexcept;

    /**
    \brief Check if this Result has an attribute.
    \param key attribute name
    \return true if attribute is set
    */
    bool contains(std::string const& key) const;

    /**
    \brief Get value of an attribute.
    This method thows unity::InvalidArgumentException if given attribute hasn't been set.
    \param key attribute name
    \return attribute value
    */
    Variant const& value(std::string const& key) const;

    /**
       \brief Returns a dictionary of all attributes of this Result instance.
       \return Dictionary of all base attributes and custom attributes set with add_metadata call.
    */
    VariantMap serialize() const;

private:
    explicit Result(const VariantMap &variant_map);
    explicit Result(internal::ResultImpl* impl);

    std::unique_ptr<internal::ResultImpl> p;

    friend class internal::ResultImpl;
    friend class internal::ScopeImpl;
    friend class CategorisedResult;
};

} // namespace scopes

} // namespace unity

#endif
