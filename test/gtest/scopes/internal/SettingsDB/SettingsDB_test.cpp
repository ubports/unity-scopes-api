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

#include <unity/scopes/internal/SettingsDB.h>

#include <unity/util/FileIO.h>

#include <gtest/gtest.h>

using namespace unity::scopes::internal;
using namespace std;

TEST(Version, basic)
{
    auto schema = unity::util::read_text_file("/usr/lib/x86_64-linux-gnu/unity-scopes/musicaggregator/musicaggregator.json");
    try
    {
        SettingsDB db("/home/michi/.local/share/musicaggregator/settings.db", schema);
        db.get_all_docs();
    }
    catch (std::exception const& e)
    {
        cerr << e.what() << endl;
    }
}
