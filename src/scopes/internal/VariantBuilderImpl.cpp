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

#include <unity/scopes/internal/VariantBuilderImpl.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

VariantBuilderImpl::VariantBuilderImpl(VariantBuilderImpl const& other)
{
    if (other.variant_)
    {
        variant_.reset(new Variant(*(other.variant_)));
    }
}

void VariantBuilderImpl::add_tuple(std::initializer_list<std::pair<std::string, Variant>> const& tuple)
{
    VariantArray va;
    if (variant_ != nullptr)
    {
        if (variant_->which() == Variant::Type::Array)
        {
            va = variant_->get_array();
        }
        else if (!variant_->is_null()) // if null (the initial type of the internal variant), then init it with an array, otherwise throw
        {
            throw unity::LogicException("Can only add tuple to a variant of array type");
        }
    }

    va.push_back(Variant(VariantMap(tuple.begin(), tuple.end())));
    variant_.reset(new Variant(std::move(va)));
}

void VariantBuilderImpl::add_tuple(std::vector<std::pair<std::string, Variant>> const& tuple)
{
    VariantArray va;
    if (variant_ != nullptr)
    {
        if (variant_->which() == Variant::Type::Array)
        {
            va = variant_->get_array();
        }
        else if (!variant_->is_null()) // if null (the initial type of the internal variant), then init it with an array, otherwise throw
        {
            throw unity::LogicException("Can only add tuple to a variant of array type");
        }
    }

    va.push_back(Variant(VariantMap(tuple.begin(), tuple.end())));
    variant_.reset(new Variant(std::move(va)));
}

Variant VariantBuilderImpl::end()
{
    if (variant_ == nullptr)
    {
        throw unity::LogicException("VariantBuilder::end(): no Variant has been constructed");
    }
    Variant v(std::move(*variant_));
    variant_.reset();
    return v;
}

} // namespace internal

} // namespace scopes

} // namespace unity
