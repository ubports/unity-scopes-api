/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/internal/ActivationResponseImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

ActivationResponseImpl::ActivationResponseImpl(ActivationResponse::Status status)
    : status_(status)
{
    if (status == ActivationResponse::Status::PerformQuery)
    {
        throw unity::InvalidArgumentException("ActivationResponse(): Status::PerformQuery allowed only with CannedQuery object");
    }
}

ActivationResponseImpl::ActivationResponseImpl(CannedQuery const& query)
    : status_(ActivationResponse::Status::PerformQuery),
      query_(std::make_shared<CannedQuery>(query))
{
}

ActivationResponseImpl::ActivationResponseImpl(VariantMap const& var)
{
    auto it = var.find("scope_data");
    if (it == var.end())
    {
        throw LogicException("ActivationResponseImpl(): Invalid data, missing 'scope_data'");
    }
    scope_data_ = it->second;

    it = var.find("status");
    if (it == var.end())
    {
        throw LogicException("ActivationResponseImpl(): Invalid data, missing 'status'");
    }
    status_ = static_cast<ActivationResponse::Status>(it->second.get_int());

    if (status_ == ActivationResponse::Status::PerformQuery)
    {
        it = var.find("query");
        if (it == var.end())
        {
            throw LogicException("ActivationResponseImpl(): Invalid data, missing 'query'");
        }
        query_ = std::make_shared<CannedQuery>(CannedQueryImpl::create(it->second.get_dict()));
    }
}

ActivationResponse::Status ActivationResponseImpl::status() const
{
    return status_;
}

void ActivationResponseImpl::set_scope_data(Variant const& hints)
{
    scope_data_ = hints;
}

Variant ActivationResponseImpl::scope_data() const
{
    return scope_data_;
}

CannedQuery ActivationResponseImpl::query() const
{
    if (query_)
    {
        assert(status_ == ActivationResponse::Status::PerformQuery);
        return *query_;
    }
    throw LogicException("ActivationResponse::query(): query is only available for status of Status::PerformQuery");
}

VariantMap ActivationResponseImpl::serialize() const
{
    VariantMap vm;
    vm["status"] = static_cast<int>(status_);
    vm["scope_data"] = scope_data_;
    if (query_)
    {
        vm["query"] = query_->serialize();
    }
    return vm;
}

ActivationResponse ActivationResponseImpl::create(VariantMap const& var)
{
    return ActivationResponse(new ActivationResponseImpl(var));
}

} // namespace internal

} // namespace scopes

} // namespace unity
