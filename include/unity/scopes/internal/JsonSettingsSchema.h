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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/scopes/internal/SettingsSchema.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class JsonSettingsSchema : public SettingsSchema
{
public:
    NONCOPYABLE(JsonSettingsSchema);
    UNITY_DEFINES_PTRS(JsonSettingsSchema);

    static UPtr create(std::string const& json_string);

    static UPtr create_empty();

    ~JsonSettingsSchema();

    JsonSettingsSchema(JsonSettingsSchema&&) = default;
    JsonSettingsSchema& operator=(JsonSettingsSchema&&) = default;

    virtual VariantArray definitions() const override;

    void add_location_setting() override;

private:
    JsonSettingsSchema(std::string const& json_string);

    JsonSettingsSchema();

    VariantArray definitions_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
