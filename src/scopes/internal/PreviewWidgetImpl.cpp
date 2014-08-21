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

#include <unity/scopes/internal/PreviewWidgetImpl.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal

{

//! @cond

PreviewWidgetImpl PreviewWidgetImpl::from_json(std::string const& json_text)
{
    //TODO: json validation
    const internal::JsonCppNode node(json_text);
    auto var = node.to_variant().get_dict();

    return from_json_node(var);
}

PreviewWidgetImpl PreviewWidgetImpl::from_json_node(VariantMap const& var)
{
    PreviewWidgetImpl pimpl;

    //
    // the JSON representation of preview widget keeps all the attributes at the top level of the dict,
    // and has "components" sub-dictionary; split it into components and attributes and extract id & type.

    auto it = var.find("components");
    if (it != var.end()) // components are optional
    {
        // convert VariantMap to map<string,string>
        for (auto const& kv: it->second.get_dict())
        {
            pimpl.add_attribute_mapping(kv.first, kv.second.get_string());
        }
    }

    PreviewWidgetList widgets;

    // iterate over top-level attributes, skip 'components' key
    for (auto const& kv: var)
    {
        if (kv.first != "components")
        {
            if (kv.first == "id")
            {
                pimpl.set_id(kv.second.get_string());
            }
            else if (kv.first == "type")
            {
                pimpl.set_widget_type(kv.second.get_string());
            }
            else if (kv.first == "widgets")
            {
                auto const va = kv.second.get_array();
                for (auto const w: va)
                {
                    widgets.push_back(PreviewWidget(new PreviewWidgetImpl(PreviewWidgetImpl::from_json_node(w.get_dict()))));
                }
            }
            else
            {
                pimpl.add_attribute_value(kv.first, kv.second);
            }
        }
    }

    if (pimpl.type_ == "expandable")
    {
        pimpl.widgets_ = widgets;
    }

    return pimpl;
}

PreviewWidgetImpl::PreviewWidgetImpl(std::string const& id, std::string const &widget_type)
    : id_(id),
      type_(widget_type)
{
    throw_on_empty("id", id_);
    throw_on_empty("widget_type", type_);
}

PreviewWidgetImpl::PreviewWidgetImpl(VariantMap const& var)
{
    auto it = var.find("id");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("PreviewWidgetImpl(): missing 'id'");
    }
    set_id(it->second.get_string());

    it = var.find("type");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("PreviewWidgetImpl(): missing 'type'");
    }
    set_widget_type(it->second.get_string());

    it = var.find("attributes");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("PreviewWidgetImpl(): missing 'attributes'");
    }
    for (auto const& kv: it->second.get_dict())
    {
        add_attribute_value(kv.first, kv.second);
    }

    it = var.find("components");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("PreviewWidgetImpl(): missing 'components'");
    }
    for (auto const& kv: it->second.get_dict())
    {
        add_attribute_mapping(kv.first, kv.second.get_string());
    }

    if (type_ == "expandable")
    {
        it = var.find("widgets");
        if (it != var.end())
        {
            for (auto const& w: it->second.get_array())
            {
                widgets_.push_back(PreviewWidget(new PreviewWidgetImpl(w.get_dict())));
            }
        }
    }
}

PreviewWidget PreviewWidgetImpl::create(VariantMap const& var)
{
    return PreviewWidget(new PreviewWidgetImpl(var));
}

void PreviewWidgetImpl::set_id(std::string const& id)
{
    throw_on_empty("id", id);
    id_ = id;
}

void PreviewWidgetImpl::set_widget_type(std::string const& widget_type)
{
    throw_on_empty("widget_type", widget_type);
    type_ = widget_type;
}

void PreviewWidgetImpl::add_attribute_value(std::string const& key, Variant const& value)
{
    if (key == "id" || key == "type" || key == "components" || key == "widgets")
    {
        throw InvalidArgumentException("PreviewWidget::add_attribute_value(): Can't override '" + key + "'");
    }
    attributes_[key] = value;
}

void PreviewWidgetImpl::add_widget(PreviewWidget const& widget)
{
    if (type_ != "expandable")
    {
        throw LogicException("PreviewWidget::add_widget(): widgets can only be added to widget of type 'expandable'");
    }
    if (widget.widget_type() == "expandable")
    {
        throw LogicException("PreviewWidget::add_widget(): can't add 'expandable' widget '" + widget.id() + "' into another 'expandable'");
    }
    if (widget.id() == id_)
    {
        throw LogicException("PreviewWidget::add_widget(): can't add widget '" + widget.id() + "' with same id as the 'expandable' widget");
    }

    for (auto const& w: widgets_)
    {
        if (w.id() == widget.id())
        {
            throw unity::LogicException("PreviewWidget::add_widget(): widget '" + widget.id() + "' already added to widget '" + id_);
        }
    }
    widgets_.push_back(widget);
}

void PreviewWidgetImpl::add_attribute_mapping(std::string const& key, std::string const& field_name)
{
    if (key.empty())
    {
        throw InvalidArgumentException("PreviewWidget::add_attribute_mapping(): Invalid empty key string");
    }
    if (field_name.empty())
    {
        throw InvalidArgumentException("PreviewWidget::add_attribute_mapping(): Invalid field_name string");
    }

    if (key == "id" || key == "type")
    {
        throw InvalidArgumentException("PreviewWidget::add_attribute_mapping(): Can't override component '" + key + "'");
    }
    components_[key] = field_name;
}

std::string PreviewWidgetImpl::id() const
{
    return id_;
}

std::string PreviewWidgetImpl::widget_type() const
{
    return type_;
}

std::map<std::string, std::string> PreviewWidgetImpl::attribute_mappings() const
{
    return components_;
}

VariantMap PreviewWidgetImpl::attribute_values() const
{
    return attributes_;
}

PreviewWidgetList PreviewWidgetImpl::widgets() const
{
    return widgets_;
}

std::string PreviewWidgetImpl::data() const
{
    // convert from map<string,string> to VariantMap
    VariantMap cm;
    for (auto const& kv: components_)
    {
        cm[kv.first] = Variant(kv.second);
    }

    VariantMap var;
    var["id"] = id_;
    var["type"] = type_;
    if (widgets_.size())
    {
        VariantArray va;
        for (auto w: widgets_)
        {
            va.push_back(Variant(w.serialize()));
        }
        var["widgets"] = Variant(va);
    }

    var["components"] = Variant(cm);
    for (auto const& kv: attributes_)
    {
        var[kv.first] = kv.second;
    }

    const Variant outer(var);
    const internal::JsonCppNode node(outer);
    return node.to_json_string();
}

void PreviewWidgetImpl::throw_on_empty(std::string const& name, std::string const& value)
{
    if (value.empty())
    {
        throw InvalidArgumentException("PreviewWidget: required attribute " + name + " is empty");
    }
}

VariantMap PreviewWidgetImpl::serialize() const
{
    // convert from map<string,string> to VariantMap
    VariantMap cm;
    for (auto const& kv: components_)
    {
        cm[kv.first] = Variant(kv.second);
    }

    // note: the internal on-wire serialization doesn't exactly match the json definition
    VariantMap vm;
    vm["id"] = id_;
    vm["type"] = type_;
    vm["attributes"] = attributes_;
    vm["components"] = Variant(cm);
    if (widgets_.size())
    {
        VariantArray va;
        for (auto w: widgets_)
        {
            va.push_back(Variant(w.serialize()));
        }
        vm["widgets"] = Variant(va);
    }
    return vm;
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace unity
