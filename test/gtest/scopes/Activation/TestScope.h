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

#include <unity/scopes/ActivationListenerBase.h>
#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/internal/SearchReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeBase.h>

#ifndef UNITY_SCOPES_TEST_SCOPE_H
#define UNITY_SCOPES_TEST_SCOPE_H

namespace unity
{

namespace scopes
{

class TestQuery : public SearchQueryBase
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

class TestActivation : public ActivationQueryBase
{
public:
    TestActivation(std::string const& hint, std::string const& hint_val, std::string const &uri, Variant const& hints)
        : hint_key_(hint),
          hint_val_(hint_val),
          uri_(uri),
          recv_hints_(hints)
    {
    }

    virtual ActivationResponse activate() override
    {
        ActivationResponse resp(ActivationResponse::Status::ShowDash);
        VariantMap var;
        var[hint_key_] = hint_val_;
        var["received_hints"] = recv_hints_; // send received hints back for testing
        var["activated_uri"] = uri_; //send activated uri back for testing
        resp.set_scope_data(Variant(var));
        return resp;
    }

private:
    std::string hint_key_;
    std::string hint_val_;
    std::string uri_;
    Variant recv_hints_;
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

    virtual SearchQueryBase::UPtr search(CannedQuery const &, SearchMetadata const &) override
    {
        return SearchQueryBase::UPtr(new TestQuery());
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const &) override
    {
        return nullptr;
    }

    virtual ActivationQueryBase::UPtr activate(Result const& result, ActionMetadata const& hints) override
    {
        return ActivationQueryBase::UPtr(new TestActivation("foo", "bar", result.uri(), hints.scope_data()));
    }

    virtual ActivationQueryBase::UPtr perform_action(Result const& result, ActionMetadata const& hints, std::string const& widget_id, std::string const& action_id) override
    {
        return ActivationQueryBase::UPtr(new TestActivation("activated action", widget_id + action_id, result.uri(), hints.scope_data()));
    }
};

} // namespace scopes

} // namespace unity

#endif
