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

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/internal/PreviewWidgetImpl.h>

namespace unity
{

namespace scopes
{

//! @cond

PreviewWidget::PreviewWidget(std::string const& definition)
    : p(new internal::PreviewWidgetImpl(definition))
{
}

PreviewWidget::PreviewWidget(VariantMap const& variant_map)
    : p(new internal::PreviewWidgetImpl(variant_map))
{
}

std::string PreviewWidget::data() const
{
    return p->data();
}

VariantMap PreviewWidget::serialize() const
{
    return p->serialize();
}

//! @endcond

} // namespace scopes

} // namespace unity
