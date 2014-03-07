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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 *              Thomas Voß <thomas.voss@canonical.com>
 */

#include <unity/scopes/ScopeBase.h>

namespace testing
{

class Scope : public unity::scopes::ScopeBase
{
public:
    int start(std::string const&, unity::scopes::RegistryProxy const &) override;

    void stop() override;

    void run() override;

    unity::scopes::SearchQueryBase::UPtr search(
            unity::scopes::CannedQuery const &,
            unity::scopes::SearchMetadata const &) override;

    unity::scopes::ActivationQueryBase::UPtr activate(
            unity::scopes::Result const& result,
            unity::scopes::ActionMetadata const& metadata) override;

    unity::scopes::ActivationQueryBase::UPtr perform_action(
            unity::scopes::Result const& result,
            unity::scopes::ActionMetadata const& metadata,
            std::string const& widget_id,
            std::string const& action_id) override;

    unity::scopes::PreviewQueryBase::UPtr preview(
            unity::scopes::Result const&,
            unity::scopes::ActionMetadata const &) override;
};

} // namespace testing
