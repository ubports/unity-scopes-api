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

typedef std::pair<double, std::string> ValueLabelPair;
typedef std::vector<ValueLabelPair> ValueLabelPairList;

/**
\brief Labels used by a ValueSliderFilter.

The ValueSliderLabels objects holds labels for a ValueSliderFilter. At the very least labels for minimum and maximum
values need to be provided. In addition, an arbitrary number of extra labels can be defined to mark values between minimum and maximum values.
*/
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

    /**
      \brief Create ValueSliderLabels object with labels for minimum and maximum values only.

      \throws unity::InvalidArgumentException on invalid (empty or duplicated) labels.
     */
    ValueSliderLabels(std::string const& min_label, std::string const& max_label);

    /**
      \brief Create ValueSliderLabels object with labels for minimum and maximum, as well as extra labels.

      Create ValueSliderLabels object with labels for minimum, maximum and values between minimum and maximum.
      Note that these extra labels act only as a guidance for the user (i.e. they do not limit the available choices)
      and also Unity shell is free to omit some or all of them depending on the available screen space.

      \param min_label The label for minimum value of the associated ValueSliderFilter filter.
      \param max_label The label for maximum value of the associated ValueSliderFilter filter.
      \param extra_labels Additional labels for values between minimum and maximum values.

      \throws unity::InvalidArgumentException on invalid labels (empty or duplicate labels, non-increasing values).
     */
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

} // namespace scopes

} // namespace unity
