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
#include <unity/scopes/internal/CategorisedResultImpl.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/FilterGroupImpl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>
#include <unity/util/ResourcePtr.h>

#include <cassert>

#include <stdlib.h>
#include <unistd.h>

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
    catch (unity::LogicException const&)
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

bool SearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const&)
{
    return push(filters);
}

bool SearchReplyImpl::push(unity::scopes::Filters const& filters)
{
    // basic consistency check
    try
    {
        internal::FilterBaseImpl::validate_filters(filters);
    }
    catch (unity::LogicException const&)
    {
        throw unity::LogicException("SearchReplyImpl::push(): Failed to validate filters");
    }

    if (query_string_.empty())
    {
        lock_guard<mutex> lock(mutex_);
        cached_filters_ = filters;
    }

    VariantMap var;
    auto filter_groups = internal::FilterGroupImpl::serialize_filter_groups(filters);
    if (filter_groups.size())
    {
        var["filter_groups"] = filter_groups;
    }
    var["filters"] = internal::FilterBaseImpl::serialize_filters(filters);
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

static constexpr char const* cache_file_name = ".surfacing_cache";

void SearchReplyImpl::write_cached_results() noexcept
{
    assert(finished_);

    if (!query_string_.empty())
    {
        return;  // Caching applies only to surfacing queries
    }

    string tmp_path;
    try
    {
        // Open a temporary file for writing.
        tmp_path = mw_proxy_->mw_base()->runtime()->cache_directory() + "/" + cache_file_name + "XXXXXX";
        auto opener = [&tmp_path]()
        {
            int tmp_fd = mkstemp(const_cast<char*>(tmp_path.c_str()));
            if (tmp_fd == -1)
            {
                throw FileException("cannot open tmp file " + tmp_path, errno);
            }
            return tmp_fd;
        };
        auto closer = [&tmp_path](int fd)
        {
            if (::close(fd) == -1)
            {
                // LCOV_EXCL_START
                throw FileException("cannot close tmp file " + tmp_path + " (fd = " + std::to_string(fd) + ")", errno);
                // LCOV_EXCL_STOP
            }
        };
        unity::util::ResourcePtr<int, decltype(closer)> tmp_file(opener(), closer);

        // Turn departments, categories, and results into a JSON string.
        VariantMap departments;
        if (cached_departments_)
        {
            departments = cached_departments_->serialize();
        }
        auto filters = internal::FilterBaseImpl::serialize_filters(cached_filters_);
        VariantArray categories = cat_registry_->serialize();
        VariantArray results;
        for (auto const& r : cached_results_)
        {
            results.push_back(Variant(r.serialize()));
        }
        VariantMap vm;
        vm["departments"] = move(departments);
        vm["categories"] = move(categories);

        // serialize filter groups if present
        auto filter_groups = internal::FilterGroupImpl::serialize_filter_groups(cached_filters_);
        if (filter_groups.size())
        {
            vm["filter_groups"] = move(filter_groups);
        }

        vm["filters"] = move(filters);

        vm["results"] = move(results);
        string const json = Variant(move(vm)).serialize_json();

        // Write tmp file.
        if (::write(tmp_file.get(), json.c_str(), json.size()) != static_cast<int>(json.size()))
        {
            // LCOV_EXCL_START
            throw FileException("cannot write tmp file " + tmp_path + " (fd = " + std::to_string(tmp_file.get()) + ")",
                                errno);
            // LCOV_EXCL_STOP
        }
        tmp_file.dealloc();  // Close tmp file.

        // Atomically replace the old cache with the new one.
        string cache_path = mw_proxy_->mw_base()->runtime()->cache_directory() + "/" + cache_file_name;
        if (rename(tmp_path.c_str(), cache_path.c_str()) == -1)
        {
            throw FileException("cannot rename tmp file " + tmp_path + " to " + cache_path, errno);  // LCOV_EXCL_LINE
        }
    }
    catch (std::exception const& e)
    {
        ::unlink(tmp_path.c_str());
        mw_proxy_->mw_base()->runtime()->logger()() << "SearchReply::write_cached_results(): " << e.what();
    }
    // LCOV_EXCL_START
    catch (...)
    {
        ::unlink(tmp_path.c_str());
        mw_proxy_->mw_base()->runtime()->logger()() << "SearchReply::write_cached_results(): unknown exception";
    }
    // LCOV_EXCL_STOP
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

    string cache_path = mw_proxy_->mw_base()->runtime()->cache_directory() + "/" + cache_file_name;
    try
    {
        // Read cache file.
        string json;
        try
        {
            json = unity::util::read_text_file(cache_path);
        }
        catch (unity::FileException const& e)
        {
            if (e.error() == ENOENT)
            {
                ReplyImpl::finished();
                return;  // No cache has been written yet.
            }
            throw;
        }

        // Decode JSON for the three sections.
        Variant v(Variant::deserialize_json(json));
        VariantMap vm = v.get_dict();

        auto it = vm.find("departments");
        if (it == vm.end())
        {
            throw unity::scopes::NotFoundException("malformed cache file", "departments");
        }
        auto department_dict = it->second.get_dict();

        it = vm.find("categories");
        if (it == vm.end())
        {
            throw unity::scopes::NotFoundException("malformed cache file", "categories");
        }
        auto category_array = it->second.get_array();

        it = vm.find("filters");
        if (it == vm.end())
        {
            throw unity::scopes::NotFoundException("malformed cache file", "filters");
        }
        auto filter_array = it->second.get_array();

        it = vm.find("results");
        if (it == vm.end())
        {
            throw unity::scopes::NotFoundException("malformed cache file", "results");
        }
        auto result_array = it->second.get_array();

        // We have the JSON strings as Variants, re-create the native representations
        // and re-instate them.
        if (!department_dict.empty())
        {
            auto departments = DepartmentImpl::create(move(department_dict));
            register_departments(move(departments));
        }

        for (auto const& c : category_array)
        {
            // Can't use make_shared here because that isn't a friend of Category.
            auto cp = Category::SCPtr(new Category(move(c.get_dict())));
            register_category(cp);
        }

        std::map<std::string, FilterGroup::SCPtr> groups;
        it = vm.find("filter_groups");
        if (it != vm.end())
        {
            groups = FilterGroupImpl::deserialize_filter_groups(it->second.get_array());
        }

        auto filters = FilterBaseImpl::deserialize_filters(move(filter_array), groups);
        push(filters);

        for (auto const& r : result_array)
        {
            VariantMap dict = r.get_dict();
            auto cr = CategorisedResult(new CategorisedResultImpl(*cat_registry_, dict));
            push(cr);
        }
    }
    catch (std::exception const& e)
    {
        mw_proxy_->mw_base()->runtime()->logger()()
            << "SearchReply::push_surfacing_results_from_cache() (file = " + cache_path + "): " << e.what();
    }
    // LCOV_EXCL_START
    catch (...)
    {
        mw_proxy_->mw_base()->runtime()->logger()()
            << "SearchReply::push_surfacing_results_from_cache() (file = " + cache_path + "): unknown exception";
    }
    // LCOV_EXCL_STOP

    // Query is complete.
    ReplyImpl::finished();
}

} // namespace internal

} // namespace scopes

} // namespace unity
