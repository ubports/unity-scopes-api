/* Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/internal/LinkImpl.h>
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

LinkImpl::LinkImpl(std::string const& label, CannedQuery const& query)
    : label_(label),
      query_(std::make_shared<CannedQuery>(query))
{
}

LinkImpl::LinkImpl(VariantMap const& variant_map)
{
    auto it = variant_map.find("label");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Invalid variant, 'label' is missing");
    }
    label_ = it->second.get_string();

    it = variant_map.find("query");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Invalid variant, 'query' is missing");
    }

    query_.reset(new CannedQuery(internal::CannedQueryImpl::create(it->second.get_dict())));
}

std::string LinkImpl::label() const
{
    return label_;
}

CannedQuery LinkImpl::query() const
{
    return *query_;
}

VariantMap LinkImpl::serialize() const
{
    VariantMap vm;
    vm["label"] = label_;
    vm["query"] = query_->serialize();
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace unity
