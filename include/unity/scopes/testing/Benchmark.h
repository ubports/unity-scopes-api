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
#include <unity/scopes/Query.h>
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
/**
 * \brief The Benchmark class defines an interface to provide scope authors with runtime benchmarking capabilities
 * to be used in their own testing.
 */
class Benchmark
{
public:
    /**
     * \brief The Result struct encapsulates all of the result gathered from one
     * individual benchmark run consisting of multiple independent trials.
     */
    struct Result
    {
        /** Size of the sample, corresponds to number of trials. */
        std::size_t sample_size{0};
        /** Timing characteristics captured during the benchmark run. */
        struct Timing
        {
            /** Minimum execution time for the benchmarked operation. */
            std::chrono::microseconds min{std::chrono::microseconds::min()};
            /** Maximum execution time for the benchmarked operation. */
            std::chrono::microseconds max{std::chrono::microseconds::min()};
            /** Mean execution time for the benchmarked operation. */
            std::chrono::microseconds mean{std::chrono::microseconds::min()};
            /** Std. deviation in execution time for the benchmarked operation. */
            std::chrono::microseconds std_dev{std::chrono::microseconds::min()};
            /** Kurtosis in execution time for the benchmarked operation. */
            std::chrono::microseconds kurtosis{std::chrono::microseconds::min()};
            /** Skewness in execution time for the benchmarked operation. */
            std::chrono::microseconds skewness{std::chrono::microseconds::min()};
            /** Histogram of measured execution times for the benchmarked operation. */
            std::vector<std::pair<std::chrono::microseconds, double>> histogram{};
            /** Raw sample vector, with sample.size() == sample_size */
            std::vector<std::chrono::microseconds> sample{};
        } timing{};

        /**
         * \brief load_from restores a result from the given input stream.
         * \throw std::runtime_error in case of issues.
         * \param in The stream to read from.
         */
        void load_from(std::istream& in);

        /**
         * \brief save_to stores a result to the given output stream.
         * \throw std::runtime_error in case of issues.
         * \param out The stream to write to.
         */
        void save_to(std::ostream& out);

        /**
         * \brief load_from_xml restores a result stored as xml from the given input stream.
         * \throw std::runtime_error in case of issues.
         * \param in The stream to read from.
         */
        void load_from_xml(std::istream& in);

        /**
         * \brief save_to_xml stores a result as xml to the given output stream.
         * \throw std::runtime_error in case of issues.
         * \param out The stream to write to.
         */
        void save_to_xml(std::ostream& out);
    };

    /**
     * \brief The StatisticsConfiguration struct contains options controlling
     * the calculation of benchmark result statistics.
     */
    struct StatisticsConfiguration
    {
        /** Number of bins in the final histogram. */
        std::size_t histogram_bin_count{10};
    };

    /**
     * \brief The TrialConfiguration struct contains options controlling
     * the execution of individual trials.
     */
    struct TrialConfiguration
    {
        /** The number of independent trials. Please note that the number should not be << 10 */
        std::size_t trial_count{10};
        /** Wait at most this time for one trial to finish or throw if a timeout is encountered. */
        std::chrono::microseconds per_trial_timeout{std::chrono::seconds{10}};
        /** Fold in statistics configuration into the overall trial setup. */
        StatisticsConfiguration statistics_configuration{};
    };

    /**
     * \brief The QueryConfiguration struct constains all options controlling the
     * benchmark of scope query operations.
     */
    struct QueryConfiguration
    {
        /** Function signature for choosing a query configuration. */
        typedef std::function<
            std::pair<
                unity::scopes::Query,
                unity::scopes::SearchMetadata>()
        > Sampler;

        /**
         * The sampling function instance for choosing a query configuration.
         * Has to be set to an actual instance.
         */
        Sampler sampler{};
        /** fold in trial configuration options into the overall setup. */
        TrialConfiguration trial_configuration{};
    };

