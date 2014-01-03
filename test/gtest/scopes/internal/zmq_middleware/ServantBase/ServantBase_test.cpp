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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/internal/zmq_middleware/ServantBase.h>

#include <internal/zmq_middleware/capnproto/Message.capnp.h>
#include <scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

#include <cassert>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;
using namespace unity::api::scopes::internal::zmq_middleware;

// Test servant that throws the exception passed to the constructor from the op() method.

class MyDelegate : public AbstractObject
{
};

using namespace std::placeholders;

class MyServant : public ServantBase
{
public:
    enum ThrowType { NoException, UnityException, StdException, OtherException };

    MyServant(ThrowType t) :
        ServantBase(make_shared<MyDelegate>(), { { "op", bind(&MyServant::op, this, _1, _2, _3) } }),
        t_(t)
    {
    }

    virtual void op(Current const&,
                    capnp::ObjectPointer::Reader&,
                    capnproto::Response::Builder& r)
    {

        EXPECT_NE(nullptr, del().get());  // Just so we get coverage
        switch (t_)
        {
            case NoException:
            {
                r.setStatus(capnproto::ResponseStatus::SUCCESS);
                break;
            }
            case UnityException:
            {
                throw SyscallException("system call blew up", 99);
                break;
            }
            case StdException:
            {
                throw std::bad_alloc();
                break;
            }
            case OtherException:
            {
                throw 42;
                break;
            }
            default:
            {
                assert(false);
            }
        }
    }

private:
    ThrowType t_;
};

TEST(ServantBase, success)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ServantBase/Zmq.ini",
                     (RuntimeImpl*)0x1);
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    Current current;
    current.op_name = "op";
    current.adapter = &a;   // Dummy needed for the throw code to get at the adapter name and endpoint.

    MyServant s(MyServant::NoException);
    capnp::MallocMessageBuilder b;
    auto response = b.initRoot<capnproto::Response>();
    capnp::ObjectPointer::Reader in_params;
    s.safe_dispatch_(current, in_params, response);
    EXPECT_EQ(capnproto::ResponseStatus::SUCCESS, response.getStatus());
}

TEST(ServantBase, ping)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ServantBase/Zmq.ini",
                     (RuntimeImpl*)0x1);
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    Current current;
    current.op_name = "ping";
    current.adapter = &a;   // Dummy needed for the throw code to get at the adapter name and endpoint.

    MyServant s(MyServant::NoException);
    capnp::MallocMessageBuilder b;
    auto response = b.initRoot<capnproto::Response>();
    capnp::ObjectPointer::Reader in_params;
    s.safe_dispatch_(current, in_params, response);
    EXPECT_EQ(capnproto::ResponseStatus::SUCCESS, response.getStatus());
}

TEST(ServantBase, exceptions)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ServantBase/Zmq.ini",
                     (RuntimeImpl*)0x1);
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    Current current;
    current.op_name = "op";
    current.adapter = &a;   // Dummy needed for the throw code to get at the adapter name and endpoint.

    {
        MyServant s(MyServant::UnityException);
        capnp::MallocMessageBuilder b;
        auto response = b.initRoot<capnproto::Response>();
        capnp::ObjectPointer::Reader in_params;
        s.safe_dispatch_(current, in_params, response);
        EXPECT_EQ(capnproto::ResponseStatus::RUNTIME_EXCEPTION, response.getStatus());
        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
        EXPECT_STREQ("unity::SyscallException: system call blew up (errno = 99)", ex.getUnknown().cStr());
    }

    {
        MyServant s(MyServant::StdException);
        capnp::MallocMessageBuilder b;
        auto response = b.initRoot<capnproto::Response>();
        capnp::ObjectPointer::Reader in_params;
        s.safe_dispatch_(current, in_params, response);
        EXPECT_EQ(capnproto::ResponseStatus::RUNTIME_EXCEPTION, response.getStatus());
        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
        EXPECT_STREQ("std::bad_alloc", ex.getUnknown().cStr());
    }

    {
        MyServant s(MyServant::OtherException);
        capnp::MallocMessageBuilder b;
        auto response = b.initRoot<capnproto::Response>();
        capnp::ObjectPointer::Reader in_params;
        s.safe_dispatch_(current, in_params, response);
        EXPECT_EQ(capnproto::ResponseStatus::RUNTIME_EXCEPTION, response.getStatus());
        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
        EXPECT_STREQ("unknown exception", ex.getUnknown().cStr());
    }

    {
        // Servant doesn't raise an exception, but invocation contains non-existent operation.
        MyServant s(MyServant::NoException);
        capnp::MallocMessageBuilder b;
        auto response = b.initRoot<capnproto::Response>();
        capnp::ObjectPointer::Reader in_params;
        current.op_name = "no_such_op";
        s.safe_dispatch_(current, in_params, response);
        EXPECT_EQ(capnproto::ResponseStatus::RUNTIME_EXCEPTION, response.getStatus());
        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::OPERATION_NOT_EXIST, ex.which());
        EXPECT_STREQ("no_such_op", ex.getOperationNotExist().getOpName().cStr());
    }
}
