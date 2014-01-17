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

#include"FindFiles.h"
#include"ScopeSet.h"
#include <unity/scopes/ScopeMetadata.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/ScopeConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include<memory>
#include<map>
#include<set>
#include<cstring>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace scoperegistry
{

static string strip_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        if (s.compare(s_len - suffix_len, suffix_len, suffix) == 0)
        {
            return string(s, 0, s_len - suffix_len);
        }
    }
    return s;
}

struct ScopeSetPrivate
{
    std::set<string> overridable_scopes;
    std::map<string, unique_ptr<ScopeConfig>> scopes;
};

ScopeSet::ScopeSet(const RegistryConfig& c) : p(new ScopeSetPrivate())
{
    string canonical_dir = c.scope_installdir();
    string oem_dir = c.oem_installdir();
    auto canonical_files = find_scope_config_files(canonical_dir, ".ini");
    auto oem_files = find_scope_config_files(oem_dir, ".ini");
    for (const auto &path : canonical_files)
    {
        unique_ptr<ScopeConfig> sc(new ScopeConfig(path));
        // basename() modifies its argument
        string file_name = basename(const_cast<char*>(string(path).c_str()));
        string scope_name = strip_suffix(file_name, ".ini");
        if (sc->overrideable())
        {
            p->overridable_scopes.insert(path);
        }
        p->scopes[scope_name] = move(sc);
    }
    for (const auto &path : oem_files)
    {
        unique_ptr<ScopeConfig> sc(new ScopeConfig(path));
        string file_name = basename(const_cast<char*>(string(path).c_str()));
        string scope_name = strip_suffix(file_name, ".ini");
        if (p->scopes.find(scope_name) != p->scopes.end())
        {
            if (p->overridable_scopes.find(scope_name) != p->overridable_scopes.end())
            {
                p->scopes[scope_name] = move(sc);
            }
            else
            {
                // print error about trying to override a non-overridable scope.
            }
        }
        else
        {
            p->scopes[scope_name] = move(sc);
        }
    }
    // Add click scope parsing here.
}

ScopeSet::~ScopeSet()
{
    delete p;
}

std::vector<std::string> ScopeSet::list() const
{
    vector<string> list;
    for (auto &i : p->scopes)
    {
        list.push_back(i.first);
    }
    return list;
}

const unity::scopes::internal::ScopeConfig&
ScopeSet::get(const std::string& name)
{
    if (p->scopes.find(name) == p->scopes.end())
    {
        throw NotFoundException("Scope does not exist", name);
    }
    return *p->scopes[name].get();
}

}
