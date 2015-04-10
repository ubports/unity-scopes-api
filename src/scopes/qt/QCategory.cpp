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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#include <unity/scopes/qt/QCategory.h>
#include <unity/scopes/qt/internal/QCategoryImpl.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;

/// @cond
QCategory::QCategory(unity::scopes::Category::SCPtr category)
    : p(new internal::QCategoryImpl(category))
{
}

QCategory::QCategory(internal::QCategoryImpl* impl)
    : p(impl)
{
}

QCategory::~QCategory()
{
}

QString QCategory::id() const
{
    return p->id();
}

QString QCategory::title() const
{
    return p->title();
}

QString QCategory::icon() const
{
    return p->icon();
}

CannedQuery::SCPtr QCategory::query() const
{
    return p->query();
}

CategoryRenderer const& QCategory::renderer_template() const
{
    return p->renderer_template();
}

QVariantMap QCategory::serialize() const
{
    return p->serialize();
}

/// @endcond
