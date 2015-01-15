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

#include <unity/scopes/testing/InProcessBenchmark.h>

#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/testing/Category.h>

#include <core/posix/fork.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace acc = boost::accumulators;

namespace
{
constexpr static const int result_idx = 0;
constexpr static const int metadata_idx = 1;
constexpr static const int widget_idx = 2;
constexpr static const int action_idx = 3;

struct ObjectImpl : public virtual unity::scopes::Object
{
    std::string endpoint() override
    {
        return "";
    }

    std::string identity() override
    {
        return "";
    }

    std::string target_category() override
    {
        return "";
    }

    int64_t timeout() override
    {
        return -1;
    }

    std::string to_string() override
    {
        return "";
    }
};

struct WaitableReply : public virtual unity::scopes::Reply, public ObjectImpl
{
    enum class State
    {
        idle,
        running,
        finished_with_success,
        finished_with_error
    };

    std::mutex guard;
    mutable std::condition_variable wait_condition;
    mutable std::atomic<State> state{State::idle};

    bool is_finished()
    {
        return state.load() == State::finished_with_success || state.load() == State::finished_with_error;
    }

    bool is_finished_with_error()
    {
        return state.load() == State::finished_with_error;
    }

    bool is_finished_with_success()
    {
        return state.load() == State::finished_with_success;
    }

    bool wait_for_finished_for(const std::chrono::microseconds& duration)
    {
        if (is_finished())
            return true;

        std::unique_lock<std::mutex> ul(guard);
        return wait_condition.wait_for(ul, duration, [this]()
        {
            return is_finished();
        });
    }

    void finished() override
    {
        state.store(State::finished_with_success);
        wait_condition.notify_all();
    }

    void error(std::exception_ptr) override
    {
        state.store(State::finished_with_error);
        wait_condition.notify_all();
    }

    void info(unity::scopes::OperationInfo const&) override
    {
    }
};

struct DevNullPreviewReply : public unity::scopes::PreviewReply, public WaitableReply
{
    bool register_layout(unity::scopes::ColumnLayoutList const&) override
    {
        return true;
    }

    bool push(unity::scopes::PreviewWidgetList const&) override
    {
        return true;
    }

    bool push(std::string const&, unity::scopes::Variant const&) override
    {
        return true;
    }
};

struct DevNullSearchReply : public unity::scopes::SearchReply, public WaitableReply
{
    unity::scopes::internal::CategoryRegistry category_registry;

    void register_departments(unity::scopes::Department::SCPtr const&) override
    {
    }

    unity::scopes::Category::SCPtr register_category(
            std::string const& id,
            std::string const& title,
            std::string const& icon,
            unity::scopes::CategoryRenderer const& renderer) override
    {
        return category_registry.register_category(id, title, icon, nullptr, renderer);
    }

    unity::scopes::Category::SCPtr register_category(
            std::string const& id,
            std::string const& title,
            std::string const& icon,
            unity::scopes::CannedQuery const& query,
            unity::scopes::CategoryRenderer const& renderer) override
    {
        return category_registry.register_category(id, title, icon, std::make_shared<unity::scopes::CannedQuery>(query), renderer);
    }

    void register_category(unity::scopes::Category::SCPtr category) override
    {
        category_registry.register_category(category);
    }

    unity::scopes::Category::SCPtr lookup_category(std::string const& id) override
    {
        return category_registry.lookup_category(id);
    }

    bool push(unity::scopes::CategorisedResult const&) override
    {
        return true;
    }

    bool push(unity::scopes::Filters const&, unity::scopes::FilterState const&) override
    {
        return true;
    }

    bool push(unity::scopes::experimental::Annotation const&) override
    {
        return true;
    }
};

typedef std::chrono::high_resolution_clock Clock;
typedef unity::scopes::testing::Benchmark::Result::Timing::Seconds Resolution;

typedef acc::accumulator_set<
    Resolution::rep,
    acc::stats<
        acc::tag::count,
        acc::tag::density,
        acc::tag::min,
        acc::tag::max,
        acc::tag::mean,
        acc::tag::kurtosis,
        acc::tag::skewness,
        acc::tag::variance
    >
> Statistics;

void fill_results_from_statistics(unity::scopes::testing::Benchmark::Result& result,
                                  const Statistics& stats)
{
    result.sample_size = acc::count(stats);

    auto histogram = acc::density(stats);

    for (const auto& bin : histogram)
    {
        result.timing.histogram.push_back(
                    std::make_pair(
                        Resolution(bin.first),
                        bin.second));
    }

    result.timing.min = Resolution
    {
        static_cast<Resolution::rep>(acc::min(stats))
    };
    result.timing.max = Resolution
    {
        static_cast<Resolution::rep>(acc::max(stats))
    };
    result.timing.kurtosis = Resolution
    {
        static_cast<Resolution::rep>(acc::kurtosis(stats))
    };
    result.timing.skewness = Resolution
    {
        static_cast<Resolution::rep>(acc::skewness(stats))
    };
    result.timing.mean = Resolution
    {
        static_cast<Resolution::rep>(acc::mean(stats))
    };
    result.timing.std_dev = Resolution
    {
        static_cast<Resolution::rep>(std::sqrt(acc::variance(stats)))
    };
}
}

