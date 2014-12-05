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
#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>

#include <memory>
#include <future>
#include "HttpAsyncReader.h"

namespace unity
{

namespace scopes
{

namespace utils
{

namespace qt
{

template <typename TYPE>
using ResultsList = std::deque<std::shared_ptr<TYPE>>;

template <typename B, typename T>
static bool get_results_json(QJsonDocument& json,
                             std::string const& object_name,
                             ResultsList<B>& results,
                             std::string&);

class JsonAsyncReader
{
public:
    NONCOPYABLE(JsonAsyncReader);
    UNITY_DEFINES_PTRS(JsonAsyncReader);

    template <typename B>
    using ParseFunc =
        std::function<bool(QJsonDocument& root, const std::string&, std::deque<std::shared_ptr<B>>&, std::string&)>;

    template <typename T>
    using ResultsFuture = std::future<std::deque<std::shared_ptr<T>>>;

    template <typename TYPE>
    using ResultsList = std::deque<std::shared_ptr<TYPE>>;

    typedef std::future<std::shared_ptr<QJsonDocument>> JsonDocumentFuture;

    typedef std::shared_ptr<QJsonDocument> QJsonDocumentSptr;

    typedef std::vector<std::pair<std::string, std::string>> JsonParameters;

    JsonAsyncReader();

    virtual ~JsonAsyncReader() = default;

    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  ParseFunc<BASE> const& parse = get_results_json<BASE, TYPE>) const;

    template <typename T>
    ResultsFuture<T> async_get(std::string const& uri,
                               std::string const& object_name,
                               ParseFunc<T> const& parse = get_results_json<T, T>) const;

    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& host,
                                  JsonParameters const& params,
                                  std::string const& object_name,
                                  ParseFunc<BASE> const& parse = get_results_json<BASE, TYPE>) const;

    template <typename T>
    ResultsFuture<T> async_get(std::string const& host,
                               JsonParameters const& params,
                               std::string const& object_name,
                               ParseFunc<T> const& parse = get_results_json<T, T>) const;

    JsonDocumentFuture async_get_parser(std::string const& uri) const;

    JsonDocumentFuture async_get_parser(std::string const& host, JsonParameters const& params) const;

protected:
    static QJsonDocumentSptr create_parser_with_data(std::string const& data, std::string& error);

    std::shared_ptr<HttpAsyncReader> p_;
};

template <typename BASE, typename TYPE>
JsonAsyncReader::ResultsFuture<BASE> JsonAsyncReader::async_get(std::string const& uri,
                                                                std::string const& object_name,
                                                                ParseFunc<BASE> const& parse) const
{
    return p_->async_get<BASE, TYPE, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename T>
JsonAsyncReader::ResultsFuture<T> JsonAsyncReader::async_get(std::string const& uri,
                                                             std::string const& object_name,
                                                             ParseFunc<T> const& parse) const
{
    return p_->async_get<T, T, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename BASE, typename TYPE>
JsonAsyncReader::ResultsFuture<BASE> JsonAsyncReader::async_get(std::string const& host,
                                                                JsonParameters const& params,
                                                                std::string const& object_name,
                                                                ParseFunc<BASE> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<BASE, TYPE, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename T>
JsonAsyncReader::ResultsFuture<T> JsonAsyncReader::async_get(std::string const& host,
                                                             JsonParameters const& params,
                                                             std::string const& object_name,
                                                             ParseFunc<T> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<T, T, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename B, typename T>
static bool get_results_json_object(QJsonObject& root,
                                    const std::string& object_name,
                                    JsonAsyncReader::ResultsList<B>& results,
                                    std::string& error_string)
{
    QString qobject_name(object_name.c_str());
    if (root.contains(qobject_name))
    {
        if (root[qobject_name].isArray())
        {
            auto sub_root = root[qobject_name];
            QJsonArray levelArray = sub_root.toArray();
            for (int levelIndex = 0; levelIndex < levelArray.size(); ++levelIndex)
            {
                QJsonObject levelObject = levelArray[levelIndex].toObject();
                results.emplace_back(std::make_shared<T>(levelObject));
            }
        }
        else if (root[qobject_name].isObject())
        {
            auto sub_root = root[qobject_name].toObject();
            results.emplace_back(std::make_shared<T>(sub_root));
        }
    }
    else
    {
        // iterate thow the existing members
        for (auto member : root.keys())
        {
            auto item = root[member];
            if (item.isObject())
            {
                QJsonObject obj = item.toObject();
                get_results_json_object<B, T>(obj, object_name, results, error_string);
            }
        }
    }
    return true;
}

template <typename B, typename T>
static bool get_results_json(QJsonDocument& doc,
                             const std::string& object_name,
                             JsonAsyncReader::ResultsList<B>& results,
                             std::string& error_string)
{
    if (doc.isArray())
    {
        QJsonArray levelArray = doc.array();
        for (int levelIndex = 0; levelIndex < levelArray.size(); ++levelIndex)
        {
            QJsonObject levelObject = levelArray[levelIndex].toObject();
            get_results_json_object<B, T>(levelObject, object_name, results, error_string);
        }
    }
    else if (doc.isObject())
    {
        QJsonObject levelObject = doc.object();
        get_results_json_object<B, T>(levelObject, object_name, results, error_string);
    }
    return true;
}

}  // namespace qt

}  // namespace utils

}  // namespace scopes

}  // namespace unity
