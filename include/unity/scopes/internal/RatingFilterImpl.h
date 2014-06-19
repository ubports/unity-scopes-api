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

#ifndef UNITY_INTERNAL_RATINGFILTERIMPL_H
#define UNITY_INTERNAL_RATINGFILTERIMPL_H

#include <unity/scopes/internal/RadioButtonsFilterImpl.h>
#include <unity/scopes/RatingFilter.h>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace experimental;

class RatingFilterImpl : public RadioButtonsFilterImpl
{
public:
    RatingFilterImpl(std::string const& id, std::string const& label);
    RatingFilterImpl(std::string const& id, std::string const& label, int top_rating);
    RatingFilterImpl(VariantMap const& var);
    FilterOption::SCPtr add_option(std::string const& id, std::string const& label) override;
    void set_on_icon(std::string const& on_icon);
    void set_off_icon(std::string const& off_icon);
    std::string on_icon() const;
    std::string off_icon() const;
    FilterOption::SCPtr active_rating(FilterState const& filter_state) const;

    static RatingFilter::SPtr create(VariantMap const& var);

    std::string filter_type() const override;

    static const int max_rating = 10;

private:
    std::string on_icon_;
    std::string off_icon_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
