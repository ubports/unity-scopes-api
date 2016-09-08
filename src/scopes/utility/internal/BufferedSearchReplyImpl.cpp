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

#include <unity/scopes/utility/internal/BufferedSearchReplyImpl.h>

namespace unity
{

namespace scopes
{

namespace utility
{

namespace internal
{

BufferedSearchReplyImpl::BufferedSearchReplyImpl(unity::scopes::SearchReplyProxy const& upstream)
    : SearchReply(),
      upstream_(upstream),
      buffer_(true)
{
}

void BufferedSearchReplyImpl::register_departments(Department::SCPtr const& parent)
{
    upstream_->register_departments(parent);
}

Category::SCPtr BufferedSearchReplyImpl::register_category(std::string const& id,
        std::string const& title,
        std::string const &icon,
        CategoryRenderer const& renderer_template)
{
    return upstream_->register_category(id, title, icon, renderer_template);
}

Category::SCPtr BufferedSearchReplyImpl::register_category(std::string const& id,
        std::string const& title,
        std::string const &icon,
        CannedQuery const &query,
        CategoryRenderer const& renderer_template)
{
    return upstream_->register_category(id, title, icon, query, renderer_template);
}

void BufferedSearchReplyImpl::register_category(Category::SCPtr category)
{
    return upstream_->register_category(category);
}

Category::SCPtr BufferedSearchReplyImpl::lookup_category(std::string const& id)
{
    return upstream_->lookup_category(id);
}

bool BufferedSearchReplyImpl::push(unity::scopes::experimental::Annotation const& annotation)
{
    return upstream_->push(annotation);
}

bool BufferedSearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_)
        {
            results_.push_back(result);
            return true;
        }
    }
    return upstream_->push(result);
}

bool BufferedSearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    return upstream_->push(filters, filter_state);
}

bool BufferedSearchReplyImpl::push(unity::scopes::Filters const& filters)
{
    return upstream_->push(filters);
}

void BufferedSearchReplyImpl::push_surfacing_results_from_cache() noexcept
{
    upstream_->push_surfacing_results_from_cache();
}

void BufferedSearchReplyImpl::finished()
{
    upstream_->finished();
}

void BufferedSearchReplyImpl::error(std::exception_ptr ex)
{
    upstream_->error(ex);
}

void BufferedSearchReplyImpl::info(OperationInfo const& op_info)
{
    upstream_->info(op_info);
}

std::string BufferedSearchReplyImpl::endpoint()
{
    return upstream_->endpoint();
}

std::string BufferedSearchReplyImpl::identity()
{
    return upstream_->identity();
}

std::string BufferedSearchReplyImpl::target_category()
{
    return upstream_->target_category();
}

int64_t BufferedSearchReplyImpl::timeout()
{
    return upstream_->timeout();
}

std::string BufferedSearchReplyImpl::to_string()
{
    return upstream_->to_string();
}

void BufferedSearchReplyImpl::flush()
{
    std::vector<CategorisedResult> r;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_)
        {
            r.swap(results_);
            buffer_ = false;
        }
    }
    for (auto const& res : r)
    {
        upstream_->push(res);
    }
}

} // namespace internal

} // namespace utility

} // namespace scopes

} // namespace unity
