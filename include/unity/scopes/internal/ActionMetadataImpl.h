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

#ifndef UNITY_INTERNAL_ACTIONMETADATAIMPL_H
#define UNITY_INTERNAL_ACTIONMETADATAIMPL_H

#include <unity/scopes/internal/QueryMetadataImpl.h>
#include <unity/scopes/ActionMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ActionMetadataImpl : public QueryMetadataImpl
{
public:
    ActionMetadataImpl(std::string const& locale, std::string const& form_factor);
    ActionMetadataImpl(VariantMap const& var);
    ~ActionMetadataImpl() = default;

    void set_scope_data(Variant const& data);
    Variant scope_data() const;

    static ActionMetadata create(VariantMap const& var);

protected:
    std::string metadata_type() const override;
    void serialize(VariantMap &var) const override;

private:
    Variant scope_data_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
