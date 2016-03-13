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

#pragma once

#include <unity/scopes/Variant.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterGroup.h>
#include <string>
#include <map>

namespace unity
{

namespace scopes
{
class FilterState;

namespace internal
{

class FilterBaseImpl
{
public:
    FilterBaseImpl(std::string const& id);
    FilterBaseImpl(VariantMap const& var);
    virtual ~FilterBaseImpl();
    void set_display_hints(int hints);
    std::string id() const;
    void set_title(std::string const& title);
    std::string title() const;
    int display_hints() const;
    void add_to_filter_group(FilterGroup::SCPtr const& group);
    FilterGroup::SCPtr filter_group() const;
    VariantMap serialize() const;
    virtual std::string filter_type() const = 0;
    static FilterBase::SCPtr deserialize(VariantMap const& var);
    static VariantArray serialize_filters(Filters const& filters);
    static Filters deserialize_filters(VariantArray const& var, std::map<std::string, FilterGroup::SCPtr> const& groups);
    static void validate_filters(Filters const& filters);
    virtual void validate_display_hints() const = 0;

protected:
    virtual void serialize(VariantMap& var) const = 0;
    static Variant get(FilterState const& filter_state, std::string const& filter_id);
    static VariantMap& get(FilterState const& filter_state);

private:
    std::string id_;
    std::string title_;
    int display_hints_;
    FilterGroup::SCPtr filter_group_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
