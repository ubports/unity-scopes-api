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

#include <unity/scopes/internal/smartscopes/SSConfig.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>

#include <unistd.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

namespace
{
    const string ss_config_group = "Smartscopes";
    const string http_reply_timeout_key = "Http.Reply.Timeout";
    const string reg_refresh_rate_key = "Registry.Refresh.Rate";
    const string reg_refresh_fail_timeout_key = "Registry.Refresh.Fail.Timeout";
    const string scope_identity_key = "Scope.Identity";
}

SSConfig::SSConfig(string const& configfile) :
    ConfigBase(configfile, DFLT_SS_INI)
{
    if (configfile.empty())
    {
        http_reply_timeout_ = DFLT_SS_HTTP_TIMEOUT;
        reg_refresh_rate_ = DFLT_SS_REG_REFRESH_RATE;
        reg_refresh_fail_timeout_ = DFLT_SS_REG_REFRESH_FAIL_TIMEOUT;
        scope_identity_ = DFLT_SS_SCOPE_IDENTITY;
    }
    else
    {
        http_reply_timeout_ = get_optional_int(ss_config_group, http_reply_timeout_key, DFLT_SS_HTTP_TIMEOUT);
        if (http_reply_timeout_ < 1 || http_reply_timeout_ > 60)
        {
            throw_ex("Illegal value (" + to_string(http_reply_timeout_) + ") for " +
                     http_reply_timeout_key + ": value must be 10 - 60");
        }

        reg_refresh_rate_ = get_optional_int(ss_config_group, reg_refresh_rate_key, DFLT_SS_REG_REFRESH_RATE);
        if (reg_refresh_rate_ < 60)
        {
            throw_ex("Illegal value (" + to_string(reg_refresh_rate_) + ") for " +
                     reg_refresh_rate_key + ": value must be >= 60");
        }

        reg_refresh_fail_timeout_ = get_optional_int(ss_config_group,
                                                     reg_refresh_fail_timeout_key,
                                                     DFLT_SS_REG_REFRESH_FAIL_TIMEOUT);
        if (reg_refresh_fail_timeout_ < 1)
        {
            throw_ex("Illegal value (" + to_string(reg_refresh_fail_timeout_) + ") for " +
                     reg_refresh_fail_timeout_key + ": value must be >= 1");
        }

        scope_identity_ = get_optional_string(ss_config_group, scope_identity_key, DFLT_SS_SCOPE_IDENTITY);
    }

    const KnownEntries known_entries = {
                                          {  ss_config_group,
                                             {
                                                http_reply_timeout_key,
                                                reg_refresh_rate_key,
                                                reg_refresh_fail_timeout_key,
                                                scope_identity_key,
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

SSConfig::~SSConfig()
{
}

int SSConfig::http_reply_timeout() const
{
    return http_reply_timeout_;
}

int SSConfig::reg_refresh_rate() const
{
    return reg_refresh_rate_;
}

int SSConfig::reg_refresh_fail_timeout() const
{
    return reg_refresh_fail_timeout_;
}

string SSConfig::scope_identity() const
{
    return scope_identity_;
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity
