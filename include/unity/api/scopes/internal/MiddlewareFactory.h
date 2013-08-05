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

#ifndef UNITY_API_SCOPES_INTERNAL_MIDDLEWAREFACTORY_H
#define UNITY_API_SCOPES_INTERNAL_MIDDLEWAREFACTORY_H

#include <unity/api/scopes/internal/MiddlewareBase.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Factory to make middleware instances available.
//
// The lookup functions for the client-side use lazy initialization. The middleware
// instance is created when caller first asks for it, and the same single instance
// is always returned thereafter. The client-side middleware(s) are shut down when the factory
// is destroyed.
//
// The create functions for the server-side pass ownership to the caller.
//
// This class is thread-safe.

class MiddlewareFactory final : private util::NonCopyable
{
public:
    UNITY_DEFINES_PTRS(MiddlewareFactory);

    MiddlewareFactory(std::string const& configfile);
    ~MiddlewareFactory() noexcept;

    // Returns the middleware with the given name and kind. If that middleware hasn't been created yet,
    // it is created first.
    MiddlewareBase::SPtr create(std::string const& server_name,
                                std::string const& kind,
                                std::string const& configfile) const;

    // Find the middleware with the given name and kind. Returns nullptr if that middleware doesn't exist.
    MiddlewareBase::SPtr find(std::string const& server_name, std::string const& kind) const;

private:
    MiddlewareBase::SPtr find_unlocked(std::string const& server_name, std::string const& kind) const;

    enum Kind { Kind_Ice, Kind_REST };
    static Kind to_kind(::std::string const& kind);

    struct MiddlewareData
    {
        std::string server_name;
        std::string kind;

        struct Compare
        {
            bool operator()(MiddlewareData const& lhs, MiddlewareData const& rhs) const
            {
                if (lhs.server_name < rhs.server_name)
                {
                    return true;
                }
                return lhs.kind < rhs.kind;
            }
        };
    };

    mutable std::map<MiddlewareData, MiddlewareBase::SPtr, MiddlewareData::Compare> mw_map_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
