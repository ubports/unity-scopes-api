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

#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/internal/ValueSliderLabelsImpl.h>

namespace unity
{

namespace scopes
{

ValueSliderLabels::ValueSliderLabels(internal::ValueSliderLabelsImpl* pimpl)
    : p(pimpl)
{
}

ValueSliderLabels::ValueSliderLabels(std::string const& min_label, std::string const& max_label)
    : p(new internal::ValueSliderLabelsImpl(min_label, max_label))
{
}

ValueSliderLabels::ValueSliderLabels(std::string const& min_label, std::string const& max_label, ValueLabelPairList const& extra_labels)
    : p(new internal::ValueSliderLabelsImpl(min_label, max_label, extra_labels))
{
}

ValueSliderLabels::ValueSliderLabels(VariantMap const& var)
    : p(new internal::ValueSliderLabelsImpl(var))
{
}

ValueSliderLabels::ValueSliderLabels(ValueSliderLabels const& other)
    : p(new internal::ValueSliderLabelsImpl(*(other.p)))
{
}

ValueSliderLabels& ValueSliderLabels::operator=(ValueSliderLabels const& other)
{
    if (this != &other)
    {
        p.reset(new internal::ValueSliderLabelsImpl(*(other.p)));
    }
    return *this;
}

ValueSliderLabels::ValueSliderLabels(ValueSliderLabels&&) = default;
ValueSliderLabels& ValueSliderLabels::operator=(ValueSliderLabels&&) = default;

ValueSliderLabels::~ValueSliderLabels() = default;

std::string ValueSliderLabels::min_label() const
{
    return p->min_label();
}

std::string ValueSliderLabels::max_label() const
{
    return p->max_label();
}

ValueLabelPairList ValueSliderLabels::extra_labels() const
{
    return p->extra_labels();
}

VariantMap ValueSliderLabels::serialize() const
{
    return p->serialize();
}

}

}
