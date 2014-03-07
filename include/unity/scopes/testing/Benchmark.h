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

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/SearchMetadata.h>

#include <chrono>
#include <functional>
#include <iosfwd>
#include <memory>

namespace unity
{

namespace scopes
{
class ScopeBase;

namespace testing
{

/// @cond

class Benchmark
{
public:
    struct Result
    {
        std::size_t sample_size;
        struct Time
        {
            std::chrono::microseconds mean;
            std::chrono::microseconds std_dev;
        } time;
    };

    struct TrialConfiguration
    {
        std::size_t trial_count;
        std::chrono::microseconds per_trial_timeout;
    };

    struct QueryConfiguration
    {
        std::function<std::pair<unity::scopes::CannedQuery, unity::scopes::SearchMetadata>()> sampler;
        TrialConfiguration trial_configuration;
    };

    struct PreviewConfiguration
    {
        std::function<std::pair<unity::scopes::Result, unity::scopes::ActionMetadata>()> sampler;
        TrialConfiguration trial_configuration;
    };

    struct ActivationConfiguration
    {
        std::function<std::pair<unity::scopes::Result, unity::scopes::ActionMetadata>()> sampler;
        TrialConfiguration trial_configuration;
    };

    struct ActionConfiguration
    {
        std::function<
            std::tuple<
                unity::scopes::Result,
                unity::scopes::ActionMetadata,
                std::string,
                std::string
            >()
        > sampler;
        TrialConfiguration trial_configuration;
    };

    virtual ~Benchmark() = default;
    Benchmark(const Benchmark&) = delete;
    Benchmark(Benchmark&&) = delete;

    Benchmark& operator=(const Benchmark&) = delete;
    Benchmark& operator=(Benchmark&&) = delete;

    virtual Result for_query(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                             QueryConfiguration configuration) = 0;

    virtual Result for_preview(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                               PreviewConfiguration preview_configuration) = 0;

    virtual Result for_activation(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                                  ActivationConfiguration activation_configuration) = 0;

    virtual Result for_action(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                              ActionConfiguration activation_configuration) = 0;

protected:
    Benchmark() = default;
};

std::ostream& operator<<(std::ostream&, const Benchmark::Result&);

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
