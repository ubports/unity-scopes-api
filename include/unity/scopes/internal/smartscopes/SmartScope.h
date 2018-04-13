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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/scopes/ScopeBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartQuery : public SearchQueryBase
{
public:
    SmartQuery(std::string const& scope_id, SSRegistryObject::SPtr reg, CannedQuery const& query, SearchMetadata const& hints);
    ~SmartQuery();

    virtual void cancelled() override;
    virtual void run(SearchReplyProxy const& reply) override;

    Department::SCPtr create_department(std::shared_ptr<DepartmentInfo const> const& deptinfo);

private:
    std::string scope_id_;
    CannedQuery query_;
    SearchHandle::UPtr search_handle_;
    std::string base_url_;
    SearchMetadata hints_;
};

class SmartPreview : public PreviewQueryBase
{
public:
    SmartPreview(std::string const& scope_id, SSRegistryObject::SPtr reg, Result const& result, ActionMetadata const& hints);
    ~SmartPreview();

    virtual void cancelled() override;
    virtual void run(PreviewReplyProxy const& reply) override;

private:
    std::string scope_id_;
    PreviewHandle::UPtr preview_handle_;
    Result result_;
    std::string base_url_;
    ActionMetadata hints_;
};

class SmartActivation : public ActivationQueryBase
{
public:
    SmartActivation(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id);
    ActivationResponse activate() override;
};

class NullActivation : public ActivationQueryBase
{
public:
    NullActivation(Result const& result, ActionMetadata const& metadata);
    ActivationResponse activate() override;
};

class SmartScope
{
public:
    SmartScope(SSRegistryObject::SPtr reg);

    SearchQueryBase::UPtr search(std::string const& scope_id, CannedQuery const& q, SearchMetadata const& hints);
    QueryBase::UPtr preview(std::string const& scope_id, Result const& result, ActionMetadata const& hints);
    ActivationQueryBase::UPtr activate(std::string const&, Result const& result, ActionMetadata const& metadata);
    ActivationQueryBase::UPtr perform_action(std::string const& scope_id, Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id);
    ActivationQueryBase::UPtr activate_result_action(std::string const& scope_id, Result const& result, ActionMetadata const& metadata, std::string const& action_id);

private:
    SSRegistryObject::SPtr reg_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity
