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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#ifndef UNITY_SCOPES_TESTING_BENCHMARK_H
#define UNITY_SCOPES_TESTING_BENCHMARK_H

#include <chrono>
#include <iosfwd>
#include <memory>

namespace unity
{

namespace scopes
{
class ActionMetadata;
class Query;
class Result;
class ScopeBase;
class SearchMetadata;

namespace testing
{

class Benchmark
{
public:
    struct Result
    {
        std::size_t sample_size = 0;
        struct
        {
            std::chrono::microseconds mean{};
            std::chrono::microseconds std_dev{};
        } time{};
    };

    Result for_query(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                     const unity::scopes::Query& query,
                     const unity::scopes::SearchMetadata& md,
                     std::size_t sample_size,
                     const std::chrono::microseconds& per_query_timeout);

    Result for_preview(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                       const unity::scopes::Result& result,
                       const unity::scopes::ActionMetadata& md,
                       std::size_t sample_size,
                       const std::chrono::microseconds& per_query_timeout);
};

std::ostream& operator<<(std::ostream&, const Benchmark::Result&);

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
