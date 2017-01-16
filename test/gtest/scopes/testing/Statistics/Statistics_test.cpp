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

#include <unity/scopes/testing/Benchmark.h>
#include <unity/scopes/testing/Statistics.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "config.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <cstdio>
#include <fstream>
#include <random>

namespace acc = boost::accumulators;

namespace
{
struct Sample : public unity::scopes::testing::Sample
{
    std::vector<double> raw;
    double mean;
    double variance;

    std::size_t get_size() const
    {
        return raw.size();
    }

    ValueType get_mean() const
    {
        return mean;
    }

    ValueType get_variance() const
    {
        return variance;
    }

    void enumerate(const Enumerator& enumerator) const
    {
        for (const auto& observation : raw)
        {
            enumerator(observation);
        }
    }
};

typedef std::chrono::duration<double> Microseconds;
typedef acc::accumulator_set<
    Microseconds::rep,
    acc::stats<
        acc::tag::count,
        acc::tag::mean,
        acc::tag::variance
    >
> Statistics;

template<typename Sampler>
Sample
a_sample_of_size(std::size_t size, Sampler sampler)
{
    Statistics stats
    {
        acc::tag::density::num_bins = 10,
        acc::tag::density::cache_size = 10
    };

    Sample sample;
    sample.raw.resize(size);
    sample.mean = -1.f;
    sample.variance = -1.f;

    for (std::size_t i = 0; i < size; i++)
    {
        auto s = sampler();
        sample.raw[i] = s;
        stats(s);
    }

    sample.mean = acc::mean(stats);
    sample.variance = acc::variance(stats);

    return sample;
}

Sample a_normally_distributed_sample(std::size_t size, double mean = 0, double variance = 1)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    gen.seed(42);
    std::normal_distribution<> normal(mean, variance);

    return a_sample_of_size(size, [&]() { return normal(gen); });
}

Sample a_uniformly_distributed_sample(std::size_t size)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    gen.seed();
    std::uniform_real_distribution<> uniform;

    return a_sample_of_size(size, [&]() { return uniform(gen); });
}

}

TEST(StudentsTTestOneSample, eq_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 0, 1);

    auto result = unity::scopes::testing::StudentsTTest().one_sample(r1, 0., 1.);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.both_means_are_equal(::testing::alpha));
}

TEST(StudentsTTestOneSample, lt_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 0, 1);

    auto result = unity::scopes::testing::StudentsTTest().one_sample(r1, 2., 1.);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.both_means_are_equal(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.sample1_mean_lt_sample2_mean(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.sample1_mean_gt_sample2_mean(::testing::alpha));
}

TEST(StudentsTTestOneSample, gt_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 2, 1);

    auto result = unity::scopes::testing::StudentsTTest().one_sample(r1, 0., 1.);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.both_means_are_equal(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.sample1_mean_lt_sample2_mean(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.sample1_mean_gt_sample2_mean(::testing::alpha));
}

TEST(StudentsTTestTwoSample, eq_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 0, 1);
    auto r2 = a_normally_distributed_sample(10000, 0, 1);

    auto result = unity::scopes::testing::StudentsTTest().two_independent_samples(r1, r2);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.both_means_are_equal(::testing::alpha));
}

TEST(StudentsTTestTwoSample, lt_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 0, 1);
    auto r2 = a_normally_distributed_sample(10000, 2, 1);

    auto result = unity::scopes::testing::StudentsTTest().two_independent_samples(r1, r2);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.both_means_are_equal(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.sample1_mean_lt_sample2_mean(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.sample1_mean_gt_sample2_mean(::testing::alpha));
}

TEST(StudentsTTestTwoSample, gt_is_detected_correctly)
{
    auto r1 = a_normally_distributed_sample(10000, 2, 1);
    auto r2 = a_normally_distributed_sample(10000, 0, 1);

    auto result = unity::scopes::testing::StudentsTTest().two_independent_samples(r1, r2);
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.both_means_are_equal(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              result.sample1_mean_lt_sample2_mean(::testing::alpha));
    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              result.sample1_mean_gt_sample2_mean(::testing::alpha));
}

TEST(AndersonDarlingTest, responds_with_not_rejected_for_data_from_normal_distribution)
{
    auto result = a_normally_distributed_sample(1000);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
              unity::scopes::testing::AndersonDarlingTest()
              .for_normality(result)
              .data_fits_normal_distribution(
                  unity::scopes::testing::Confidence::zero_point_five_percent));
}

TEST(AndersonDarlingTest, responds_with_rejected_for_data_from_uniform_distribution)
{
    auto result = a_uniformly_distributed_sample(1000);

    EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
              unity::scopes::testing::AndersonDarlingTest()
              .for_normality(result)
              .data_fits_normal_distribution(
                  unity::scopes::testing::Confidence::zero_point_five_percent));
}
