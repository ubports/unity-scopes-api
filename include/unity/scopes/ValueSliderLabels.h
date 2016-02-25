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

#include <string>
#include <list>
#include <memory>
#include <unity/SymbolExport.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ValueSliderFilterImpl;
class ValueSliderLabelsImpl;
}

namespace experimental
{

typedef std::pair<double, std::string> ValueLabelPair;
typedef std::vector<ValueLabelPair> ValueLabelPairList;

class UNITY_API ValueSliderLabels final
{
public:
    /**@name Copy and assignment
    Copy and assignment (move and non-move versions) have the usual value semantics.
    */
    //{@
    ValueSliderLabels(ValueSliderLabels const& other);
    ValueSliderLabels(ValueSliderLabels&&);
    ValueSliderLabels& operator=(ValueSliderLabels const& other);
    ValueSliderLabels& operator=(ValueSliderLabels&&);
    ~ValueSliderLabels();
    //@}

    ValueSliderLabels(std::string const& min_label, std::string const& max_label);
    ValueSliderLabels(std::string const& min_label, std::string const& max_label, ValueLabelPairList const& extra_labels);

    /**
     \brief Get the label for the minimum value of the filter.
     \return The label for minimum value
     */
    std::string min_label() const;

    /**
     \brief Get the label for the maximum value of the filter.
     \return The label for maximum value
     */
    std::string max_label() const;

    /**
     \brief Get the labels for intermediate values of the filter.
     */
    ValueLabelPairList extra_labels() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    ValueSliderLabels(internal::ValueSliderLabelsImpl* pimpl);
    ValueSliderLabels(VariantMap const& var);
    std::unique_ptr<internal::ValueSliderLabelsImpl> p;
    friend class internal::ValueSliderLabelsImpl;
    friend class internal::ValueSliderFilterImpl;
};

} // namespace experimental

} // namespace scopes

} // namespace unity
