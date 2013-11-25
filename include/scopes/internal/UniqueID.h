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

#ifndef UNITY_API_SCOPES_INTERNAL_UNIQUEID_H
#define UNITY_API_SCOPES_INTERNAL_UNIQUEID_H

#include <unity/util/NonCopyable.h>

#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Poor man's thread-safe unique ID generator.
// Generates a random number concatenated with a counter.
// Return value is a string of 16 hex digits.

class UniqueID : private util::NonCopyable
{
public:
    UniqueID();                                         // Gets seed from std::random_device
    explicit UniqueID(std::mt19937::result_type seed);  // Uses specified seed

    std::string gen();                                  // Returns a unique id

private:
    std::mt19937 engine;
    std::uniform_int_distribution<uint32_t> uniform_dist;
    int counter;
    std::ostringstream s;
    std::mutex m;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
