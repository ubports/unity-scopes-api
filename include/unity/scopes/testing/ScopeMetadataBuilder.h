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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#pragma once

#include <unity/scopes/ScopeMetadata.h>

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class ScopeMetadataBuilder
{
public:
    template<typename T>
    class Optional
    {
    public:
        inline Optional() = default;
        inline Optional(const T& t) : value(new T{t})
        {
        }

        inline operator bool() const
        {
            return value.get() != nullptr;
        }

        inline Optional<T>& operator=(const T& rhs)
        {
            if (*this)
                *value = rhs;
            else
                value.reset(new T{rhs});

            return *this;
        }

        inline Optional<T>& operator=(const Optional<T>& rhs)
        {
            if (rhs)
                *this = *rhs;
            else
                value.reset();

            return *this;
        }

        inline const T& operator*() const
        {
            return *value;
        }

    private:
        std::unique_ptr<T> value;
    };

    ScopeMetadataBuilder();
    ~ScopeMetadataBuilder();

    ScopeMetadataBuilder& scope_id(std::string const& value);
    ScopeMetadataBuilder& proxy(ScopeProxy const& value);
    ScopeMetadataBuilder& display_name(std::string const& value);
    ScopeMetadataBuilder& description(std::string const& value);
    ScopeMetadataBuilder& author(std::string const& value);
    ScopeMetadataBuilder& art(Optional<std::string> const& value);
    ScopeMetadataBuilder& icon(Optional<std::string> const& value);
    ScopeMetadataBuilder& search_hint(Optional<std::string> const& value);
    ScopeMetadataBuilder& hot_key(Optional<std::string> const& value);
    ScopeMetadataBuilder& invisible(Optional<bool> const& value);
    ScopeMetadataBuilder& appearance_attributes(Optional<VariantMap> const& value);
    ScopeMetadataBuilder& scope_directory(Optional<std::string> const& value);
    ScopeMetadataBuilder& results_ttl_type(Optional<ScopeMetadata::ResultsTtlType> const& value);
    ScopeMetadataBuilder& settings_definitions(Optional<VariantArray> const& value);
    ScopeMetadataBuilder& location_data_needed(Optional<bool> const& value);
    ScopeMetadataBuilder& child_scope_ids(Optional<std::vector<std::string>> const& value);
    ScopeMetadataBuilder& version(Optional<int> const& value);
    ScopeMetadataBuilder& keywords(Optional<std::set<std::string>> const& value);
    ScopeMetadataBuilder& is_aggregator(Optional<bool> const& value);

    ScopeMetadata operator()() const;

private:
    struct Private;
    std::unique_ptr<Private> p;
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity
