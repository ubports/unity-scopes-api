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

#include <unity/scopes/testing/OutOfProcessBenchmark.h>

#include <core/posix/fork.h>

#include <chrono>
#include <iostream>

unity::scopes::testing::Benchmark::Result unity::scopes::testing::OutOfProcessBenchmark::for_query(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::QueryConfiguration config)
{
    auto child = core::posix::fork([this, config, scope]()
    {
        auto result = InProcessBenchmark::for_query(scope, config);
        std::cout << result.sample_size << " " << result.timing.mean.count() << " " << result.timing.std_dev.count();
        return core::posix::exit::Status::success;
    },
    core::posix::StandardStream::stdout);

    unity::scopes::testing::Benchmark::Result result;
    std::size_t sample_size; std::chrono::microseconds::rep mean, std_dev;
    child.cout() >> sample_size >> mean >> std_dev;
    result.sample_size = sample_size;
    result.timing.mean = std::chrono::microseconds{mean};
    result.timing.std_dev = std::chrono::microseconds{std_dev};

    auto wait_result = child.wait_for(core::posix::wait::Flags::untraced);

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

    return result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::OutOfProcessBenchmark::for_preview(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::PreviewConfiguration config)
{
    auto child = core::posix::fork([this, config, scope]()
    {
        auto result = InProcessBenchmark::for_preview(scope, config);
        std::cout << result.sample_size << " " << result.timing.mean.count() << " " << result.timing.std_dev.count();
        return core::posix::exit::Status::success;
    },
    core::posix::StandardStream::stdout);

    unity::scopes::testing::Benchmark::Result result;
    std::size_t sample_size; std::chrono::microseconds::rep mean, std_dev;
    child.cout() >> sample_size >> mean >> std_dev;
    result.sample_size = sample_size;
    result.timing.mean = std::chrono::microseconds{mean};
    result.timing.std_dev = std::chrono::microseconds{std_dev};

    auto wait_result = child.wait_for(core::posix::wait::Flags::untraced);

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

    return result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::OutOfProcessBenchmark::for_activation(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::ActivationConfiguration config)
{
    auto child = core::posix::fork([this, config, scope]()
    {
        auto result = InProcessBenchmark::for_activation(scope, config);
        std::cout << result.sample_size << " " << result.timing.mean.count() << " " << result.timing.std_dev.count();
        return core::posix::exit::Status::success;
    },
    core::posix::StandardStream::stdout);

    unity::scopes::testing::Benchmark::Result result;
    std::size_t sample_size; std::chrono::microseconds::rep mean, std_dev;
    child.cout() >> sample_size >> mean >> std_dev;
    result.sample_size = sample_size;
    result.timing.mean = std::chrono::microseconds{mean};
    result.timing.std_dev = std::chrono::microseconds{std_dev};

    auto wait_result = child.wait_for(core::posix::wait::Flags::untraced);

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

    return result;
}

unity::scopes::testing::Benchmark::Result unity::scopes::testing::OutOfProcessBenchmark::for_action(
        const std::shared_ptr<unity::scopes::ScopeBase>& scope,
        unity::scopes::testing::Benchmark::ActionConfiguration config)
{
    auto child = core::posix::fork([this, config, scope]()
    {
        auto result = InProcessBenchmark::for_action(scope, config);
        std::cout << result.sample_size << " " << result.timing.mean.count() << " " << result.timing.std_dev.count();
        return core::posix::exit::Status::success;
    },
    core::posix::StandardStream::stdout);

    unity::scopes::testing::Benchmark::Result result;
    std::size_t sample_size; std::chrono::microseconds::rep mean, std_dev;
    child.cout() >> sample_size >> mean >> std_dev;
    result.sample_size = sample_size;
    result.timing.mean = std::chrono::microseconds{mean};
    result.timing.std_dev = std::chrono::microseconds{std_dev};

    auto wait_result = child.wait_for(core::posix::wait::Flags::untraced);

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

    return result;
}
