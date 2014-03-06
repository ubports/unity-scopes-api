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

#ifndef UNITY_SCOPES_TESTING_OUT_OF_PROCESS_BENCHMARK_H
#define UNITY_SCOPES_TESTING_OUT_OF_PROCESS_BENCHMARK_H

#include <unity/scopes/testing/InProcessBenchmark.h>

namespace unity
{

namespace scopes
{
class ScopeBase;

namespace testing
{

/**
 * \brief The OutOfProcessBenchmark class provides scope authors
 * with runtime benchmarking capabilities. The actual runs are executed in another process.
 *
 * Please note that this class provides better test isolation than the class InProcessBenchmark
 * and we promote use of this implementation due to this feature. However, as the internal
 * implementation relies on fork(), the results in case of multi-threaded environments that exist
 * before the execution of the benchmark are undefined.
 *
 * \code
 * unity::scopes::testing::OutOfProcessBenchmark benchmark;
 *
 * unity::scopes::Query query{scope_name};
 * query.set_query_string(scope_query_string);
 *
 * unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};
 *
 * unity::scopes::testing::Benchmark::QueryConfiguration config;
 * config.sampler = [query, meta_data]()
 * {
 *     return std::make_pair(query, meta_data);
 * };
 *
 * auto result = benchmark.for_query(scope, config);
 * \endcode
 *
 */
class OutOfProcessBenchmark : public InProcessBenchmark
{
public:
    OutOfProcessBenchmark() = default;

    Result for_query(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                     QueryConfiguration configuration) override;

    Result for_preview(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                       PreviewConfiguration preview_configuration) override;

    Result for_activation(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                          ActivationConfiguration activation_configuration) override;

    Result for_action(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                      ActionConfiguration activation_configuration) override;
};

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
