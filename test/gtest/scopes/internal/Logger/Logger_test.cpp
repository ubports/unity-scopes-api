/*
 * Copyright (C) 2016 Canonical Ltd
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

#include <unity/scopes/internal/Logger.h>

#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::scopes::internal;

TEST(Logger, basic)
{
    {
        ostringstream s;
        Logger l("me", s);
        l() << "";
        EXPECT_TRUE(s.str().empty());
    }

    {
        ostringstream s;
        Logger l("me", s);
        bool v = false;
        l() << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 0\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        char const* v = "cs";
        l() << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: cs\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        string v = "s";
        l() << v << " " << 5 << "xyz";
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: s 5xyz\n")) << s.str();
    }
}

TEST(Logger, severity)
{
    {
        ostringstream s;
        Logger l("me", s);

        l.set_severity_threshold(LoggerSeverity::Fatal);
        l() << "blah blah";
        EXPECT_TRUE(s.str().empty()) << s.str();

        l(LoggerSeverity::Error) << "hello";
        EXPECT_TRUE(s.str().empty()) << s.str();

        l(LoggerSeverity::Fatal) << "world";
        EXPECT_TRUE(boost::ends_with(s.str(), "] FATAL: me: world\n")) << s.str();
    }
}

TEST(Logger, channel)
{
    {
        ostringstream s;
        Logger l("me", s);

        l(LoggerChannel::IPC) << "hi";
        EXPECT_TRUE(s.str().empty()) << s.str();

        EXPECT_FALSE(l.set_channel("IPC", true));
        l(LoggerChannel::IPC) << "there";
        EXPECT_TRUE(boost::ends_with(s.str(), "] IPC: me: there\n")) << s.str();

        EXPECT_TRUE(l.set_channel("IPC", false));
        l(LoggerChannel::IPC) << "y";
        EXPECT_TRUE(boost::ends_with(s.str(), "] IPC: me: there\n")) << s.str();

        EXPECT_FALSE(l.set_channel("IPC", true));
        l(LoggerChannel::IPC) << "x";
        EXPECT_TRUE(boost::ends_with(s.str(), "] IPC: me: x\n")) << s.str();
    }
}

TEST(Logger, bad_channel)
{
    {
        ostringstream s;
        Logger l("me", s);

        try
        {
            l.set_channel("", true);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: Logger::set_channel(): invalid channel name: \"\"", e.what());
        }

        try
        {
            l.set_channel("xyz", true);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: Logger::set_channel(): invalid channel name: \"xyz\"", e.what());
        }
    }
}
