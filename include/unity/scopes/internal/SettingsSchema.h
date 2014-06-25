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

#ifndef UNITY_SCOPES_INTERNAL_SETTINGSSCHEMA_H
#define UNITY_SCOPES_INTERNAL_SETTINGSSCHEMA_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

// Simple class to parse a JSON string with settings definitions
// and convert them into a into a VariantMap that maps each
// setting ID to its default value.
// If a particular setting ID does not define any default value, the
// corresponding Variant is null.

class SettingsSchema final
{
public:
    NONCOPYABLE(SettingsSchema);
    UNITY_DEFINES_PTRS(SettingsSchema);

    SettingsSchema(std::string const& json_schema);
    ~SettingsSchema();

    VariantArray definitions() const;

private:
    VariantArray definitions_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