/// @cond

unity::scopes::testing::Benchmark::Result unity::scopes::testing::InProcessBenchmark::for_query(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::QueryConfiguration config)
{
    Statistics stats(
                acc::tag::density::num_bins = config.trial_configuration.statistics_configuration.histogram_bin_count,
                acc::tag::density::cache_size = 10);

    unity::scopes::testing::Benchmark::Result benchmark_result;
    benchmark_result.timing.sample.resize(config.trial_configuration.trial_count);

    for (unsigned int i = 0; i < config.trial_configuration.trial_count; i++)
    {
        DevNullSearchReply search_reply;

        auto before = Clock::now();
        {
            auto sample = config.sampler();
            auto q = scope->search(sample.first, sample.second);

            q->run(unity::scopes::SearchReplyProxy
            {
                &search_reply,
                [](unity::scopes::SearchReply* r)
                {
                    r->finished();
                }
            });
            if (!search_reply.wait_for_finished_for(config.trial_configuration.per_trial_timeout))
                throw std::runtime_error("Query did not complete within the specified timeout interval.");
        }
        auto after = Clock::now();

        auto duration = std::chrono::duration_cast<Resolution>(after - before);
        stats(duration.count());
        benchmark_result.timing.sample[i] = duration;
    }

    fill_results_from_statistics(benchmark_result, stats);
    return benchmark_result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::InProcessBenchmark::for_preview(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::PreviewConfiguration config)
{
    Statistics stats(
                acc::tag::density::num_bins = config.trial_configuration.statistics_configuration.histogram_bin_count,
                acc::tag::density::cache_size = 10);

    unity::scopes::testing::Benchmark::Result benchmark_result;
    benchmark_result.timing.sample.resize(config.trial_configuration.trial_count);

    for (unsigned int i = 0; i < config.trial_configuration.trial_count; i++)
    {
            DevNullPreviewReply preview_reply;

            auto before = Clock::now();
            {
                auto sample = config.sampler();

                auto q = scope->preview(sample.first, sample.second);
                q->run(unity::scopes::PreviewReplyProxy
                {
                    &preview_reply,
                    [](unity::scopes::PreviewReply* r)
                    {
                        r->finished();
                    }
                });
                if (!preview_reply.wait_for_finished_for(config.trial_configuration.per_trial_timeout))
                    throw std::runtime_error("Preview did not complete within the specified timeout interval.");
            }
            auto after = Clock::now();

            auto duration = std::chrono::duration_cast<Resolution>(after - before);
            stats(duration.count());
            benchmark_result.timing.sample[i] = duration;
    }

    fill_results_from_statistics(benchmark_result, stats);
    return benchmark_result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::InProcessBenchmark::for_activation(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::ActivationConfiguration config)
{
    Statistics stats(
                acc::tag::density::num_bins = config.trial_configuration.statistics_configuration.histogram_bin_count,
                acc::tag::density::cache_size = 10);

    unity::scopes::testing::Benchmark::Result benchmark_result;
    benchmark_result.timing.sample.resize(config.trial_configuration.trial_count);

    for (unsigned int i = 0; i < config.trial_configuration.trial_count; i++)
    {
            auto before = Clock::now();
            {
                auto sample = config.sampler();
                auto a = scope->activate(sample.first, sample.second);
                (void) a->activate();
            }
            auto after = Clock::now();

            auto duration = std::chrono::duration_cast<Resolution>(after - before);
            stats(duration.count());
            benchmark_result.timing.sample[i] = duration;
    }

    fill_results_from_statistics(benchmark_result, stats);
    return benchmark_result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::InProcessBenchmark::for_action(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::ActionConfiguration config)
{
    Statistics stats(
                acc::tag::density::num_bins = config.trial_configuration.statistics_configuration.histogram_bin_count,
                acc::tag::density::cache_size = 10);

    unity::scopes::testing::Benchmark::Result benchmark_result;
    benchmark_result.timing.sample.resize(config.trial_configuration.trial_count);

    for (unsigned int i = 0; i < config.trial_configuration.trial_count; i++)
    {
            auto before = Clock::now();
            {
                auto sample = config.sampler();
                auto a = scope->perform_action(std::get<result_idx>(sample),
                                               std::get<metadata_idx>(sample),
                                               std::get<widget_idx>(sample),
                                               std::get<action_idx>(sample));
                (void) a->activate();
            }
            auto after = Clock::now();

            auto duration = std::chrono::duration_cast<Resolution>(after - before);
            stats(duration.count());
            benchmark_result.timing.sample[i] = duration;
    }

    fill_results_from_statistics(benchmark_result, stats);

    return benchmark_result;
}

/// @endcond
