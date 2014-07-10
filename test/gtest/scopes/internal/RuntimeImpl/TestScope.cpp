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

#include "TestScope.h"

using namespace std;
using namespace unity::scopes;

void TestScope::start(string const&)
{
    lock_guard<mutex> lock(mutex_);

    // Retrieve all the environment variables that are supposed to be set
    // and make them available via the env_vars() method.
    env_vars_["XDG_RUNTIME_DIR"] = getenv("XDG_RUNTIME_DIR");
    env_vars_["TMPDIR"] = getenv("TMPDIR");
    env_vars_["XDG_DATA_HOME"] = getenv("XDG_DATA_HOME");
    env_vars_["XDG_CONFIG_HOME"] = getenv("XDG_CONFIG_HOME");
    env_vars_["UBUNTU_APPLICATION_ISOLATION"] = getenv("UBUNTU_APPLICATION_ISOLATION");
    env_vars_["LD_LIBRARY_PATH"] = getenv("LD_LIBRARY_PATH");
    env_vars_["PATH"] = getenv("PATH");
}

map<string, string> TestScope::env_vars() const
{
    lock_guard<mutex> lock(mutex_);
    return env_vars_;
}

SearchQueryBase::UPtr TestScope::search(CannedQuery const&, SearchMetadata const&)
{
    // Never called
    abort();
    return nullptr;
}

PreviewQueryBase::UPtr TestScope::preview(Result const&, ActionMetadata const&)
{
    // Never called
    abort();
    return nullptr;
}
