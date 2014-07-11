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

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/SearchReply.h>

#include <boost/algorithm/string.hpp>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const&) override
    {
    }
};

class MyPreview : public PreviewQueryBase
{
public:
    MyPreview(Result const& result, ActionMetadata const& metadata)
        : PreviewQueryBase(result, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const&) override
    {
    }
};

class MyScope : public ScopeBase
{
public:
    virtual void start(string const&) override
    {
        string ld_lib_path = getenv("LD_LIBRARY_PATH");
        string expected_path_prefix = string(SCOPE_CONFIG_PATH) + ":" + SCOPE_CONFIG_PATH + "/lib";
        // We don't use gtest here because then the scope would have to link against it,
        // but libgtest.a isn't compiled with -fPIC.
        assert(boost::algorithm::starts_with(ld_lib_path, expected_path_prefix));
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata));
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr preview(new MyPreview(result, metadata));
        return preview;
    }
};

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }
}
