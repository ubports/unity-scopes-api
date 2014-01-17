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

namespace unity
{

namespace scopes
{

namespace internal
{

ActivationResponseImpl::ActivationResponseImpl(ActivationResponse::Status status):
    status_(status)
{
}

ActivationResponseImpl::ActivationResponseImpl(VariantMap const& var)
{
    auto it = var.find("hints");
    if (it == var.end())
    {
        throw LogicException("ActivationResponseImpl(): Invalid data, missing 'hints'");
    }
    hints_ = it->second.get_dict();

    it = var.find("status");
    if (it == var.end())
    {
        throw LogicException("ActivationResponseImpl(): Invalid data, missing 'status'");
    }
    status_ = static_cast<ActivationResponse::Status>(it->second.get_int());
}

ActivationResponse::Status ActivationResponseImpl::status() const
{
    return status_;
}

void ActivationResponseImpl::setHints(VariantMap const& hints)
{
    hints_ = hints;
}

VariantMap ActivationResponseImpl::hints() const
{
    return hints_;
}

VariantMap ActivationResponseImpl::serialize() const
{
    VariantMap vm;
    vm["status"] = static_cast<int>(status_);
    vm["hints"] = Variant(hints_);
    return vm;
}

ActivationResponse ActivationResponseImpl::create(VariantMap const& var)
{
    auto impl = std::make_shared<ActivationResponseImpl>(var);
    return ActivationResponse(impl);
}

} // namespace internal

} // namespace scopes

} // namespace unity
