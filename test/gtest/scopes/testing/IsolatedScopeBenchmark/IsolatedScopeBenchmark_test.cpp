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

#include <unity/scopes/testing/InProcessBenchmark.h>
#include <unity/scopes/testing/OutOfProcessBenchmark.h>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/Result.h>
#include <unity/scopes/testing/ScopeMetadataBuilder.h>
#include <unity/scopes/testing/Statistics.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

#include <unity/scopes/CategoryRenderer.h>

#include <gtest/gtest.h>

#include "scope.h"

#include <cstdio>
#include <fstream>

namespace
{
typedef unity::scopes::testing::TypedScopeFixture<testing::Scope> BenchmarkScopeFixture;

static const std::string scope_name{"does.not.exist.scope"};
static const std::string scope_query_string{"does.not.exist.scope.query_string"};

static const std::string default_locale{"C"};
static const std::string default_form_factor{"SuperDuperPhablet"};

static const double alpha = 0.1;

static const std::size_t dont_care{0};

unity::scopes::testing::Benchmark::Result reference_query_performance()
{
    unity::scopes::testing::Benchmark::Result result{};
    result.timing.mean = std::chrono::milliseconds{500};
    result.timing.std_dev = std::chrono::microseconds{dont_care};

    return result;
}

unity::scopes::testing::Benchmark::Result reference_preview_performance()
{
    unity::scopes::testing::Benchmark::Result result{};
    result.timing.mean = std::chrono::milliseconds{500};
    result.timing.std_dev = std::chrono::microseconds{dont_care};

    return result;
}

unity::scopes::testing::Benchmark::Result reference_activation_performance()
{
    unity::scopes::testing::Benchmark::Result result{};
    result.timing.mean = std::chrono::milliseconds{500};
    result.timing.std_dev = std::chrono::microseconds{dont_care};

    return result;
}

unity::scopes::testing::Benchmark::Result reference_action_performance()
{
    unity::scopes::testing::Benchmark::Result result{};
    result.timing.mean = std::chrono::milliseconds{500};
    result.timing.std_dev = std::chrono::microseconds{dont_care};

    return result;
}

}

TEST(BenchmarkResult, saving_and_loading_works)
{
    const std::string fn{"test.result"};
    std::remove(fn.c_str());

    unity::scopes::testing::Benchmark::Result reference;
    reference.sample_size = std::rand();

    {
        std::ofstream out{fn.c_str()};
        ASSERT_NO_THROW(reference.save_to(out));
    }

    {
        unity::scopes::testing::Benchmark::Result result;
        std::ifstream in{fn.c_str()};
        ASSERT_NO_THROW(result.load_from(in));
        EXPECT_EQ(reference, result);
    }
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_query_performance_oop_works)
{
    unity::scopes::testing::OutOfProcessBenchmark benchmark;

    unity::scopes::Query query{scope_name};
    query.set_query_string(scope_query_string);

    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::QueryConfiguration config;
    config.sampler = [query, meta_data]()
    {
        return std::make_pair(query, meta_data);
    };

    auto result = benchmark.for_query(scope, config);
    result.save_to(std::cout);
    result.save_to_xml(std::cout);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_query_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_preview_performance_oop_works)
{
    unity::scopes::testing::OutOfProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::PreviewConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_pair(search_result, meta_data);
    };

    auto result = benchmark.for_preview(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_preview_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_activation_performance_oop_works)
{
    unity::scopes::testing::OutOfProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::ActivationConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_pair(search_result, meta_data);
    };

    auto result = benchmark.for_activation(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_activation_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_action_performance_oop_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};
    static const std::string widget_id{"does.not.exist.widget"};
    static const std::string action_id{"does.not.exist.action"};

    unity::scopes::testing::Benchmark::ActionConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_tuple(search_result, meta_data, widget_id, action_id);
    };

    auto result = benchmark.for_action(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_action_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_query_performance_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::Query query{scope_name};
    query.set_query_string(scope_query_string);

    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::QueryConfiguration config;
    config.sampler = [query, meta_data]()
    {
        return std::make_pair(query, meta_data);
    };

    auto result = benchmark.for_query(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_query_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_preview_performance_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::PreviewConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_pair(search_result, meta_data);
    };

    auto result = benchmark.for_preview(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_preview_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_activation_performance_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::ActivationConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_pair(search_result, meta_data);
    };

    config.trial_configuration.statistics_configuration.histogram_bin_count = 20;

    auto result = benchmark.for_activation(scope, config);

    std::ofstream out("histogram.txt");
    for (const auto& bin : result.timing.histogram)
        out << bin.first.count() << " " << bin.second << std::endl;

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_activation_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_action_performance_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::testing::Result search_result;
    unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};
    static const std::string widget_id{"does.not.exist.widget"};
    static const std::string action_id{"does.not.exist.action"};

    unity::scopes::testing::Benchmark::ActionConfiguration config;
    config.sampler = [search_result, meta_data]()
    {
        return std::make_tuple(search_result, meta_data, widget_id, action_id);
    };

    auto result = benchmark.for_action(scope, config);

    auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
                reference_action_performance(),
                result);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_eq_to_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              test_result.sample_mean_is_gt_reference(alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              test_result.sample_mean_is_lt_reference(alpha));
}
