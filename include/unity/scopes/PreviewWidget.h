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

class PreviewWidget;

/*! \typedef PreviewWidgetList
\brief List of preview widgets (see unity::scopes::PreviewWidget)
*/
typedef std::list<PreviewWidget> PreviewWidgetList;

class PreviewWidget
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(PreviewWidget);
    /// @endcond

    /**
    \brief Create an empty widget definition with a specific id and type.
    \param id The unique widget identifier.
    \param widget_type The type of the widget.
    */
    PreviewWidget(std::string const& id, std::string const &widget_type);

    /**
    \brief Create a widget from a JSON definition.

    The JSON definition must be a dictionary that includes widget "id" and all the values of attributes required by desired
    \link previewwidgets widget type\endlink. For example, a definition of image widget may look as follows:
    \code{.cpp}
    PreviewWidget img(R"({"id": "img", "type": "image", "source": "http://imageuri"})");
    \endcode

    For cases where attribute mappings are to be used instead of direct values, they need to be enclosed in the "components" dictionary, e.g.
    \code{.cpp}
    PreviewWidget img(R"({"id": "img", "type": "image", "components": { "source": "screenshot-url" } })");
    \endcode
    (this example assumes "screenshot-url" value is either available in the result object that's being previewed, or it will be pushed with
    unity::scopes::PreviewReply::push() method)

    \note It is recommended to create widgets via unity::scopes::PreviewWidget(std::string const&, std::string const&) constructor
    and unity::scopes::PreviewWidget::add_attribute_value() / unity::scopes::PreviewWidget::add_attribute_mapping() methods,
    rather than via JSON definition.

    \param definition The JSON definition.
    */
    PreviewWidget(std::string const& definition);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    PreviewWidget(PreviewWidget const& other);
    PreviewWidget(PreviewWidget&& other);
    virtual ~PreviewWidget();

    PreviewWidget& operator=(PreviewWidget const& other);
    PreviewWidget& operator=(PreviewWidget&& other);
    //@}

    /**
    \brief Adds an attribute definition and its value.
    \param key The name of the attribute.
    \param value The value of the attribute.
    */
    void add_attribute_value(std::string const& key, Variant const& value);

    /**
    \brief Adds an attribute definition using a component mapping.

    If an attribute value is either not known, or the value is already present
    in a result field, this method creates a mapping between the attribute
    name and given the field name.

    If an attribute value is not known, the scope is expected to push the attribute value using
    unity::scopes::PreviewReply::push(); otherwise, the value is automatically
    mapped from the result.
    */
    void add_attribute_mapping(std::string const& key, std::string const& field_name);

    /**
      \brief Adds a widget into expandable widget.

      Adds a widget into this widget, which needs to be of 'expandable' type. This method throws
      if adding a widget into any other widget type. Also, adding an 'expandable' widget into
      another 'expandable' is not allowed.

      \throws unity::LogicException if type of this widget is other than 'expandable', or when
      adding 'expandable' to 'expandable'.
    */
    void add_widget(PreviewWidget const& widget);

    /**
    \brief Get the identifier of this widget.
    \return The widget identifier.
    */
    std::string id() const;

    /**
    \brief Get type name of this widget.
    \return The widget type.
    */
    std::string widget_type() const;

    /**
    \brief Get the components of this widget.

    The returned map is a dictionary of (key, field name) pairs, as defined by calls to add_attribute_mapping().
    \return The components map.
    */
    std::map<std::string, std::string> attribute_mappings() const;

    /**
    \brief Get the attributes of this widget.

    The returned map is a dictionary of (key, value) pairs, as defined by calls to add_attribute_value().
    \return The attribute map.
    */
    VariantMap attribute_values() const;

    /**
    \brief Get widgets of 'expandable' widget.

    Returns the list of widget attached to this widget, which must be of 'expandable' type. This list is
    always empty for other widget types.
    */
    PreviewWidgetList widgets() const;

    /**
    \brief Get a JSON representation of this widget.
    \return The JSON string.
    */
    std::string data() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    std::unique_ptr<internal::PreviewWidgetImpl> p;
    PreviewWidget(internal::PreviewWidgetImpl *impl);
    friend class internal::PreviewWidgetImpl;
};

} // namespace scopes

} // namespace unity

#endif
