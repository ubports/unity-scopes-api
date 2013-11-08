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
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/internal/UniqueID.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

UniqueID::UniqueID() :
    UniqueID(random_device()())
{
}

UniqueID::UniqueID(mt19937::result_type seed) :
    engine(seed),
    counter(0)
{
    s << hex << setfill('0');
}

string UniqueID::gen()
{
    lock_guard<std::mutex> lock(m);
    s << setw(8) << uniform_dist(engine) << setw(8) << counter++;
    string id(s.str());
    s.str("");
    return id;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
