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
#include <unity/scopes/internal/Utils.h>

namespace unity
{

namespace scopes
{

namespace internal
{

QueryMetadataImpl::QueryMetadataImpl(std::string const& locale, std::string const& form_factor)
    : locale_(locale),
      form_factor_(form_factor)
{
}

QueryMetadataImpl::QueryMetadataImpl(VariantMap const& var)
{
    static const std::string context("QueryMetadataImpl()");
    auto it = find_or_throw(context, var, "locale");
    locale_ = it->second.get_string();
    it = find_or_throw(context, var, "form_factor");
    form_factor_ = it->second.get_string();
}

std::string QueryMetadataImpl::locale() const
{
    return locale_;
}

std::string QueryMetadataImpl::form_factor() const
{
    return form_factor_;
}

void QueryMetadataImpl::serialize(VariantMap &var) const
{
    var["type"] = metadata_type();
    var["locale"] = locale_;
    var["form_factor"] = form_factor_;
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
