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

PreviewWidget::PreviewWidget(std::string const& id, std::string const &widget_type)
    : p(new internal::PreviewWidgetImpl(id, widget_type))
{
}

PreviewWidget::PreviewWidget(internal::PreviewWidgetImpl *impl)
    : p(impl)
{
}

PreviewWidget::PreviewWidget(PreviewWidget const& other)
    : p(new internal::PreviewWidgetImpl(*(other.p)))
{
}

PreviewWidget::PreviewWidget(PreviewWidget&&) = default;
PreviewWidget& PreviewWidget::operator=(PreviewWidget&&) = default;

PreviewWidget& PreviewWidget::operator=(PreviewWidget const& other)
{
    if (this != &other)
    {
        p.reset(new internal::PreviewWidgetImpl(*(other.p)));
    }
    return *this;
}

VariantMap PreviewWidget::serialize() const
{
    return p->serialize();
}

void PreviewWidget::add_attribute(std::string const& key, Variant const& value)
{
    p->add_attribute(key, value);
}

void PreviewWidget::add_component(std::string const& key, std::string const& field_name)
{
    p->add_component(key, field_name);
}

std::string PreviewWidget::id() const
{
    return p->id();
}

std::string PreviewWidget::widget_type() const
{
    return p->widget_type();
}

std::map<std::string, std::string> PreviewWidget::components() const
{
    return p->components();
}

VariantMap PreviewWidget::attributes() const
{
    return p->attributes();
}

std::string PreviewWidget::data() const
{
    return p->data();
}

//! @endcond

} // namespace scopes

} // namespace unity
