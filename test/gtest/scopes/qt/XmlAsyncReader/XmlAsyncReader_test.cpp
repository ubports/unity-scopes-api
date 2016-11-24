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

#include <unity/scopes/qt/XmlAsyncReader.h>
#include <unity/scopes/qt/XmlReader.h>
#include <unity/scopes/ScopeExceptions.h>

#include <QtCore/QXmlStreamReader>
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
protected:
    virtual void SetUp()
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

// std::string fake_server_uri = getServerAddress();

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

        Result(QXmlStreamReader& xml)
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::Characters)
            {
                token = xml.readNext();
            }
            if (xml.name() == "title")
            {
                token = xml.readNext();
                title = xml.text().toString().toStdString();
            }
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

TEST(TimeoutException, state)
{
    XmlAsyncReader reader;
    auto reader_future = reader.async_get<Client::Result, Client::Result>(
        "http://ws.audioscrobbler.com/1.0/user/test/recenttracks.rss", "item");
    // do your stuff
    EXPECT_THROW(HttpAsyncReader::get_or_throw(reader_future, 0), unity::scopes::TimeoutException);
}

TEST(BadUrlException, state)
{
    XmlAsyncReader reader;
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
    catch (std::exception const& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(ExceptionsTest, user_results_ok)
{
    XmlReader reader;

    XmlReader::QXmlStreamReaderParameters parameters{{"method", "xml_chart_gettoptracks"},
                                                     {"api_key", "1cf69754d419139d08d42f307d060fff"}};

    XmlReader::QXmlStreamReaderSptr xml = reader.read(QString(fake_server_host.c_str()), parameters);

    // count the number of items read
    int number_items = 0;
    // do your stuff
    while (!xml->atEnd() && !xml->hasError())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml->readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement && xml->name() == "track")
        {
            ++number_items;
        }
    }

    // check that we've got 5 valid results
    EXPECT_EQ(number_items, 5);
}

TEST_F(ExceptionsTest, check_xml_bad_formed)
{
    XmlReader reader;

    XmlReader::QXmlStreamReaderParameters parameters{{"method", "xml_bad_formed_chart_gettoptracks"},
                                                     {"api_key", "1cf69754d419139d08d42f307d060fff"}};

    XmlReader::QXmlStreamReaderSptr xml = reader.read(QString(fake_server_host.c_str()), parameters);

    // count the number of items read
    int number_items = 0;

    // do your stuff
    while (!xml->atEnd() && !xml->hasError())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml->readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement && xml->name() == "track")
        {
            ++number_items;
        }
    }

    // The XML has errors
    std::cout << xml->errorString().toStdString() << std::endl;
    EXPECT_EQ(xml->hasError(), true);
}

TEST_F(ExceptionsTest, check_xml_bad_formed_exception)
{
    XmlAsyncReader reader;

    XmlAsyncReader::QXmlStreamReaderParams parameters{{"method", "xml_bad_formed_chart_gettoptracks"},
                                                      {"api_key", "1cf69754d419139d08d42f307d060fff"}};
    try
    {
        auto reader_future = reader.async_get<Client::Result>(fake_server_host.c_str(), parameters, "track");
        auto items = HttpAsyncReader::get_or_throw(reader_future, 10);
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_STREQ(e.what(),
                     "unity::LogicException: AsyncReader::async_get: error parsing data: get_results: ERROR: Opening "
                     "and ending tag mismatch. at line: 10");
    }
}
