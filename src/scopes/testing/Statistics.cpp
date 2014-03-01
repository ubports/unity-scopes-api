/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/testing/Statistics.h>

#include <boost/math/distributions/students_t.hpp>
#include <boost/math/special_functions/pow.hpp>

#include <cmath>

#include <iostream>

namespace math = boost::math;

unity::scopes::testing::StudentsTTest::Result unity::scopes::testing::StudentsTTest::one_sample(
        const unity::scopes::testing::Benchmark::Result& reference,
        const unity::scopes::testing::Benchmark::Result& sample)
{
    double difference_of_means = reference.timing.mean.count() - sample.timing.mean.count();
    std::size_t degrees_of_freedom = sample.sample_size - 1;
    double t_statistics =
            difference_of_means *
            std::sqrt(static_cast<double>(sample.sample_size)) / math::pow<2>(sample.timing.std_dev.count());

    math::students_t_distribution<> dist(degrees_of_freedom);

    return unity::scopes::testing::StudentsTTest::Result
    {
        [=](double alpha)
        {
            return 2*math::cdf(math::complement(dist, std::fabs(t_statistics))) < alpha ?
                        unity::scopes::testing::HypothesisStatus::not_rejected :
                        unity::scopes::testing::HypothesisStatus::rejected;
        },
        [=](double alpha)
        {
            return math::cdf(dist, t_statistics) < alpha ?
                        unity::scopes::testing::HypothesisStatus::not_rejected :
                        unity::scopes::testing::HypothesisStatus::rejected;
        },
        [=](double alpha)
        {
            return math::cdf(math::complement(dist, t_statistics)) < alpha ?
                        unity::scopes::testing::HypothesisStatus::not_rejected :
                        unity::scopes::testing::HypothesisStatus::rejected;
        },
    };
}
