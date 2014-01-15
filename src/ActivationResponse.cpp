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

#include <scopes/ActivationResponse.h>
#include <scopes/internal/ActivationResponseImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

ActivationResponse::ActivationResponse(Status status)
    : p(new internal::ActivationResponseImpl(status))
{
}

ActivationResponse::ActivationResponse(std::shared_ptr<internal::ActivationResponseImpl> pimpl)
    : p(pimpl)
{
}

ActivationResponse::~ActivationResponse() = default;

ActivationResponse::ActivationResponse(ActivationResponse const& other)
    : p(new internal::ActivationResponseImpl(*other.p))
{
}

ActivationResponse::ActivationResponse(ActivationResponse&&) = default;

ActivationResponse& ActivationResponse::operator=(ActivationResponse const& other)
{
    if (this != &other)
    {
        p.reset(new internal::ActivationResponseImpl(*other.p));
    }
    return *this;
}

ActivationResponse& ActivationResponse::operator=(ActivationResponse&&) = default;

void ActivationResponse::setHints(VariantMap const& hints)
{
    p->setHints(hints);
}

ActivationResponse::Status ActivationResponse::status() const
{
    return p->status();
}

VariantMap ActivationResponse::hints() const
{
    return p->hints();
}

VariantMap ActivationResponse::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace api

} // namespace unity
