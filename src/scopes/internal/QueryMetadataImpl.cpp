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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/internal/QueryMetadataImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/internal/Utils.h>

namespace unity
{

namespace scopes
{

namespace internal
{

QueryMetadataImpl::QueryMetadataImpl(std::string const& locale, std::string const& form_factor)
    : locale_(locale),
      form_factor_(form_factor),
      internet_connectivity_(QueryMetadata::Unknown)
{
}

QueryMetadataImpl::QueryMetadataImpl(VariantMap const& var)
{
    static const std::string context("QueryMetadataImpl()");
    auto it = find_or_throw(context, var, "locale");
    locale_ = it->second.get_string();
    it = find_or_throw(context, var, "form_factor");
    form_factor_ = it->second.get_string();
    it = find_or_throw("QueryMetadataImpl()", var, "hints");
    hints_ = it->second.get_dict();
    it = var.find("internet_connectivity");
    if (it != var.end())
    {
        internet_connectivity_ = it->second.get_bool() ? QueryMetadata::Connected : QueryMetadata::Disconnected;
    }
    else
    {
        internet_connectivity_ = QueryMetadata::Unknown;
    }
}

std::string QueryMetadataImpl::locale() const
{
    return locale_;
}

std::string QueryMetadataImpl::form_factor() const
{
    return form_factor_;
}

void QueryMetadataImpl::set_internet_connectivity(QueryMetadata::ConnectivityStatus connectivity_status)
{
    internet_connectivity_ = connectivity_status;
}

QueryMetadata::ConnectivityStatus QueryMetadataImpl::internet_connectivity() const
{
    return internet_connectivity_;
}

Variant& QueryMetadataImpl::hint(std::string const& key)
{
    if (key.empty())
    {
        throw InvalidArgumentException("QueryMetadata::hint(): Invalid empty key string");
    }
    return hints_[key];
}

Variant const& QueryMetadataImpl::hint(std::string const& key) const
{
    if (key.empty())
    {
        throw InvalidArgumentException("QueryMetadata::hint(): Invalid empty key string");
    }

    auto const& it = hints_.find(key);
    if (it != hints_.end())
    {
        return it->second;
    }
    std::ostringstream s;
    s << "QueryMetadataImpl::hint(): requested key " << key << " doesn't exist";
    throw unity::LogicException(s.str());
}

void QueryMetadataImpl::set_hint(std::string const& key, Variant const& value)
{
    if (key.empty())
    {
        throw InvalidArgumentException("QueryMetadata::set_hint(): Invalid empty key string");
    }
    hints_[key] = value;
}

VariantMap QueryMetadataImpl::hints() const
{
    return hints_;
}

bool QueryMetadataImpl::contains_hint(std::string const& key) const
{
    if (key.empty())
    {
        throw InvalidArgumentException("QueryMetadata::contains_hint(): Invalid empty key string");
    }
    return hints_.find(key) != hints_.end();
}

void QueryMetadataImpl::serialize(VariantMap& var) const
{
    var["type"] = metadata_type();
    var["locale"] = locale_;
    var["form_factor"] = form_factor_;
    var["hints"] = hints_;
    if (internet_connectivity_ != QueryMetadata::Unknown)
    {
        var["internet_connectivity"] = internet_connectivity_ == QueryMetadata::Connected;
    }
}

VariantMap QueryMetadataImpl::serialize() const
{
    VariantMap vm;
    serialize(vm);
    return vm;
}

std::string QueryMetadataImpl::metadata_type() const
{
    static const std::string t("query_metadata");
    return t;
}

} // namespace internal

} // namespace scopes

} // namespace unity
