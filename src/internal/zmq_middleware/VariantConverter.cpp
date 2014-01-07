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

#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

void to_value(Variant const& v, capnproto::Value::Builder& b)
{
    switch (v.which())
    {
        case Variant::Int:
        {
            b.setIntVal(v.get_int());
            break;
        }
        case Variant::Double:
        {
            b.setDoubleVal(v.get_double());
            break;
        }
        case Variant::Bool:
        {
            b.setBoolVal(v.get_bool());
            break;
        }
        case Variant::String:
        {
            b.setStringVal(v.get_string().c_str());
            break;
        }
        case Variant::Dict:
        {
            auto vb = b.initDictVal();
            to_value_dict(v.get_dict(), vb);
            break;
        }
        case Variant::Array:
        {
            auto vb = b.initArrayVal(v.get_array().size());
            to_value_array(v.get_array(), vb);
            break;
        }
        case Variant::Null:
        {
            b.initNullVal();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

Variant to_variant(capnproto::Value::Reader const& r)
{
    switch (r.which())
    {
        case capnproto::Value::INT_VAL:
        {
            return Variant(r.getIntVal());
        }
        case capnproto::Value::DOUBLE_VAL:
        {
            return Variant(r.getDoubleVal());
        }
        case capnproto::Value::BOOL_VAL:
        {
            return Variant(r.getBoolVal());
        }
        case capnproto::Value::STRING_VAL:
        {
            return Variant(r.getStringVal().cStr());
        }
        case capnproto::Value::DICT_VAL:
        {
            return Variant(to_variant_map(r.getDictVal()));
        }
        case capnproto::Value::ARRAY_VAL:
        {
            return Variant(to_variant_array(r.getArrayVal()));
        }
        case capnproto::Value::NULL_VAL:
        {
            return Variant::null();
        }
        default:
        {
            assert(false);           // LCOV_EXCL_LINE
        }
    }
    return Variant::null();  // LCOV_EXCL_LINE (stop compiler warning about not returning a value)
}

void to_value_dict(VariantMap const& vm, capnproto::ValueDict::Builder& b)
{
    auto dict = b.initPairs(vm.size());
    int i = 0;
    for (auto const& pair : vm)
    {
        dict[i].setName(pair.first.c_str());
        auto val = dict[i].initValue();
        to_value(pair.second, val);
        ++i;
    }
}

void to_value_array(VariantArray const& va, capnp::List<capnproto::Value>::Builder& b)
{
    assert(va.size() == b.size());
    uint i = 0;
    for (auto const& el: va)
    {
        auto elm_bld = b[i];
        to_value(el, elm_bld);
        ++i;
    }
}

VariantMap to_variant_map(capnproto::ValueDict::Reader const& r)
{
    VariantMap vm;
    auto pairs = r.getPairs();
    for (size_t i = 0; i < pairs.size(); ++i)
    {
        vm[pairs[i].getName().cStr()] = to_variant(pairs[i].getValue());
    }
    return vm;
}

VariantArray to_variant_array(capnp::List<capnproto::Value>::Reader const &r)
{
    VariantArray va;
    for (auto const& el: r)
    {
        va.push_back(to_variant(el));
    }

    return va;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
