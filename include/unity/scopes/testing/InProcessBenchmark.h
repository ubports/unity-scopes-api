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

#pragma once

#include <unity/scopes/testing/Benchmark.h>

namespace unity
{

namespace scopes
{
class ScopeBase;

namespace testing
{
/**
 * \brief The InProcessBenchmark class provides scope authors
 * with runtime benchmarking capabilities. The actual runs are executed in the same process.
 *
 * \code
 * unity::scopes::testing::InProcessBenchmark benchmark;
 *
 * unity::scopes::Query query{scope_id};
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
class InProcessBenchmark : public Benchmark
{
public:
    InProcessBenchmark() = default;

    virtual Result for_query(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                             QueryConfiguration configuration) override;

    virtual Result for_preview(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                               PreviewConfiguration preview_configuration) override;

    virtual Result for_activation(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                                  ActivationConfiguration activation_configuration) override;

    virtual Result for_action(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                              ActionConfiguration activation_configuration) override;
};

} // namespace testing

} // namespace scopes

} // namespace unity
