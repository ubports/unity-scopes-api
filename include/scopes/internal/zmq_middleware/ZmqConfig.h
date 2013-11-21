/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQCONFIG_H
#define UNITY_API_SCOPES_INTERNAL_ZMQCONFIG_H

#include <scopes/internal/ConfigBase.h>

#include <mutex>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ZmqConfig : public ConfigBase
{
public:
    static const char* ZMQ_CONFIG_GROUP;

    ZmqConfig(std::string const& configfile);
    ~ZmqConfig() noexcept;

    std::string public_dir() const;
    std::string private_dir() const;

private:
    std::string public_dir_;
    mutable std::string private_dir_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
