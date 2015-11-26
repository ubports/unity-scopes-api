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

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace experimental;

class ValueSliderLabelsImpl
{
public:
    ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label);
    ValueSliderLabelsImpl(std::string const& min_label, std::string const& max_label, ValueSliderLabels::ValueLabelPairList const& extra_labels);

    std::string min_label() const;
    std::string max_label() const;
    ValueSliderLabels::ValueLabelPairList extra_labels() const;
};

}

}

}
