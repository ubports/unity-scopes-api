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

enum class HypothesisStatus
{
    rejected,
    not_rejected
};

typedef std::function<HypothesisStatus(double)> Hypothesis;

struct StudentsTTest
{
    struct Result
    {
        Hypothesis sample_mean_is_eq_to_reference;
        Hypothesis sample_mean_is_ge_than_reference;
        Hypothesis sample_mean_is_le_than_reference;
    };

    Result one_sample(const Benchmark::Result& reference,
                      const Benchmark::Result& sample);
};

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
