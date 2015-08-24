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

#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/ActivationResponseImpl.h>

namespace unity
{

namespace scopes
{

ActivationResponse::ActivationResponse(Status status)
    : p(new internal::ActivationResponseImpl(status))
{
}

ActivationResponse::ActivationResponse(CannedQuery const& query)
    : p(new internal::ActivationResponseImpl(query))
{
}

ActivationResponse::ActivationResponse(Result const& updated_result)
    : p(new internal::ActivationResponseImpl(updated_result))
{
}

ActivationResponse::ActivationResponse(PreviewWidgetList const& updated_widgets)
    :p(new internal::ActivationResponseImpl(updated_widgets))
{
}

/// @cond
ActivationResponse::ActivationResponse(internal::ActivationResponseImpl* pimpl)
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

VariantMap ActivationResponse::serialize() const
{
    return p->serialize();
}
/// @endcond

void ActivationResponse::set_scope_data(Variant const& data)
{
    p->set_scope_data(data);
}

ActivationResponse::Status ActivationResponse::status() const
{
    return p->status();
}

Variant ActivationResponse::scope_data() const
{
    return p->scope_data();
}

CannedQuery ActivationResponse::query() const
{
    return p->query();
}

Result ActivationResponse::updated_result() const
{
    return p->updated_result();
}

PreviewWidgetList ActivationResponse::updated_widgets() const
{
    return p->updated_widgets();
}

} // namespace scopes

} // namespace unity
