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

#ifndef UNITY_SCOPES_TESTING_STATISTICS_H
#define UNITY_SCOPES_TESTING_STATISTICS_H

#include <unity/scopes/testing/Benchmark.h>

#include <functional>

namespace unity
{

namespace scopes
{
class ScopeBase;

namespace testing
{

/**
 * \brief Summarizes the different outcomes of evaluating a hypothesis at a
 *  given confidence level.
 *
 * Please note that in statistical testing, !(rejected) !=  not_rejected. That is,
 * whenever a hypothesis is rejected, further investigations are required and the
 * conclusion that a hypothesis does not hold is not valid.
 *
 */
enum class HypothesisStatus
{
    rejected, ///< The hypothesis holds at the given confidence level
    not_rejected ///< The hypothesis does not hold at the given confidence level
};

/** \brief We model a hypothesis as a function that can be evaluated for a given confidence value. */
typedef std::function<HypothesisStatus(double)> Hypothesis;

/**
 * \brief Implements different variants of the Student's T-test (see http://en.wikipedia.org/wiki/Student's_t-test)
 *
 * \code
 *
 * unity::scopes::testing::OutOfProcessBenchmark benchmark;
 *
 *  unity::scopes::testing::Result search_result;
 *  unity::scopes::ActionMetadata meta_data{default_locale, default_form_factor};
 *
 *  static const std::size_t sample_size{10};
 *  static const std::chrono::seconds per_trial_timeout{1};
 *
 *  unity::scopes::testing::Benchmark::PreviewConfiguration config
 *  {
 *      [search_result, meta_data]() { return std::make_pair(search_result, meta_data); },
 *      {
 *          sample_size,
 *          per_trial_timeout
 *      }
 *  };
 *
 * auto result = benchmark.for_preview(scope, config);
 *
 *  auto test_result = unity::scopes::testing::StudentsTTest().one_sample(
 *              reference_preview_performance,
 *              result);
 *
 *  EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
 *            test_result.sample_mean_is_eq_to_reference(0.05));
 *  EXPECT_EQ(unity::scopes::testing::HypothesisStatus::not_rejected,
 *            test_result.sample_mean_is_ge_than_reference(0.05));
 *  EXPECT_EQ(unity::scopes::testing::HypothesisStatus::rejected,
 *            test_result.sample_mean_is_le_than_reference(0.05));
 * \endcode
 *
 */
struct StudentsTTest
{
    /**
     * \brief Executing the test returns a set of hypothesis that have to be evaluated
     * at the desired confidence level.
     */
    struct Result
    {
        Hypothesis sample_mean_is_eq_to_reference; ///< H0, both means are equal.
        Hypothesis sample_mean_is_gt_reference; ///< H1, reference mean < sample mean.
        Hypothesis sample_mean_is_lt_reference; ///< H2, sample mean < reference mean.
    };

    /**
     * @brief one_sample calculates the Student's T test for the given reference and sample result.
     * @param reference The reference result as obtained by a previous run.
     * @param sample The current sample of the system performance.
     * @return An instance of Result.
     */
    Result one_sample(const Benchmark::Result& reference,
                      const Benchmark::Result& sample);
};

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
