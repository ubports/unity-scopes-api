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

#include <unity/scopes/testing/Benchmark.h>

#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReplyBase.h>

#include <unity/scopes/internal/ReplyImpl.h>

#include <unity/scopes/testing/Category.h>

#include <core/posix/fork.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
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

struct WaitableReply : public virtual unity::scopes::ReplyBase
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

    void finished() const override
    {
        state.store(State::finished_with_success);
        wait_condition.notify_all();
    }

    void error(std::exception_ptr) const override
    {
        state.store(State::finished_with_error);
        wait_condition.notify_all();
    }
};

struct DevNullPreviewReply : public virtual unity::scopes::PreviewReplyBase, public WaitableReply
{
    bool register_layout(unity::scopes::ColumnLayoutList const&) const override
    {
        return true;
    }

    bool push(unity::scopes::PreviewWidgetList const&) const override
    {
        return true;
    }

    bool push(std::string const&, unity::scopes::Variant const&) const override
    {
        return true;
    }
};

struct DevNullSearchReply : public virtual unity::scopes::SearchReplyBase, public WaitableReply
{
    std::unordered_map<std::string, unity::scopes::Category::SCPtr> categories;

    void register_departments(unity::scopes::DepartmentList const&, std::string)
    {
    }

    unity::scopes::Category::SCPtr register_category(
            std::string const& id,
            std::string const& title,
            std::string const& icon,
            unity::scopes::CategoryRenderer const& renderer)
    {
        auto it = categories.find(id);

        if (it != categories.end())
            return it->second;

        unity::scopes::Category::SCPtr category{new unity::scopes::testing::Category{id, title, icon, renderer}};
        categories[id] = category;

        return category;
    }

    void register_category(unity::scopes::Category::SCPtr)
    {
    }

    unity::scopes::Category::SCPtr lookup_category(std::string const& id) const
    {
        auto it = categories.find(id);

        if (it != categories.end())
            return it->second;

        return unity::scopes::Category::SCPtr{};
    }

    bool push(unity::scopes::CategorisedResult const&) const
    {
        return true;
    }

    bool push(unity::scopes::Filters const&, unity::scopes::FilterState const&) const
    {
        return true;
    }

    bool register_annotation(unity::scopes::Annotation const&) const
    {
        return true;
    }
};

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::microseconds Resolution;

typedef acc::accumulator_set<
    Resolution::rep,
    acc::stats<
        acc::tag::count,
        acc::tag::mean,
        acc::tag::variance
    >
> Statistics;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::Benchmark::for_query(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        const unity::scopes::Query& query,
        const unity::scopes::SearchMetadata& md,
        std::size_t sample_size,
        const std::chrono::microseconds& per_query_timeout)
{
    Statistics stats;

    for (unsigned int i = 0; i < sample_size; i++)
    {
        auto child = core::posix::fork([per_query_timeout, scope, query, md]()
        {
            core::posix::exit::Status exit_status{core::posix::exit::Status::success};
            DevNullSearchReply search_reply;

            auto before = Clock::now();
            {
                auto q = scope->create_query(query, md);
                q->run(unity::scopes::SearchReplyProxy
                {
                    &search_reply,
                    [](unity::scopes::SearchReplyBase* r)
                    {
                        r->finished();
                    }
                });
                if (!search_reply.wait_for_finished_for(per_query_timeout))
                    exit_status = core::posix::exit::Status::failure;
            }
            auto after = Clock::now();

            std::cout << std::chrono::duration_cast<Resolution>(after.time_since_epoch() - before.time_since_epoch()).count() << std::endl;

            return exit_status;
        },
        core::posix::StandardStream::stdout);

        Resolution::rep result; child.cout() >> result;

        auto wait_result = child.wait_for(core::posix::wait::Flags::continued);

        switch(wait_result.status)
        {
        case core::posix::wait::Result::Status::signaled:
        case core::posix::wait::Result::Status::stopped:
            throw std::runtime_error("unity::scopes::testing::Benchmark::for_query: "
                                     "Trial terminated with error, bailing out now. "
                                     "Please see the detailed error output and backtrace.");
        default:
            break;
        }

        if (wait_result.detail.if_exited.status != core::posix::exit::Status::success)
            throw std::runtime_error("unity::scopes::testing::Benchmark::for_query: "
                                     "Trial exited with failure, bailing out now. "
                                     "Please see the detailed error output and backtrace.");

        stats(result);
    }

    unity::scopes::testing::Benchmark::Result result;
    result.sample_size = acc::count(stats);
    result.time.mean = std::chrono::microseconds
    {
        static_cast<Resolution::rep>(acc::mean(stats))
    };
    result.time.std_dev = std::chrono::microseconds
    {
        static_cast<Resolution::rep>(std::sqrt(acc::variance(stats)))
    };

    return result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::Benchmark::for_preview(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        const unity::scopes::Result& result,
        const unity::scopes::ActionMetadata& md,
        std::size_t sample_size,
        const std::chrono::microseconds& per_query_timeout)
{
    Statistics stats;

    for (unsigned int i = 0; i < sample_size; i++)
    {
        auto child = core::posix::fork([per_query_timeout, scope, result, md]()
        {
            core::posix::exit::Status exit_status{core::posix::exit::Status::success};
            DevNullPreviewReply preview_reply;

            auto before = Clock::now();
            {
                auto q = scope->preview(result, md);
                q->run(unity::scopes::PreviewReplyProxy
                {
                    &preview_reply,
                    [](unity::scopes::PreviewReplyBase* r)
                    {
                        r->finished();
                    }
                });
                if (!preview_reply.wait_for_finished_for(per_query_timeout))
                    exit_status = core::posix::exit::Status::failure;
            }
            auto after = Clock::now();

            std::cout << std::chrono::duration_cast<Resolution>(after.time_since_epoch() - before.time_since_epoch()).count() << std::endl;

            return exit_status;
        },
        core::posix::StandardStream::stdout);

        Resolution::rep result; child.cout() >> result;

        auto wait_result = child.wait_for(core::posix::wait::Flags::continued);

        switch(wait_result.status)
        {
        case core::posix::wait::Result::Status::signaled:
        case core::posix::wait::Result::Status::stopped:
            throw std::runtime_error("unity::scopes::testing::Benchmark::for_query: "
                                     "Trial terminated with error, bailing out now. "
                                     "Please see the detailed error output and backtrace.");
        default:
            break;
        }

        if (wait_result.detail.if_exited.status != core::posix::exit::Status::success)
            throw std::runtime_error("unity::scopes::testing::Benchmark::for_query: "
                                     "Trial exited with failure, bailing out now. "
                                     "Please see the detailed error output and backtrace.");

        stats(result);
    }

    unity::scopes::testing::Benchmark::Result benchmark_result;
    benchmark_result.sample_size = acc::count(stats);
    benchmark_result.time.mean = std::chrono::microseconds
    {
        static_cast<Resolution::rep>(acc::mean(stats))
    };
    benchmark_result.time.std_dev = std::chrono::microseconds
    {
        static_cast<Resolution::rep>(std::sqrt(acc::variance(stats)))
    };

    return benchmark_result;
}

std::ostream& unity::scopes::testing::operator<<(std::ostream& out, const unity::scopes::testing::Benchmark::Result& result)
{
    out << "{"
        << "sample_size: " << result.sample_size << ", "
        << "time: {"
        << "µ: " << result.time.mean.count() << " [µs], "
        << "σ: " << result.time.std_dev.count() << " [µs]"
        << "}}";

    return out;
}
