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

#pragma once

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/SwitchFilter.h>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace experimental;

class SwitchFilterImpl : public FilterBaseImpl
{
public:
    SwitchFilterImpl(std::string const& id, std::string const& label);
    SwitchFilterImpl(VariantMap const& var);
    std::string label() const;
    bool is_on(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, bool active) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, bool active);
    static SwitchFilter::SPtr create(VariantMap const& var);

protected:
    void validate_display_hints(int hints) const override;
    std::string filter_type() const override;
    void serialize(VariantMap& var) const override;
    void deserialize(VariantMap const& var);

private:
    std::string label_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
