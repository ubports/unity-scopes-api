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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "config.h"
#include "scope.h"

#include <cstdio>
#include <fstream>
#include <random>

namespace
{
std::chrono::milliseconds mean{100};
std::chrono::milliseconds variance{20};
}

namespace unity
{
namespace scopes
{
namespace testing
{
template<>
struct ScopeTraits<::testing::Scope>
{
    inline static const char* name()
    {
        return "BenchmarkingScope";
    }

    inline static std::shared_ptr<::testing::Scope> construct()
    {
        return std::make_shared<::testing::Scope>(mean, variance);
    }
};
}
}
}

namespace
{
typedef unity::scopes::testing::TypedScopeFixture<testing::Scope> BenchmarkScopeFixture;

static const std::string scope_id{"does.not.exist.scope"};
static const std::string scope_query_string{"does.not.exist.scope.query_string"};

static const std::string default_locale{"C"};
static const std::string default_form_factor{"SuperDuperPhablet"};

std::pair<
    unity::scopes::testing::Benchmark::Result::Timing::Seconds,
    unity::scopes::testing::Benchmark::Result::Timing::Seconds
> reference_query_performance()
{
    return std::make_pair(
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    mean
                },
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    std::sqrt(variance.count()) // We have to consider std. dev. for the statistical test.
                });
}

std::pair<
    unity::scopes::testing::Benchmark::Result::Timing::Seconds,
    unity::scopes::testing::Benchmark::Result::Timing::Seconds
> reference_preview_performance()
{
    return std::make_pair(
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    mean
                },
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    std::sqrt(variance.count()) // We have to consider std. dev. for the statistical test.
                });
}

std::pair<
    unity::scopes::testing::Benchmark::Result::Timing::Seconds,
    unity::scopes::testing::Benchmark::Result::Timing::Seconds
> reference_activation_performance()
{
    return std::make_pair(
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    mean
                },
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    std::sqrt(variance.count()) // We have to consider std. dev. for the statistical test.
                });
}

std::pair<
    unity::scopes::testing::Benchmark::Result::Timing::Seconds,
    unity::scopes::testing::Benchmark::Result::Timing::Seconds
> reference_action_performance()
{
    return std::make_pair(
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    mean
                },
                unity::scopes::testing::Benchmark::Result::Timing::Seconds
                {
                    std::sqrt(variance.count()) // We have to consider std. dev. for the statistical test.
                });
}

}

TEST(BenchmarkResult, saving_and_loading_works)
{
    const std::string fn{"test.result"};
    std::remove(fn.c_str());

    unity::scopes::testing::Benchmark::Result reference;
    reference.sample_size = 100;

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

TEST(BenchmarkResultXml, saving_and_loading_works)
{
    const std::string fn{"test.result"};
    std::remove(fn.c_str());

    unity::scopes::testing::Benchmark::Result reference;
    reference.sample_size = std::rand();

    {
        std::ofstream out{fn.c_str()};
        ASSERT_NO_THROW(reference.save_to_xml(out));
    }

    {
        unity::scopes::testing::Benchmark::Result result;
        std::ifstream in{fn.c_str()};
        ASSERT_NO_THROW(result.load_from_xml(in));
        EXPECT_EQ(reference, result);
    }
}

// This test relies on real world benchmarking data from previous runs to
// ensure that the performance of the system does not degrade. For that, we work
// under the hypothesis that a change will not result in any significant change in
// performance. If it does result in a change, we fail the test and either have to
// investigate why the performance degraded or take the new results as our new
// performance baseline.
TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_query_performance_oop_works)
{
    unity::scopes::testing::OutOfProcessBenchmark benchmark;

    unity::scopes::testing::Benchmark::Result reference_result;
    {
        std::ifstream in{testing::reference_result_file};
        reference_result.load_from_xml(in);
    }

    unity::scopes::CannedQuery query{scope_id};
    query.set_query_string(scope_query_string);

    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::QueryConfiguration config;
    config.sampler = [query, meta_data]()
    {
        return std::make_pair(query, meta_data);
    };

    auto result = benchmark.for_query(scope, config);

    // We store the potential new reference such that we can copy over if
    // the last test fails.
    std::ofstream out{"ref.xml"};
    result.save_to_xml(out);

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(reference_result.timing));
    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(reference_result.timing));
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
    auto reference = reference_preview_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
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
    auto reference = reference_activation_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
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
    auto reference = reference_action_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
}

TEST_F(BenchmarkScopeFixture, benchmarking_a_scope_query_performance_works)
{
    unity::scopes::testing::InProcessBenchmark benchmark;

    unity::scopes::CannedQuery query{scope_id};
    query.set_query_string(scope_query_string);

    unity::scopes::SearchMetadata meta_data{default_locale, default_form_factor};

    unity::scopes::testing::Benchmark::QueryConfiguration config;
    config.sampler = [query, meta_data]()
    {
        return std::make_pair(query, meta_data);
    };

    auto result = benchmark.for_query(scope, config);
    auto reference = reference_query_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
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
    auto reference = reference_preview_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
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

    auto reference = reference_preview_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
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
    auto reference = reference_preview_performance();

    EXPECT_FALSE(result.timing.is_significantly_faster_than_reference(
                     reference.first.count(),
                     reference.second.count()));

    EXPECT_FALSE(result.timing.is_significantly_slower_than_reference(
                     reference.first.count(),
                     reference.second.count()));
}
