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
#include <unity/util/IniParser.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class SettingsSchema
{
public:
    NONCOPYABLE(SettingsSchema);
    UNITY_DEFINES_PTRS(SettingsSchema);

    SettingsSchema() = default;
    virtual ~SettingsSchema() = default;

    SettingsSchema(SettingsSchema&&) = default;
    SettingsSchema& operator=(SettingsSchema&&) = default;

    virtual VariantArray definitions() const = 0;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
