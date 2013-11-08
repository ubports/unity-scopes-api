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

#include <scopes/internal/zmq_middleware/VariantConverter.h>
#include <internal/zmq_middleware/capnproto/ValueDict.capnp.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <capnp/message.h>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal::zmq_middleware;

// This test checks conversions between Variant/VariantMap and Value/ValueDict (capnproto)
// performed by VariantConverter utility functions (to_value, to_variant).
TEST(VariantConverter, basic)
{
    // convert a Variant into capnproto Value
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<unity::api::scopes::internal::zmq_middleware::capnproto::Value>();
    {
        VariantMap m;
        VariantMap inner;
        inner["iron"] = Variant("maiden");
        m["hints"] = inner;
        m["foo"] = Variant(1);
        m["bar"] = Variant(true);
        Variant v(m);

        to_value(v, builder);
    }

    // convert capnproto Value back to Variant
    auto reader = message.getRoot<unity::api::scopes::internal::zmq_middleware::capnproto::Value>();
    EXPECT_EQ(capnproto::Value::Which::DICT_VAL, reader.which());

    auto resultVar = to_variant(reader);
    EXPECT_EQ(Variant::Dict, resultVar.which());
    auto outerDict = resultVar.get_dict();
    EXPECT_EQ(1, outerDict["foo"].get_int());
    EXPECT_EQ(true, outerDict["bar"].get_bool());
    auto innerDict = outerDict["hints"].get_dict();
    EXPECT_EQ("maiden", innerDict["iron"].get_string());
}
