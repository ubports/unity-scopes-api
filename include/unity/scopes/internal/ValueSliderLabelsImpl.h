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

#pragma once

#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ValueSliderLabelsImpl
{
public:
    ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label);
    ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label, std::vector<std::pair<double, std::string>> const& extra_labels);
    ValueSliderLabelsImpl(VariantMap const& var);
    ValueSliderLabelsImpl(ValueSliderLabelsImpl const&) = default;

    std::string min_label() const;
    std::string max_label() const;
    std::vector<std::pair<double, std::string>> extra_labels() const;
    VariantMap serialize() const;
    void deserialize(VariantMap const& var);

    void validate(double min, double max) const;

private:
    std::string min_label_;
    std::string max_label_;
    std::vector<std::pair<double, std::string>> extra_labels_;
};

}

}

}
