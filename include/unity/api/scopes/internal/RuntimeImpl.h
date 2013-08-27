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

#ifndef UNITY_API_SCOPES_INTERNAL_RUNTIMEIMPL_H
#define UNITY_API_SCOPES_INTERNAL_RUNTIMEIMPL_H

#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/api/scopes/internal/MiddlewareFactory.h>
#include <unity/api/scopes/internal/Reaper.h>
#include <unity/api/scopes/Runtime.h>

#include <atomic>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class RuntimeImpl final : private util::NonCopyable
{
public:
    UNITY_DEFINES_PTRS(RuntimeImpl);

    static UPtr create(std::string const& scope_name, std::string const& configfile = Runtime::DFLT_CONFIGFILE);
    void destroy();

    std::string scope_name() const;
    MiddlewareFactory const* factory() const;
    RegistryProxy registry() const;
    std::string registry_configfile() const;
    std::string registry_identity() const;
    Reaper::SPtr reply_reaper() const;

    ~RuntimeImpl() noexcept;

private:
    RuntimeImpl(std::string const& scope_name, std::string const& configfile);

    std::atomic_bool destroyed_;
    std::string scope_name_;
    MiddlewareFactory::UPtr middleware_factory_;
    MiddlewareBase::SPtr middleware_;
    mutable RegistryProxy registry_;
    mutable std::string registry_configfile_;
    mutable std::string registry_identity_;
    mutable Reaper::SPtr reply_reaper_;
    mutable std::mutex mutex_;                          // For lazy initialization of reply_reaper_
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
