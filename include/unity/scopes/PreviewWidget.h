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

#ifndef UNITY_SCOPES_PREVIEW_WIDGET_H
#define UNITY_SCOPES_PREVIEW_WIDGET_H

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <memory>
#include <list>

namespace unity
{

namespace scopes
{
    class VariantMapBuilder;

namespace internal
{
    class PreviewWidgetImpl;
}

class UNITY_API PreviewWidget
{
public:
/// @cond
    UNITY_DEFINES_PTRS(PreviewWidget);
/// @endcond

    PreviewWidget(std::string const& id, std::string const &widget_type);
    PreviewWidget(std::string const& definition);
    PreviewWidget(PreviewWidget const& other);
    PreviewWidget(PreviewWidget&& other);

    PreviewWidget& operator=(PreviewWidget const& other);
    PreviewWidget& operator=(PreviewWidget&& other);

    void set_id(std::string const& id);
    void set_widget_type(std::string const &id);
    void add_attribute(std::string const& key, Variant const& value);
    void add_component(std::string const& key, std::string const& field_name);

    std::string id() const;
    std::string widget_type() const;
    VariantMap components() const;
    VariantMap attributes() const;

    VariantMap serialize() const;

private:
    std::shared_ptr<internal::PreviewWidgetImpl> p;
    PreviewWidget(internal::PreviewWidgetImpl *impl);
    friend class internal::PreviewWidgetImpl;
};

typedef std::list<PreviewWidget> PreviewWidgetList;

} // namespace scopes

} // namespace unity

#endif
