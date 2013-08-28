/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/ice_middleware/VariantValueConverter.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

middleware::ValueBasePtr to_value(Variant const& v)
{
    switch (v.which())
    {
        case Variant::Int:
        {
            return new middleware::IntValue(middleware::IntType, v.get_int());
        }
        case Variant::Bool:
        {
            return new middleware::BoolValue(middleware::BoolType, v.get_bool());
        }
        case Variant::String:
        {
            return new middleware::StringValue(middleware::StringType, v.get_string());
        }
        default:
        {
            assert(false);
        }
    }
}

Variant to_variant(middleware::ValueBasePtr const& v)
{
    switch (v->type)
    {
        case middleware::IntType:
        {
            middleware::IntValuePtr p = middleware::IntValuePtr::dynamicCast(v);
            assert(p);
            return Variant(p->value);
        }
        case middleware::BoolType:
        {
            middleware::BoolValuePtr p = middleware::BoolValuePtr::dynamicCast(v);
            assert(p);
            return Variant(p->value);
        }
        case middleware::StringType:
        {
            middleware::StringValuePtr p = middleware::StringValuePtr::dynamicCast(v);
            assert(p);
            return Variant(p->value);
        }
        default:
        {
            assert(false);
        }
    }
}

middleware::ValueDict to_value_dict(VariantMap const& vm)
{
    middleware::ValueDict vd;
    for (auto v : vm)
    {
        vd[v.first] = to_value(v.second);
    }
    return vd;
}

VariantMap to_variant_map(middleware::ValueDict const& vd)
{
    VariantMap vm;
    for (auto v : vd)
    {
        vm[v.first] = to_variant(v.second);
    }
    return vm;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
