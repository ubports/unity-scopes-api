/*
* Copyright (C) 2014 Canonical, Ltd.
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of version 3 of the GNU Lesser General Public License as published
* by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Author: Xavi Garcia <xavi.garcia.mena@canonical.com>
*/
#include "../../../include/unity/scopes/scopes-qt/JsonAsyncReader.h"

using namespace std;
using namespace unity::scopes::utils::qt;

JsonAsyncReader::JsonAsyncReader()
    : p_(new HttpAsyncReader)
{
}

JsonAsyncReader::QJsonDocumentSptr JsonAsyncReader::create_parser_with_data(std::string const& data, std::string& error)
{
    shared_ptr<QJsonDocument> res;

    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray(data.c_str()), &json_error);
    if (doc.isNull())
    {
        error = "JsonAsyncReader::create_parser_with_data: " + json_error.errorString().toStdString();
    }
    else
    {
        QJsonDocument* obj = new QJsonDocument(doc);
        res.reset(obj);
    }
    return res;
}

JsonAsyncReader::JsonDocumentFuture JsonAsyncReader::async_get_parser(std::string const& uri) const
{
    return p_->async_get_parser<QJsonDocument>(uri, JsonAsyncReader::create_parser_with_data);
}

JsonAsyncReader::JsonDocumentFuture JsonAsyncReader::async_get_parser(
    std::string const& host, JsonAsyncReader::JsonParameters const& params) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get_parser<QJsonDocument>(uri, JsonAsyncReader::create_parser_with_data);
}
