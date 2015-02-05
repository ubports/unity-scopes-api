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

#include <unity/scopes/qt/QCategorisedResult.h>
#include <unity/scopes/qt/internal/QCategorisedResultImpl.h>

using namespace unity::scopes::qt;

/// @cond
QCategorisedResult::QCategorisedResult(QCategory::SCPtr category)
    : QResult(new internal::QCategorisedResultImpl(category))
{
}

QCategorisedResult::QCategorisedResult(QCategorisedResult const& other)
    : QResult(new internal::QCategorisedResultImpl(*other.fwd()))
{
}

QCategorisedResult& QCategorisedResult::operator=(QCategorisedResult const& other)
{
    if (this != &other)
    {
        p.reset(new internal::QCategorisedResultImpl(*other.fwd()));
    }
    return *this;
}

QCategorisedResult::QCategorisedResult(QCategorisedResult&&) = default;

QCategorisedResult& QCategorisedResult::operator=(QCategorisedResult&&) = default;

void QCategorisedResult::set_category(QCategory::SCPtr category)
{
    fwd()->set_category(category);
}

QCategory::SCPtr QCategorisedResult::category() const
{
    return fwd()->category();
}

internal::QCategorisedResultImpl* QCategorisedResult::fwd() const
{
    // synchronize the internal values in case anything was
    // changed by the [] operator
    p->sync_values();
    return dynamic_cast<internal::QCategorisedResultImpl*>(p.get());
}

/// @endcond
