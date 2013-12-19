/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
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
 * Authored by: Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#ifndef SCOPEREGISTRY_SCOPESET_H_
#define SCOPEREGISTRY_SCOPESET_H_

#include<vector>
#include<string>
#include<scopes/internal/RegistryConfig.h>
#include<scopes/internal/ScopeConfig.h>


namespace scoperegistry
{

struct ScopeSetPrivate;


class ScopeSet final
{

public:

    ScopeSet(const ::unity::api::scopes::internal::RegistryConfig& c);
    ~ScopeSet();

    ScopeSet(const ScopeSet&) = delete;
    ScopeSet& operator=(const ScopeSet&) = delete;

    std::vector<std::string> list() const;
    const unity::api::scopes::internal::ScopeConfig& get(const std::string& name);

private:
    ScopeSetPrivate* p;

};


}

#endif
