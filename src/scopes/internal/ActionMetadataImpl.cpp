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

#include <unity/scopes/internal/ActionMetadataImpl.h>
#include <unity/scopes/internal/Utils.h>

namespace unity
{

namespace scopes
{

namespace internal
{

ActionMetadataImpl::ActionMetadataImpl(std::string const& locale, std::string const& form_factor)
    : QueryMetadataImpl(locale, form_factor)
{
}

ActionMetadataImpl::ActionMetadataImpl(VariantMap const& var)
    : QueryMetadataImpl(var)
{
    auto it = find_or_throw("ActionMetadataImpl()", var, "scope_data");
    scope_data_ = it->second;
}

std::string ActionMetadataImpl::metadata_type() const
{
    static const std::string t("action_metadata");
    return t;
}

void ActionMetadataImpl::set_scope_data(Variant const& data)
{
    scope_data_ = data;
}

Variant ActionMetadataImpl::scope_data() const
{
    return scope_data_;
}

void ActionMetadataImpl::serialize(VariantMap &var) const
{
    QueryMetadataImpl::serialize(var);
    var["scope_data"] = Variant(scope_data_);
}

VariantMap ActionMetadataImpl::serialize() const
{
    VariantMap vm;
    serialize(vm);
    return vm;
}

ActionMetadata ActionMetadataImpl::create(VariantMap const& var)
{
    return ActionMetadata(new ActionMetadataImpl(var));
}

} // namespace internal

} // namespace scopes

} // namespace unity
