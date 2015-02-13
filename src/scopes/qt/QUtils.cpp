/*
 * Copyright (C) 2015 Canonical Ltd
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

#include <unity/scopes/qt/QUtils.h>

#include <cassert>

using namespace unity::scopes::qt;

namespace sc = unity::scopes;
namespace qt = unity::scopes::qt;

namespace unity
{

namespace scopes
{

namespace qt
{

QVariant scopeVariantToQVariant(sc::Variant const& variant)
{
    switch (variant.which())
    {
        case sc::Variant::Type::Null:
            return QVariant();
        case sc::Variant::Type::Int:
            return QVariant(variant.get_int());
        case sc::Variant::Type::Bool:
            return QVariant(variant.get_bool());
        case sc::Variant::Type::String:
            return QVariant(QString::fromStdString(variant.get_string()));
        case sc::Variant::Type::Double:
            return QVariant(variant.get_double());
        case sc::Variant::Type::Dict:
        {
            sc::VariantMap dict(variant.get_dict());
            QVariantMap result_dict;
            for (auto it = dict.begin(); it != dict.end(); ++it)
            {
                result_dict.insert(QString::fromStdString(it->first), qt::scopeVariantToQVariant(it->second));
            }
            return result_dict;
        }
        case sc::Variant::Type::Array:
        {
            sc::VariantArray arr(variant.get_array());
            QVariantList result_list;
            for (unsigned i = 0; i < arr.size(); i++)
            {
                result_list.append(qt::scopeVariantToQVariant(arr[i]));
            }
            return result_list;
        }
        default:
        {
            assert(false);
            return QVariant();
        }
    }
}

sc::Variant qVariantToScopeVariant(QVariant const& variant)
{
    if (variant.isNull())
    {
        return sc::Variant();
    }

    switch (variant.type())
    {
        case QMetaType::Bool:
            return sc::Variant(variant.toBool());
        case QMetaType::Int:
            return sc::Variant(variant.toInt());
        case QMetaType::Double:
            return sc::Variant(variant.toDouble());
        case QMetaType::QString:
            return sc::Variant(variant.toString().toStdString());
        case QMetaType::QVariantMap:
        {
            sc::VariantMap vm;
            QVariantMap m(variant.toMap());
            for (auto it = m.begin(); it != m.end(); ++it)
            {
                vm[it.key().toStdString()] = qt::qVariantToScopeVariant(it.value());
            }
            return sc::Variant(vm);
        }
        case QMetaType::QVariantList:
        {
            QVariantList l(variant.toList());
            sc::VariantArray arr;
            for (int i = 0; i < l.size(); i++)
            {
                arr.push_back(qt::qVariantToScopeVariant(l[i]));
            }
            return sc::Variant(arr);
        }
        default:
        {
            // TODO: better to throw than to return an invalid value?
            qWarning("Unhandled QVariant type: %s", variant.typeName());
            return sc::Variant();
        }
    }
}

QVariantMap scopeVariantMapToQVariantMap(unity::scopes::VariantMap const& variant)
{
    QVariantMap ret_map;
    for (auto item : variant)
    {
        ret_map[QString::fromUtf8(item.first.c_str())] = qt::scopeVariantToQVariant(item.second);
    }

    return ret_map;
}

VariantMap qVariantMapToScopeVariantMap(QVariantMap const& variant)
{
    VariantMap ret_map;
    QMapIterator<QString, QVariant> it(variant);
    while (it.hasNext())
    {
        ret_map[it.key().toUtf8().data()] = qt::qVariantToScopeVariant(it.value());
    }

    return ret_map;
}

}  // namespace qt

}  // namespace scopes

}  // namespace unity
