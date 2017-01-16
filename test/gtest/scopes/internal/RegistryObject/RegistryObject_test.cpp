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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gmock/gmock.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace testing;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace core::posix;

namespace
{
class MockExecutor: public virtual Executor
{
public:
    MOCK_METHOD5(exec, core::posix::ChildProcess(const std::string&,
                    const std::vector<std::string>&,
                    const std::map<std::string, std::string>&,
                    const core::posix::StandardStream&,
                    const std::string&));
};

class MockObject: public virtual Object
{
public:
    MOCK_METHOD0(endpoint, std::string());

    MOCK_METHOD0(identity, std::string());

    MOCK_METHOD0(target_category, std::string());

    MOCK_METHOD0(timeout, int64_t());

    MOCK_METHOD0(to_string, std::string());
};

class MockScope: public virtual Scope, public virtual MockObject
{
public:
    MOCK_METHOD3(search, QueryCtrlProxy(std::string const&,
                    SearchMetadata const&,
                    SearchListenerBase::SPtr const&));

    MOCK_METHOD4(search, QueryCtrlProxy(std::string const&,
                    FilterState const&,
                    SearchMetadata const&,
                    SearchListenerBase::SPtr const&));

    MOCK_METHOD5(search, QueryCtrlProxy(std::string const&,
                    std::string const&,
                    FilterState const&,
                    SearchMetadata const&,
                    SearchListenerBase::SPtr const&));

    MOCK_METHOD3(activate, QueryCtrlProxy(Result const&,
                    ActionMetadata const&,
                    ActivationListenerBase::SPtr const&));

    MOCK_METHOD5(perform_action, QueryCtrlProxy(Result const& result,
                    ActionMetadata const&,
                    std::string const&,
                    std::string const&,
                    ActivationListenerBase::SPtr const&));

    MOCK_METHOD3(preview, QueryCtrlProxy(Result const&,
                    ActionMetadata const&,
                    PreviewListenerBase::SPtr const&));

    MOCK_METHOD0(child_scopes, ChildScopeList());

    MOCK_METHOD6(search, QueryCtrlProxy(std::string const&,
                    std::string const&,
                    FilterState const&,
                    Variant const&,
                    SearchMetadata const&,
                    SearchListenerBase::SPtr const&));
    MOCK_METHOD1(set_child_scopes, bool(ChildScopeList const&));
    MOCK_METHOD4(activate_result_action, QueryCtrlProxy(unity::scopes::Result const&,
                                                        ActionMetadata const&,
                                                        std::string const&,
                                                        ActivationListenerBase::SPtr const&));
};


class TestRegistryObject: public Test
{
public:
    core::posix::ChildProcess mock_exec(const string&, const vector<string>&,
            const map<string, string>&, const core::posix::StandardStream&,
            const string&)
    {
        t_start.reset(new thread(pretend_started, registry->state_receiver()));
        return dummy_process;
    }

protected:
    void TearDown() override
    {
        ASSERT_TRUE(t_start.get());
        t_start->join();

        dummy_process.send_signal_or_throw(core::posix::Signal::sig_term);
    }

    static void pretend_started(StateReceiverObject::SPtr receiver)
    {
        receiver->push_state("scope-id", StateReceiverObject::State::ScopeReady);
    }

    ScopeMetadata make_meta(const string& scope_id)
    {
        unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(nullptr));

        mi->set_scope_id(scope_id);
        mi->set_art("art " + scope_id);
        mi->set_display_name("display name " + scope_id);
        mi->set_description("description " + scope_id);
        mi->set_author("author " + scope_id);
        mi->set_search_hint("search hint " + scope_id);
        mi->set_hot_key("hot key " + scope_id);
        mi->set_scope_directory("/foo");

        mi->set_proxy(make_shared<StrictMock<MockScope>>());

        return ScopeMetadataImpl::create(move(mi));
    }

    std::shared_ptr<ChildProcess::DeathObserver> death_observer()
    {
        if (!death_observer_)
        {
            std::shared_ptr<SignalTrap> trap(
                    core::posix::trap_signals_for_all_subsequent_threads(
                    { core::posix::Signal::sig_chld }));

            death_observer_ = std::move(
                    ChildProcess::DeathObserver::create_once_with_signal_trap(
                            trap));
        }
        return death_observer_;
    }

    void run_registry(string const& confinement_profile)
    {
        ScopeMetadata meta = make_meta("scope-id");

        RegistryObject::ScopeExecData exec_data;
        exec_data.scope_id = "scope-id";
        exec_data.scoperunner_path = "/path/scoperunner";
        exec_data.runtime_config = "/path/runtime.ini";
        exec_data.scope_config = "scope.ini";
        exec_data.confinement_profile = confinement_profile;
        exec_data.timeout_ms = 1500;

        registry.reset(new RegistryObject(*death_observer(), executor, nullptr));
        registry->add_local_scope("scope-id", meta, exec_data);
        registry->locate("scope-id");
        EXPECT_TRUE(registry->is_scope_running("scope-id"));
    }

    static std::shared_ptr<ChildProcess::DeathObserver> death_observer_;

    shared_ptr<MockExecutor> executor = make_shared<StrictMock<MockExecutor>>();

    ChildProcess dummy_process = core::posix::exec("/bin/sleep", vector<string>
        { "30" }, map<string, string>(), StandardStream::empty);

    unique_ptr<thread> t_start;

    unique_ptr<RegistryObject> registry;
};

std::shared_ptr<ChildProcess::DeathObserver> TestRegistryObject::death_observer_;

TEST_F(TestRegistryObject, basic)
{
    EXPECT_CALL(*executor,
            exec("/path/scoperunner", vector<string>
                    {   "/path/runtime.ini", "scope.ini"}, _,
                    StandardStream::stdin,
                    string())).WillOnce(
            Invoke(this, &TestRegistryObject::mock_exec));

    run_registry(string());
}

TEST_F(TestRegistryObject, confined)
{
    EXPECT_CALL(*executor,
            exec("/path/scoperunner", vector<string>
                    {   "/path/runtime.ini", "scope.ini"}, _,
                    StandardStream::stdin,
                    "confinement profile")).WillOnce(
            Invoke(this, &TestRegistryObject::mock_exec));

    run_registry("confinement profile");
}

}
