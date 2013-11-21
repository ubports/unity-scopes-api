/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/Variant.h>

#include <unity/UnityExceptions.h>

#include <boost/variant.hpp>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

struct NullVariant
{
    bool operator<(NullVariant const&) const
    {
        return false;
    }

    bool operator==(NullVariant const&) const
    {
        return true;
    }
};

struct VariantImpl
{
    boost::variant<int, bool, string, double, VariantMap, VariantArray, NullVariant> v;
};

} // namespace internal

Variant::Variant() noexcept
    : p(new internal::VariantImpl)
{
}

Variant::Variant(int val) noexcept
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(double val) noexcept
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(bool val) noexcept
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(std::string const& val)
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(VariantMap const& val)
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(VariantArray const& val)
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(internal::NullVariant const& val)
    : p(new internal::VariantImpl { val })
{
}

Variant::Variant(char const* val)
    : p(new internal::VariantImpl { string(val) })
{
}

Variant::~Variant() noexcept
{
}

Variant const& Variant::null()
{
    static const Variant var = internal::NullVariant();
    return var;
}

Variant::Variant(Variant const& other)
    : p(new internal::VariantImpl(*other.p))
{
}

Variant::Variant(Variant&&) = default;

Variant& Variant::operator=(Variant const& rhs)
{
    p->v = rhs.p->v;
    return *this;
}

Variant& Variant::operator=(Variant&&) = default;

Variant& Variant::operator=(int val) noexcept
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(double val) noexcept
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(bool val) noexcept
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(std::string const& val)
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(VariantMap const& val)
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(VariantArray const& val)
{
    p->v = val;
    return *this;
}

Variant& Variant::operator=(char const* val)
{
    p->v = string(val);
    return *this;
}

bool Variant::operator==(Variant const& rhs) const noexcept
{
    return p->v == rhs.p->v;
}

bool Variant::operator<(Variant const& rhs) const noexcept
{
    return p->v < rhs.p->v;
}

int Variant::get_int() const
{
    try
    {
        return boost::get<int>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain an int value");
    }
}

double Variant::get_double() const
{
    try
    {
        return boost::get<double>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain a double value");
    }
}

bool Variant::get_bool() const
{
    try
    {
        return boost::get<bool>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain a bool value");
    }
}

string Variant::get_string() const
{
    try
    {
        return boost::get<string>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain a string value");
    }
}

VariantMap Variant::get_dict() const
{
    try
    {
        return boost::get<VariantMap>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain a dictionary");
    }
}

VariantArray Variant::get_array() const
{
    try
    {
        return boost::get<VariantArray>(p->v);
    }
    catch (std::exception const&)
    {
        throw LogicException("Variant does not contain an array");
    }
}

bool Variant::is_null() const
{
    return p->v.which() == Type::Null;
}

Variant::Type Variant::which() const noexcept
{
    auto w = p->v.which();
    return static_cast<Type>(w);
}

// @cond
// For some reason, doxygen flags this as undocumented, even though it is documented in the header.

void swap(Variant& lhs, Variant& rhs) noexcept
{
    lhs.p->v.swap(rhs.p->v);
}
// @endcond

} // namespace scopes

} // namespace api

} // namespace unity
