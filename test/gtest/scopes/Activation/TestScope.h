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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scopes/Category.h>
#include <scopes/CategorisedResult.h>
#include <scopes/Reply.h>
#include <scopes/Runtime.h>
#include <scopes/ScopeBase.h>
#include <scopes/ActivationBase.h>
#include <scopes/ActivationListener.h>

#ifndef UNITY_SCOPES_TEST_SCOPE_H
#define UNITY_SCOPES_TEST_SCOPE_H

namespace unity
{

namespace api
{

namespace scopes
{

class TestQuery : public SearchQuery
{
public:
    virtual void cancelled() override {}

    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_dnd_uri("dnd_uri");
        res.set_intercept_activation();
        reply->push(res);
    }
};

class TestActivation : public ActivationBase
{
public:
    TestActivation(std::string uri, VariantMap hints)
        : uri_(uri),
          recv_hints_(hints)
    {
    }

    virtual ActivationResponse activate() override
    {
        ActivationResponse resp(ActivationResponse::Status::Handled);
        VariantMap var;
        var["foo"] = "bar";
        var["received_hints"] = recv_hints_; // send received hints back for testing
        var["activated_uri"] = uri_; //send activated uri back for testing
        resp.setHints(var);
        return resp;
    }

private:
    std::string uri_;
    VariantMap recv_hints_;
};

class TestScope : public ScopeBase
{
public:
    virtual int start(std::string const&, RegistryProxy const &) override
    {
        return VERSION;
    }

    virtual void stop() override {}

    virtual void run() override {}

    virtual QueryBase::UPtr create_query(std::string const &, VariantMap const &) override
    {
        return QueryBase::UPtr(new TestQuery());
    }

    virtual QueryBase::UPtr preview(Result const&, VariantMap const &) override
    {
        return nullptr;
    }

    virtual ActivationBase::UPtr activate(Result const& result, VariantMap const& hints) override
    {
        return ActivationBase::UPtr(new TestActivation(result.uri(), hints));
    }
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
