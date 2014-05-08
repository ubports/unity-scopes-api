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

#ifndef UNITY_SCOPES_INTERNAL_SSCONFIG_H
#define UNITY_SCOPES_INTERNAL_SSCONFIG_H

#include <unity/scopes/internal/ConfigBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SSConfig : public ConfigBase
{
public:
    SSConfig(std::string const& configfile);
    ~SSConfig();

    int http_reply_timeout() const;             // seconds
    int reg_refresh_rate() const;               // seconds
    int reg_refresh_fail_timeout() const;       // seconds
    std::string scope_identity() const;

private:
    int http_reply_timeout_;
    int reg_refresh_rate_;
    int reg_refresh_fail_timeout_;
    std::string scope_identity_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
