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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

namespace unity
{

namespace scopes
{

class Variant;

/**
 \typedef VariantMap
 \brief A dictionary of (string, Variant) pairs.
*/
typedef std::map<std::string, Variant> VariantMap;

/**
 \brief An array of variants
*/
typedef std::vector<Variant> VariantArray;

namespace internal
{

struct VariantImpl;
struct NullVariant;

} // namespace internal

/**
\brief Simple variant class that can hold an integer, boolean, string, double, dictionary, array or null value.
*/

class Variant final // LCOV_EXCL_LINE // lcov incorrectly reports this line as uncovered
{
public:
    /**
    \brief Type of value held by a Variant instance.
    */
    enum Type { Null, Int, Bool, String, Double, Dict, Array, Int64 };

    /**@name Constructors and destructor
    */
    /**
    \brief The default constructor creates a Variant instance containing a null.
    */
    Variant() noexcept;

    /**
    \brief Creates a Variant instance that stores the supplied integer.
    */
    explicit Variant(int val) noexcept;

    explicit Variant(int64_t val) noexcept;

    /**
       \brief Creates a Variant instance that stores the supplied double.
    */
    explicit Variant(double val) noexcept;

    /**
    \brief Creates a Variant instance that stores the supplied boolean.
    */
    explicit Variant(bool val) noexcept;

    /**
    \brief Creates a Variant instance that stores the supplied string.
    */
    explicit Variant(std::string const& val);

    /**
    \brief Converts the supplied pointer to a string and stores the string in the Variant instance.
    */
    explicit Variant(char const* val);          // Required to prevent Variant("Hello") from storing a bool

    explicit Variant(VariantMap const& val);

    explicit Variant(VariantArray const& val);

    /**
    \brief Construct a null variant.
    */
    static Variant const& null();

    /**
    \brief Destructor.
    */
    ~Variant();

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Variant(Variant const&);
    Variant(Variant&&);
    Variant& operator=(Variant const&);
    Variant& operator=(Variant&&);
    //@}

    /**@name Value assignment
    The assignment operators replace the value currently held by a Variant with the supplied value, potentially
    changing the type of the value held by the Variant. Assigning a `const char*` to a Variant stores the
    corresponding `std::string` value.
    */
    //{@
    Variant& operator=(int val) noexcept;
    Variant& operator=(int64_t val) noexcept;
    Variant& operator=(double val) noexcept;
    Variant& operator=(bool val) noexcept;
    Variant& operator=(std::string const& val);
    Variant& operator=(char const* val);        // Required to prevent v = "Hello" from storing a bool
    Variant& operator=(VariantMap const& val);
    Variant& operator=(VariantArray const& val);
    //@}

    /**@name Comparison operators
    Two variants are considered equal if they both store values of the same type and equal value.

    For Variants storing values of different type, ordering follows the types defined in the Type enum. For example,
    any integer value compares less than any boolean value. For Variants of the same type, the stored values determine
    order as usual.
    */
    //{@
    bool operator==(Variant const&) const noexcept;
    bool operator<(Variant const&) const noexcept;
    //@}

    /**@name Value accessors
    The accessor methods retrieve a value of the specified type.

    If a Variant currently stores a value of different type, these methods throw `unity::LogicException`.
    */
    //{@
    int get_int() const;
    int64_t get_int64_t() const;
    double get_double() const;
    bool get_bool() const;
    std::string get_string() const;
    VariantMap get_dict() const;
    VariantArray get_array() const;

    /**
    \brief Test if variant holds null value.
    \return True if variant holds null.
    */
    bool is_null() const;
    //@}

    /**@name Observers
    */
    /**
    \brief Returns the type of value currently stored by this Variant.
    */
    Type which() const noexcept;

    /**@name Modifiers
    */
    /**
    \brief Swaps the contents of this Variant with `other`.
    */
    void swap(Variant& other) noexcept;

    /**@name Serialization
    */
    //@{
    /**
    \brief Serializes the variant to a JSON encoded string.
    */
    std::string serialize_json() const;
    /**
    \brief Deserializes a JSON encoded string to a `Variant`.
    */
    static Variant deserialize_json(std::string const& json_string);
    //@}

private:
    Variant(internal::NullVariant const&);

    std::unique_ptr<internal::VariantImpl> p;
    friend class VariantImpl;
};

/**
\brief Swaps the contents of two Variants.
*/
void swap(Variant&, Variant&) noexcept;

} // namespace scopes

} // namespace unity
