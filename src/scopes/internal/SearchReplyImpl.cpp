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
 *              Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/SearchReplyImpl.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/util/FileIO.h>
#include <unity/UnityExceptions.h>

#include <stdlib.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

SearchReplyImpl::SearchReplyImpl(MWReplyProxy const& mw_proxy,
                                 shared_ptr<QueryObjectBase> const& qo,
                                 int cardinality,
                                 string const& query_string,
                                 string const& current_department_id)
    : ObjectImpl(mw_proxy)
    , ReplyImpl(mw_proxy, qo)
    , cat_registry_(new CategoryRegistry())
    , cardinality_(cardinality)
    , num_pushes_(0)
    , finished_(false)
    , query_string_(query_string)
    , current_department_(current_department_id)
{
}

SearchReplyImpl::~SearchReplyImpl()
{
    finished();
}

void SearchReplyImpl::register_departments(Department::SCPtr const& parent)
{
    // basic consistency check
    try
    {
        DepartmentImpl::validate_departments(parent, current_department_);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("SearchReplyImpl::register_departments(): Failed to validate departments");
    }

    if (query_string_.empty())
    {
        lock_guard<mutex> lock(mutex_);
        cached_departments_ = parent;
    }

    ReplyImpl::push(internal::DepartmentImpl::serialize_departments(parent)); // ignore return value?
}

void SearchReplyImpl::register_category(Category::SCPtr category)
{
    cat_registry_->register_category(category); // will throw if that category id has already been registered
    push(category);
}

Category::SCPtr SearchReplyImpl::register_category(string const& id,
                                                   string const& title,
                                                   string const &icon,
                                                   CategoryRenderer const& renderer_template)
{
    // will throw if adding same category again
    auto cat = cat_registry_->register_category(id, title, icon, nullptr, renderer_template);
    push(cat);
    return cat;
}

Category::SCPtr SearchReplyImpl::register_category(string const& id,
                                                   string const& title,
                                                   string const& icon,
                                                   CannedQuery const& query,
                                                   CategoryRenderer const& renderer_template)
{
    // will throw if adding same category again
    auto cat = cat_registry_->register_category(id, title, icon, make_shared<CannedQuery>(query), renderer_template);
    push(cat);
    return cat;
}

Category::SCPtr SearchReplyImpl::lookup_category(string const& id)
{
    return cat_registry_->lookup_category(id);
}

bool SearchReplyImpl::push(unity::scopes::experimental::Annotation const& annotation)
{
    VariantMap var;
    var["annotation"] = annotation.serialize();
    return ReplyImpl::push(var);
}

bool SearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    // If this is an aggregator scope, it may be pushing result items obtained
    // from ReplyObject without registering a category first.
    if (cat_registry_->lookup_category(result.category()->id()) == nullptr)
    {
        register_category(result.category());
    }

    VariantMap var;
    var["result"] = result.serialize();
    if (!ReplyImpl::push(var))
    {
        return false;
    }

    // We cache the results of surfacing queries so, if a scope has not connectivity,
    // we can replay the results of the last successful surfacing query.
    if (query_string_.empty())
    {
        lock_guard<mutex> lock(mutex_);
        cached_results_.push_back(result);
    }

    // Enforce cardinality limit (0 means no limit). If the scope pushes more results
    // than requested, future pushes are ignored. push() returns false
    // on the last call that actually still pushed a result.
    // To the client, a query that exceeds the limit looks like a query
    // that returned the maximum number of results and finished normally.
    if (++num_pushes_ == cardinality_)
    {
        // At most one thread will execute this.
        finished();
        return false;  // This was the last successful push
    }
    return true;
}

bool SearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    // basic consistency check
    try
    {
        internal::FilterBaseImpl::validate_filters(filters);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("SearchReplyImpl::push(): Failed to validate filters");
    }

    VariantMap var;
    var["filters"] = internal::FilterBaseImpl::serialize_filters(filters);
    var["filter_state"] = filter_state.serialize();
    return ReplyImpl::push(var);
}

bool SearchReplyImpl::push(Category::SCPtr category)
{
    VariantMap var;
    var["category"] = category->serialize();
    return ReplyImpl::push(var);
}

void SearchReplyImpl::finished()
{
    if (finished_.exchange(true))
    {
        return;
    }
    write_cached_results();
    ReplyImpl::finished();  // Upcall to deal with the normal case
}

namespace
{
    static constexpr char const* cache_file_name = ".surfacing_cache";
}

void SearchReplyImpl::write_cached_results() noexcept
{
    try
    {
        assert(finished_);

        if (!query_string_.empty())
        {
            return;  // Caching applies only to surfacing queries
        }

        // Open a temporary file for writing.
        string tmp_path = mw_proxy_->mw_base()->runtime()->tmp_directory() + "/cached_resultsXXXXXX";
        int tmp_fd = mkstemp(const_cast<char*>(tmp_path.c_str()));
        if (tmp_fd == -1)
        {
            throw SyscallException("SearchReply::write_cached_results(): cannot open tmp file " + tmp_path, errno);
        }
        auto closer = [tmp_path](int fd) {
            if (close(fd) == -1)
            {
                ::unlink(tmp_path.c_str());
                throw SyscallException("cannot close tmp file " + tmp_path, errno);
            }
        };
        unity::util::ResourcePtr<int, decltype(closer)> tmp_file(tmp_fd, closer);

        for (auto&& r : cached_results_)
        {
            // TODO: Write cached results to disk here.
        }

        tmp_file.dealloc();  // Close tmp file.

        // Atomically replace the old cache with the new one.
        string cache_path = mw_proxy_->mw_base()->runtime()->cache_directory() + "/" + cache_file_name;
        if (rename(tmp_path.c_str(), cache_path.c_str()) == -1)
        {
            ::unlink(tmp_path.c_str());
            throw SyscallException("SearchReply::write_cached_results(): cannot rename tmp file "
                                   + tmp_path + " to " + cache_path,
                                   errno);
        }
    }
    catch (std::exception const& e)
    {
        BOOST_LOG(mw_proxy_->mw_base()->runtime()->logger()) << e.what();
    }
    catch (...)
    {
        BOOST_LOG(mw_proxy_->mw_base()->runtime()->logger())
            << "SearchReply::write_cached_results(): unknown exception";
    }
}

void SearchReplyImpl::push_surfacing_results_from_cache() noexcept
{
    if (!query_string_.empty())
    {
        return;  // Caching applies only to surfacing queries
    }

    if (finished_.exchange(true))
    {
        return;
    }

    try
    {
        string cache_path = mw_proxy_->mw_base()->runtime()->cache_directory() + "/" + cache_file_name;
        try
        {
            string cache_str = unity::util::read_text_file(cache_path);
        }
        catch (unity::FileException const& e)
        {
            if (e.error() == ENOENT)
            {
                return;  // No cache has been written yet.
            }
            throw;
        }

        // TODO: Open cache file (if one exists), deserialize, and push.

        ReplyImpl::finished();  // Sends finished() message to client
    }
    catch (std::exception const& e)
    {
        BOOST_LOG(mw_proxy_->mw_base()->runtime()->logger())
            << "SearchReply::push_surfacing_results_from_cache(): " << e.what();
    }
    catch (...)
    {
        BOOST_LOG(mw_proxy_->mw_base()->runtime()->logger())
            << "SearchReply::push_surfacing_results_from_cache(): unknown exception";
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
