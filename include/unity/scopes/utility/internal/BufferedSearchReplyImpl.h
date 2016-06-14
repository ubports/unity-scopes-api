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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma once

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace unity
{

namespace scopes
{

namespace utility
{

namespace internal
{

class BufferedSearchReplyImpl : public virtual unity::scopes::SearchReply
{
public:
    BufferedSearchReplyImpl(unity::scopes::SearchReplyProxy const& upstream);

    void register_departments(Department::SCPtr const& parent) override;

    Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CategoryRenderer const& renderer_template) override;
    Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CannedQuery const &query,
                                              CategoryRenderer const& renderer_template) override;
    void register_category(Category::SCPtr category) override;
    Category::SCPtr lookup_category(std::string const& id)  override;

    bool push(unity::scopes::experimental::Annotation const& annotation) override;

    bool push(unity::scopes::CategorisedResult const& result) override;
    bool push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state) override;

    void push_surfacing_results_from_cache() noexcept override;

    // Reply interface
    void finished() override;
    void error(std::exception_ptr ex) override;
    void info(OperationInfo const& op_info) override;

    // Object interface
    std::string endpoint() override;
    std::string identity() override;
    std::string target_category() override;
    int64_t timeout() override;
    std::string to_string() override;

    void flush();
    bool push(unity::scopes::Filters const& filters) override;

private:
    std::mutex mutex_;
    unity::scopes::SearchReplyProxy const upstream_;
    std::atomic<bool> buffer_;
    std::vector<CategorisedResult> results_;
};

} // namespace internal

} // namespace utility

} // namespace scopes

} // namespace unity
