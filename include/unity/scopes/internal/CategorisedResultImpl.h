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

#include <string>
#include <memory>
#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class CategorisedResultImpl : public ResultImpl
{
public:
    explicit CategorisedResultImpl(Category::SCPtr category);
    CategorisedResultImpl(CategorisedResultImpl const& other);
    CategorisedResultImpl(Category::SCPtr category, VariantMap const& variant_map);
    CategorisedResultImpl(internal::CategoryRegistry const& reg, const VariantMap &variant_map);

    void set_category(Category::SCPtr category);
    Category::SCPtr category() const;

    static CategorisedResult create_result(CategorisedResultImpl* impl);

protected:
    void serialize_internal(VariantMap& var) const override;

private:
    Category::SCPtr category_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
