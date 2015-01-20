/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "ThrowingScope.h"

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>

#include <condition_variable>
#include <mutex>

using namespace std;
using namespace unity;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query,
              SearchMetadata const& metadata,
              string const& id)
        : SearchQueryBase(query, metadata)
        , id_(id)
        , query_cancelled_(false)
    {
    }

    virtual void cancelled() override
    {
        if (query().query_string() == "throw from cancelled")
        {
            lock_guard<mutex> lock(mutex_);
            query_cancelled_ = true;
            cond_.notify_all();
            throw ResourceException("exception from cancelled");
        }
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        string query_string = query().query_string();
        if (query_string ==  "throw from run")
        {
            throw ResourceException("exception from run");
        }
        else if (query_string == "throw from cancelled")
        {
            unique_lock<mutex> lock(mutex_);
            cond_.wait(lock, [this] { return this->query_cancelled_; });
            return;
        }
        auto cat = reply->register_category(id_, "", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title(query_string);
        if (valid())
        {
            reply->push(res);
        }
    }

private:
    string id_;
    bool query_cancelled_;
    mutex mutex_;
    condition_variable cond_;
};

class TestPreview : public PreviewQueryBase
{
public:
    TestPreview(Result const& result, ActionMetadata const& metadata)
        : PreviewQueryBase(result, metadata)
        , result_(result)
        , preview_cancelled_(false)
    {
    }

    virtual void cancelled() override
    {
        if (result_.title() == "throw from preview cancelled")
        {
            lock_guard<mutex> lock(mutex_);
            preview_cancelled_ = true;
            cond_.notify_all();
            throw ResourceException("exception from preview cancelled");
        }
    }

    virtual void run(PreviewReplyProxy const&) override
    {
        if (result_.title() == "throw from preview run")
        {
            throw ResourceException(result_.title());
        }
        else if (result_.title() == "throw from preview cancelled")
        {
            unique_lock<mutex> lock(mutex_);
            cond_.wait(lock, [this] { return this->preview_cancelled_; });
            return;
        }
    }

private:
    Result result_;
    bool preview_cancelled_;
    mutex mutex_;
    condition_variable cond_;
};

}  // namespace

void ThrowingScope::start(string const& scope_id)
{
    lock_guard<mutex> lock(mutex_);
    id_ = scope_id;
}

void ThrowingScope::stop()
{
}

void ThrowingScope::run()
{
}

SearchQueryBase::UPtr ThrowingScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    if (query.query_string() == "throw from search")
    {
        throw ResourceException("exception from search");
    }
    lock_guard<mutex> lock(mutex_);
    return SearchQueryBase::UPtr(new TestQuery(query, metadata, id_));
}

PreviewQueryBase::UPtr ThrowingScope::preview(Result const& result, ActionMetadata const& metadata)
{
    if (result.title() == "throw from preview")
    {
        throw ResourceException("exception from preview");
    }
    lock_guard<mutex> lock(mutex_);
    return PreviewQueryBase::UPtr(new TestPreview(result, metadata));
}

extern "C"
{

    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new ThrowingScope;
    }

    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }
}
