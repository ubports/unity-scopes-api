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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/scopes/Variant.h>

#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

class SettingsDB;

class QueryBaseImpl
{
public:
    QueryBaseImpl();
    virtual ~QueryBaseImpl();

    virtual void cancel() = 0;
    virtual bool valid() const = 0;

    unity::scopes::VariantMap settings() const;
    void set_settings_db(std::shared_ptr<unity::scopes::internal::SettingsDB> const& db);

private:
    std::shared_ptr<unity::scopes::internal::SettingsDB> db_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
