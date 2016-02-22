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
        bool v = false;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 0\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        short v = -1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: -1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned short v = 1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        int v = -1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: -1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned int v = 1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        long v = -1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: -1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned long v = 1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        long long v = -1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: -1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned long long v = 1;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 1\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        float v = 2.0;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 2\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        double v = 3.0;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 3\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        long double v = 4.0;
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 4\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        char v = 'a';
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: a\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        signed char v = 'b';
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: b\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned char v = 'c';
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: c\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        char const* v = "cs";
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: cs\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        signed char const v[] = { 's', 'c', 's', '\0' };
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: scs\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        unsigned char const v[] = { 'u', 'c', 's', '\0' };
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: ucs\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        string v = "s";
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: s\n")) << s.str();
    }

    {
        ostringstream s;
        Logger l("me", s);
        void* v = reinterpret_cast<void*>(0x99);
        l << v;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: 0x99\n")) << s.str();
    }

    // TODO: missing two overloads here.
    {
        ostringstream s;
        Logger l("me", s);
        l << hex << 15;
        EXPECT_TRUE(boost::ends_with(s.str(), "] ERROR: me: f\n")) << s.str();
    }
}

TEST(Logger, severity)
{
    {
        ostringstream s;
        Logger l("me", s);

        l.set_severity_threshold(LoggerSeverity::Fatal);
        l << "blah blah";
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

        vector<string> channels{ "IPC" };
        l.enable_channels(channels);
        l(LoggerChannel::IPC) << "x";
        EXPECT_TRUE(boost::ends_with(s.str(), "] IPC: me: x\n")) << s.str();
    }
}

TEST(Logger, bad_channel)
{
    {
        ostringstream s;
        Logger l("me", s);

        vector<string> channels{ "", "IPC", "wrong" };
        try
        {
            l.enable_channels(channels);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("", e.what());
        }
        catch (std::exception const& e)
        {
            EXPECT_STREQ("", e.what());
        }
        catch (...)
        {
            FAIL();
        }

#if 0
        l(LoggerChannel::IPC) << "hi";
        EXPECT_TRUE(s.str().empty()) << s.str();

        EXPECT_FALSE(l.set_channel("IPC", true));
        l(LoggerChannel::IPC) << "there";
        EXPECT_TRUE(boost::ends_with(s.str(), "] IPC: me: there\n")) << s.str();
#endif
    }
}
