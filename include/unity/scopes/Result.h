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
   \brief The attributes of a result returned by a Scope.

   The Result API provides convenience methods for some typical attributes (title,
   art), but scopes are free to add and use any custom attributes with `operator[]`.
   The only required attribute is 'uri' and it must not be empty before
   calling Reply::push().
*/

class Result
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Result);
    /// @endcond

    /**@name Copy and assignment
    Copy and assignment (move and non-move versions) have the usual value semantics.
    */
    //{@
    Result(Result const& other);
    Result(Result&&);

    Result& operator=(Result const& other);
    Result& operator=(Result&&);
    //@}

    /**
    Destructor.
    */
    virtual ~Result();

    /**
    \brief Stores a Result inside this Result instance.

    This method is meant to be used by aggregator scopes which want to modify results they receive, but want
    to keep a copy of the original result so that they can be correctly handled by the original scopes
    who created them when it comes to activation or previews.
    Scopes middleware will automatically pass the correct inner stored result to the activation or preview request handler
    of a scope which created it.
    \param other The original result to store within this result.
    \param intercept_activation True if this scope should receive activation and preview requests.
    */
    void store(Result const& other, bool intercept_activation = false);

    /**
    \brief Check if this Result instance has a stored result.
    \return True if there is a stored result
     */
    bool has_stored_result() const;

    /**
     \brief Get a stored result.
     \return stored result
     \throws unity::InvalidArgumentException if no result was stored in this Result instance.
     */
    Result retrieve() const;

    /**
     \brief Set the "uri" attribute of this result.
     */
    void set_uri(std::string const& uri);

    /**
     \brief Set the "title" attribute of this result.

     Equivalent to calling `result["title"] = title;`
     */
    void set_title(std::string const& title);
    /**
     \brief Set the "art" attribute of this result.

     Equivalent to calling `result["art"] = image;`
     */
    void set_art(std::string const& image);

    /**
     \brief Set the "dnd_uri" attribute of this result.

     Equivalent to calling `result["dnd_uri"] = dnd_uri;`
     */
    void set_dnd_uri(std::string const& dnd_uri);

    /**
     \brief Indicates to the receiver that this scope should intercept activation requests for this result.

     By default, a scope receives preview requests for the results it creates, but does not receive activation
     requests (they are handled directly by the shell).
     Intercepting activation implies intercepting preview requests as well; this is important for scopes that
     forward results from other scopes and call set_intercept_activation() on these scopes.
     A scope that sets intercept activation flag for a result should re-implement ScopeBase::activate()
     and provide an implementation of ActivationQueryBase that handles the actual activation.
     If not called, the result will be activated directly by the Unity shell whithout involving the scope,
     assuming an appropriate URI schema handler is present on the system.
     */
    void set_intercept_activation();

    /**
     \brief Check if this result should be activated directly by the shell
            because the scope doesn't handle activation of this result.
     \return True if this result needs to be activated directly.
     */
    bool direct_activation() const;

    /**
     \brief Get the proxy of a scope that handles activation and preview of this result.

     The proxy is only available when receiving this result from a scope, otherwise
     this method throws LogicException. Activation requests should only be sent to a scope
     returned by this method only if direct_activation() is false.
     \return The scope proxy.
     */
    ScopeProxy target_scope_proxy() const;

    /**
       \brief Returns reference of a Result attribute.

       This method can be used to read or initialize both standard ("uri", "title", "art", "dnd_uri")
       and custom metadata attributes. Referencing a non-existing attribute automatically creates
       it with a default value of Variant::Type::Null.
       \param key The name of the attribute.
       \return A reference to the attribute.
       \throws unity::Invalidargument if no attribute with the given name exists.
     */
    Variant& operator[](std::string const& key);

    /**
       \brief Returns a const reference to a Result attribute.

       This method can be used for read-only access to both standard ("uri", "title", "art", "dnd_uri")
       and custom metadata attributes. Referencing a non-existing attribute throws unity::InvalidArgumentException.
       \param key The name of the attribute.
       \return A const reference to the attribute.
       \throws unity::Invalidargument if no attribute with the given name exists.
     */
    Variant const& operator[](std::string const& key) const;

    /**
     \brief Get the "uri" property of this Result.

     This method returns an empty string if this attribute is not of type Variant::Type::String (e.g. it was set with operator[]).
     \return The value of "uri" or the empty string.
     */
    std::string uri() const noexcept;

    /**
     \brief Get the "title" property of this Result.

     This method returns an empty string if this attribute is not of type Variant::Type::String (e.g. it was set with operator[]).
     \return The value of "title" or the empty string.
     */
    std::string title() const noexcept;

    /**
     \brief Get the "art" property of this Result.

     This method returns an empty string if this attribute is not of type Variant::Type::String (e.g. it was set with operator[]).
     \return The value of "art" or the empty string.
     */
    std::string art() const noexcept;

    /**
     \brief Get the "dnd_uri" property of this Result.

     This method returns an empty string if this attribute is not of type Variant::Type::String type (e.g. it was set with operator[]).
     \return The value of "dnd_uri" or the empty string.
     */
    std::string dnd_uri() const noexcept;

    /**
    \brief Check if this Result has an attribute.
    \param key The attribute name.
    \return True if the attribute is set.
    */
    bool contains(std::string const& key) const;

    /**
    \brief Get the value of an attribute.

    \param key The attribute name.
    \return The attribute value.
    \throws unity::InvalidArgumentException if given attribute hasn't been set.
    */
    Variant const& value(std::string const& key) const;

    /**
    \brief Returns a dictionary of all attributes of this Result instance.
    \return All base attributes and custom attributes set with add_metadata().
    */
    VariantMap serialize() const;

protected:
    /// @cond
    explicit Result(const VariantMap &variant_map);
    explicit Result(internal::ResultImpl* impl);
    /// @endcond

private:
    std::unique_ptr<internal::ResultImpl> p;

    friend class internal::ResultImpl;
    friend class internal::ScopeImpl;
    friend class CategorisedResult;
};

} // namespace scopes

} // namespace unity

#endif