    /**
     * \brief The PreviewConfiguration struct constains all options controlling the
     * benchmark of scope preview operations.
     */
    struct PreviewConfiguration
    {
        /** Function signature for choosing a preview configuration. */
        typedef std::function<
            std::pair<
                unity::scopes::Result,
                unity::scopes::ActionMetadata
            >()
        > Sampler;

        /**
         * The sampling function instance for choosing a preview configuration.
         * Has to be set to an actual instance.
         */
        Sampler sampler{};
        /** fold in trial configuration options into the overall setup. */
        TrialConfiguration trial_configuration{};
    };

    /**
     * \brief The ActivationConfiguration struct constains all options controlling the
     * benchmark of scope activation operations.
     */
    struct ActivationConfiguration
    {
        /** Function signature for choosing an activation configuration. */
        typedef std::function<
            std::pair<
                unity::scopes::Result,
                unity::scopes::ActionMetadata>()
        > Sampler;

        /**
         * The sampling function instance for choosing a preview configuration.
         * Has to be set to an actual instance.
         */
        Sampler sampler{};
        /** fold in trial configuration options into the overall setup. */
        TrialConfiguration trial_configuration{};
    };

    /**
     * \brief The ActionConfiguration struct constains all options controlling the
     * benchmark of scope action activation operations.
     */
    struct ActionConfiguration
    {
        /** Function signature for choosing an action invocation configuration. */
        typedef std::function<
            std::tuple<
                unity::scopes::Result,
                unity::scopes::ActionMetadata,
                std::string,
                std::string
            >()
        > Sampler;

        /**
         * The sampling function instance for choosing an action activation configuration.
         * Has to be set to an actual instance.
         */
        Sampler sampler{};
        /** fold in trial configuration options into the overall setup. */
        TrialConfiguration trial_configuration;
    };

    /** \cond */
    virtual ~Benchmark() = default;
    Benchmark(const Benchmark&) = delete;
    Benchmark(Benchmark&&) = delete;

    Benchmark& operator=(const Benchmark&) = delete;
    Benchmark& operator=(Benchmark&&) = delete;
    /** \endcond */

    /**
     * \brief for_query executes a benchmark to measure the scope's query performance.
     * \throw std::runtime_error in case of timeouts.
     * \throw std::logic_error in case of misconfiguration.
     * \param scope The scope instance to benchmark.
     * \param configuration Options controlling the experiment.
     * \return An instance of Result.
     */
    virtual Result for_query(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                             QueryConfiguration configuration) = 0;

    /**
     * \brief for_preview executes a benchmark to measure the scope's preview performance.
     * \throw std::runtime_error in case of timeouts.
     * \throw std::logic_error in case of misconfiguration.
     * \param scope The scope instance to benchmark.
     * \param configuration Options controlling the experiment.
     * \return An instance of Result.
     */
    virtual Result for_preview(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                               PreviewConfiguration preview_configuration) = 0;

    /**
     * \brief for_preview executes a benchmark to measure the scope's activation performance.
     * \throw std::runtime_error in case of timeouts.
     * \throw std::logic_error in case of misconfiguration.
     * \param scope The scope instance to benchmark.
     * \param configuration Options controlling the experiment.
     * \return An instance of Result.
     */
    virtual Result for_activation(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                                  ActivationConfiguration activation_configuration) = 0;

    /**
     * \brief for_preview executes a benchmark to measure the scope's action activation performance.
     * \throw std::runtime_error in case of timeouts.
     * \throw std::logic_error in case of misconfiguration.
     * \param scope The scope instance to benchmark.
     * \param configuration Options controlling the experiment.
     * \return An instance of Result.
     */
    virtual Result for_action(const std::shared_ptr<unity::scopes::ScopeBase>& scope,
                              ActionConfiguration activation_configuration) = 0;

protected:
    Benchmark() = default;
};

bool operator==(const Benchmark::Result& lhs, const Benchmark::Result& rhs);

std::ostream& operator<<(std::ostream&, const Benchmark::Result&);

} // namespace testing

} // namespace scopes

} // namespace unity

#endif
