/*
 * Copyright (C) 2016 Canonical Ltd
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

#include <unity/scopes/internal/FilterGroupImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/Utils.h>

namespace unity
{

namespace scopes
{

namespace internal
{

FilterGroupImpl::FilterGroupImpl(std::string const& id, std::string const& label)
    : id_(id),
      label_(label)
{
}

std::string FilterGroupImpl::id() const
{
    return id_;
}

std::string FilterGroupImpl::label() const
{
    return label_;
}

VariantArray FilterGroupImpl::serialize_filter_groups(Filters const& filters)
{
    std::map<std::string, FilterGroup::SCPtr> group_lookup;
    VariantArray va;
    for (auto const& filter: filters)
    {
        auto grp = filter->filter_group();
        if (grp)
        {
            auto it = group_lookup.find(grp->id());
            if (it == group_lookup.end())
            {
                group_lookup[grp->id()] = grp;
                VariantMap grpvar;
                grpvar["id"] = grp->id();
                grpvar["label"] = grp->label();
                va.push_back(Variant(grpvar));
            }
            else
            {
                if (it->second != grp)
                {
                    throw unity::LogicException("FilterGroupImpl::serialize_filter_groups(): duplicate FilterGroup definitions for group id '" + grp->id() + "'");
                }
            }
        }
    }
    return va;
}

std::map<std::string, FilterGroup::SCPtr> FilterGroupImpl::deserialize_filter_groups(VariantArray const& var)
{
    std::map<std::string, FilterGroup::SCPtr> groups;
    for (auto it = var.begin(); it != var.end(); it++)
    {
        auto const grvar = it->get_dict();
        auto const id = find_or_throw("FilterGroup::deserialize_filter_groups", grvar, "id")->second.get_string();
        auto const label = find_or_throw("FilterGroup::deserialize_filter_groups", grvar, "label")->second.get_string();
        auto group = FilterGroup::create(id, label);
        groups[id] = group;
    }
    return groups;
}

} // namespace internal

} // namespace scopes

} // namespace unity
