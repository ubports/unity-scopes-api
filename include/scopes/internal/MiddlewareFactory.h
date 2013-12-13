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

#ifndef UNITY_API_SCOPES_INTERNAL_MIDDLEWAREFACTORY_H
#define UNITY_API_SCOPES_INTERNAL_MIDDLEWAREFACTORY_H

#include <scopes/internal/MiddlewareBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

// Factory to make middleware instances available.
//
// This class is thread-safe.

class MiddlewareFactory final : private util::NonCopyable
{
public:
    UNITY_DEFINES_PTRS(MiddlewareFactory);

    MiddlewareFactory(RuntimeImpl* runtime);
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

    enum Kind { Kind_Zmq, Kind_REST };
    static Kind to_kind(::std::string const& kind);

    struct MiddlewareData
    {
        std::string server_name;
        std::string kind;

        bool operator<(MiddlewareData const& rhs) const
        {
            if (server_name < rhs.server_name)
            {
                return true;
            }
            if (server_name > rhs.server_name)
            {
                return false;
            }
            return kind < rhs.kind;
        }
    };

    RuntimeImpl* const runtime_;
    mutable std::map<MiddlewareData, MiddlewareBase::SPtr> mw_map_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
