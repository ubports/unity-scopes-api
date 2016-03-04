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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/ValueSliderLabelsImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

ValueSliderLabelsImpl::ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label)
    : min_label_(min_label),
      max_label_(max_label)
{
}

ValueSliderLabelsImpl::ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label, std::vector<std::pair<double, std::string>> const& extra_labels)
    : min_label_(min_label),
      max_label_(max_label),
      extra_labels_(extra_labels)
{
}

ValueSliderLabelsImpl::ValueSliderLabelsImpl(VariantMap const& var)
{
    deserialize(var);
}

std::string ValueSliderLabelsImpl::min_label() const
{
    return min_label_;
}

std::string ValueSliderLabelsImpl::max_label() const
{
    return max_label_;
}

std::vector<std::pair<double, std::string>> ValueSliderLabelsImpl::extra_labels() const
{
    return extra_labels_;
}

VariantMap ValueSliderLabelsImpl::serialize() const
{
    VariantMap vm;
    VariantArray va;
    for (auto const& p: extra_labels_)
    {
        va.push_back(Variant(p.first));
        va.push_back(Variant(p.second));
    }
    vm["extra_labels"] = Variant(va);
    vm["min_label"] = Variant(min_label_);
    vm["max_label"] = Variant(max_label_);
    return vm;
}

void ValueSliderLabelsImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("ValueSliderLabels::deserialize()", var, "min_label");
    min_label_ = it->second.get_string();
    it = find_or_throw("ValueSliderLabels::deserialize()", var, "max_label");
    max_label_ = it->second.get_string();
    it = var.find("extra_labels");
    if (it != var.end())
    {
        auto const va = it->second.get_array();
        for (auto it = va.begin(); it != va.end();)
        {
            auto const value = (it++)->get_double();
            if (it == va.end())
            {
                throw unity::InvalidArgumentException("ValueSliderLabels::deserialize(): invalid value-label array");
            }
            std::string const label = (it++)->get_string();
            extra_labels_.push_back(std::make_pair<>(value, label));
        }
    }
}
}

}

}
