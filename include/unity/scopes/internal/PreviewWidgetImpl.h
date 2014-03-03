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

#ifndef UNITY_SCOPES_INTERNAL_PREVIEWWIDGETIMPL_H
#define UNITY_SCOPES_INTERNAL_PREVIEWWIDGETIMPL_H

#include <unity/scopes/PreviewWidget.h>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class PreviewWidgetImpl
{
public:
    PreviewWidgetImpl(std::string const& json_text);
    PreviewWidgetImpl(std::string const& id, std::string const& widget_type);
    PreviewWidgetImpl(VariantMap const& var);
    PreviewWidgetImpl(PreviewWidgetImpl const& other) = default;
    ~PreviewWidgetImpl() = default;

    void set_id(std::string const& id);
    void set_widget_type(std::string const &widget_type);
    void add_attribute_value(std::string const& key, Variant const& value);
    void add_attribute_mapping(std::string const& key, std::string const& field_name);

    std::string id() const;
    std::string widget_type() const;
    std::map<std::string, std::string> attribute_mappings() const;
    VariantMap attribute_values() const;
    std::string data() const;

    VariantMap serialize() const;
    static PreviewWidget create(VariantMap const& var);

private:
    static void throw_on_empty(std::string const& name, std::string const& value);
    std::string id_;
    std::string type_;
    VariantMap attributes_;
    std::map<std::string, std::string> components_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
