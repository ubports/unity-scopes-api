/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */


#include <unity/scopes/qt/JsonAsyncReader.h>
#include <unity/scopes/qt/JsonReader.h>
#include <unity/scopes/ScopeExceptions.h>

#include <QtCore/QJsonObject>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <core/posix/exec.h>

namespace posix = core::posix;

using namespace std;
using namespace unity::scopes::qt;

class ExceptionsTest : public ::testing::Test
{
protected: virtual void SetUp()
    {
        fake_server_ = posix::exec(FAKE_SERVER, {}, {}, posix::StandardStream::stdout);

        ASSERT_GT(fake_server_.pid(), 0);
        string port;
        fake_server_.cout() >> port;

        fake_server_host = "http://127.0.0.1:" + port;
    }

    posix::ChildProcess fake_server_ = posix::ChildProcess::invalid();
    std::string fake_server_host;
};

struct Client
{
public:
    struct Result
    {
        std::string uri;
        std::string title;
        std::string id;

        Result()
        {
        }

        Result(QJsonObject& json)
        {
            if (json.contains("name"))
            {
                title = json["name"].toString().toStdString();
            }
        }
    };

    /**
     * A list of weather information
     */
    typedef std::deque<Result> ResultList;
};

TEST_F(ExceptionsTest, timeout_test)
{
    JsonAsyncReader reader;
    JsonAsyncReader::JsonParameters parameters{
        {"method", "json_chart_gettoptracks"}, {"api_key", "1cf69754d419139d08d42f307d060fff"}, {"format", "json"}};

    auto reader_future = reader.async_get<Client::Result>(fake_server_host, parameters, "track");
    // do your stuff
    EXPECT_THROW(HttpAsyncReader::get_or_throw(reader_future, 0), unity::scopes::TimeoutException);
}

TEST_F(ExceptionsTest, bad_url_test)
{
    JsonAsyncReader reader;
    JsonAsyncReader::JsonParameters parameters{
        {"method", "json_not_found"}, {"api_key", "1cf69754d419139d08d42f307d060fff"}, {"format", "json"}};
    try
    {
        auto reader_future = reader.async_get<Client::Result>(fake_server_host, parameters, "track");
        // do your stuff
        auto items = HttpAsyncReader::get_or_throw(reader_future);
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        std::string error_msg = e.what();
        EXPECT_EQ(error_msg.find("<title>404 Not Found</title>") != std::string::npos, true);
    }
    catch (std::exception const& e)
    {
        FAIL() << e.what();
    }
}
TEST(BadUrlException, state)
{
    JsonAsyncReader reader;
    try
    {
        auto reader_future =
            reader.async_get<Client::Result, Client::Result>("http://bad_host_name.com/test.xml", "item");
        // do your stuff
        auto items = HttpAsyncReader::get_or_throw(reader_future);
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        std::string error_msg = e.what();
        EXPECT_EQ(error_msg.find("Couldn't resolve host name") != std::string::npos, true);
    }
    catch (unity::scopes::TimeoutException const&)
    {
        // This can happen occasionally when DNS is slow
    }
    catch (std::exception const& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(ExceptionsTest, user_results_ok)
{
    JsonReader reader;

    JsonReader::QJsonParameters parameters{
        {"method", "json_chart_gettoptracks"}, {"api_key", "1cf69754d419139d08d42f307d060fff"}, {"format", "json"}};

    JsonReader::QJsonDocumentSptr json = reader.read(QString(fake_server_host.c_str()), parameters);

    // do your stuff
    std::deque<Client::Result> results;
    QJsonObject root = json->object();
    if (root.contains("tracks"))
    {
        auto sub_root = root["tracks"].toObject();
        if (sub_root.contains("track"))
        {
            auto tracks = sub_root["track"].toArray();
            for (int levelIndex = 0; levelIndex < tracks.size(); ++levelIndex)
            {
                QJsonObject levelObject = tracks[levelIndex].toObject();
                results.emplace_back(Client::Result(levelObject));
            }
        }
    }

    // check that we've got 5 valid results
    EXPECT_EQ(5u, results.size());
}

TEST_F(ExceptionsTest, check_xml_bad_formed)
{
    JsonReader reader;

    JsonReader::QJsonParameters parameters{{"method", "xml_bad_formed_chart_gettoptracks"},
                                           {"api_key", "1cf69754d419139d08d42f307d060fff"}};

    try
    {
        JsonReader::QJsonDocumentSptr doc = reader.read(QString(fake_server_host.c_str()), parameters);
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        std::string error_msg = e.what();
        EXPECT_EQ(error_msg.find("error obtaining parser: JsonAsyncReader::create_parser_with_data: illegal value") !=
                      std::string::npos,
                  true);
    }
    catch (std::exception const& e)
    {
        FAIL() << e.what();
    }
}
