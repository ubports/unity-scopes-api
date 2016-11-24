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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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

TEST(Logger, move)
{
    // Logger move constructor.
    {
        ostringstream s;

        Logger l("me", s);
        l.set_channel("IPC", true);
        l.set_severity_threshold(LoggerSeverity::Warning);
        l(LoggerSeverity::Warning) << "hi";
        EXPECT_TRUE(boost::ends_with(s.str(), "] WARNING: me: hi\n")) << s.str();

        Logger l2(move(l));
        EXPECT_TRUE(l2.set_channel("IPC", false));
        l(LoggerSeverity::Warning) << "again";
        EXPECT_TRUE(boost::ends_with(s.str(), "] WARNING: me: again\n")) << s.str();
        EXPECT_EQ(LoggerSeverity::Warning, l2.set_severity_threshold(LoggerSeverity::Error));
    }

    // LogStream move constructor.
    {
        ostringstream s;

        Logger l("me", s);
        {
            LogStream ls(l());
            ls << "added";
            EXPECT_TRUE(s.str().empty());

            LogStream ls2(move(ls));
            EXPECT_TRUE(s.str().empty());
            ls2 << " blah";
            EXPECT_TRUE(s.str().empty());
        }
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: added blah\n")) << s.str();
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
