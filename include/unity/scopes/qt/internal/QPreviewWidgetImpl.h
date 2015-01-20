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

#pragma once

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>

#include <QtCore/QVariant>

namespace unity
{

namespace scopes
{

class PreviewWidget;

namespace qt
{

class QPreviewWidget;

/*! \typedef QPreviewWidgetList
\brief List of preview widgets (see unity::scopes::qt::QPreviewWidget)
*/
typedef QList<QPreviewWidget> QPreviewWidgetList;

namespace internal
{

class QPreviewReplyImpl;

class QPreviewWidgetImpl
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QPreviewWidgetImpl);
    /// @endcond

    /**
    \brief Create an empty widget definition with a specific id and type.
    \param id The unique widget identifier.
    \param widget_type The type of the widget.
    */
    QPreviewWidgetImpl(QString const& id, QString const& widget_type);

    /**
    \brief Create a widget from a JSON definition.

    The JSON definition must be a dictionary that includes widget "id" and all the values of attributes required by
    desired
    \link previewwidgets widget type\endlink. For example, a definition of image widget may look as follows:
    \code{.cpp}
    PreviewWidget img(R"({"id": "img", "type": "image", "source": "http://imageuri"})");
    \endcode

    For cases where attribute mappings are to be used instead of direct values, they need to be enclosed in the
    "components" dictionary, e.g.
    \code{.cpp}
    PreviewWidget img(R"({"id": "img", "type": "image", "components": { "source": "screenshot-url" } })");
    \endcode
    (this example assumes "screenshot-url" value is either available in the result object that's being previewed, or it
    will be pushed with
    unity::scopes::PreviewReply::push() method)

    \note It is recommended to create widgets via unity::scopes::PreviewWidget(QString const&, QString const&)
    constructor
    and unity::scopes::PreviewWidget::add_attribute_value() / unity::scopes::PreviewWidget::add_attribute_mapping()
    methods,
    rather than via JSON definition.

    \param definition The JSON definition.
    */
    QPreviewWidgetImpl(QString const& definition);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QPreviewWidgetImpl(QPreviewWidgetImpl const& other);
    QPreviewWidgetImpl(QPreviewWidgetImpl&& other);
    virtual ~QPreviewWidgetImpl();

    QPreviewWidgetImpl& operator=(QPreviewWidgetImpl const& other);
    QPreviewWidgetImpl& operator=(QPreviewWidgetImpl&& other);
    //@}

    /**
    \brief Adds an attribute definition and its value.
    \param key The name of the attribute.
    \param value The value of the attribute.
    */
    void add_attribute_value(QString const& key, QVariant const& value);

    /**
    \brief Adds an attribute definition using a component mapping.

    If an attribute value is either not known, or the value is already present
    in a result field, this method creates a mapping between the attribute
    name and given the field name.

    If an attribute value is not known, the scope is expected to push the attribute value using
    unity::scopes::PreviewReply::push(); otherwise, the value is automatically
    mapped from the result.
    */
    void add_attribute_mapping(QString const& key, QString const& field_name);

    /**
      \brief Adds a widget into expandable widget.

      Adds a widget into this widget, which needs to be of 'expandable' type. This method throws
      if adding a widget into any other widget type. Also, adding an 'expandable' widget into
      another 'expandable' is not allowed.

      \throws unity::LogicException if type of this widget is other than 'expandable', or when
      adding 'expandable' to 'expandable'.
    */
    void add_widget(QPreviewWidget const& widget);

    /**
    \brief Get the identifier of this widget.
    \return The widget identifier.
    */
    QString id() const;

    /**
    \brief Get type name of this widget.
    \return The widget type.
    */
    QString widget_type() const;

    /**
    \brief Get the components of this widget.

    The returned map is a dictionary of (key, field name) pairs, as defined by calls to add_attribute_mapping().
    \return The components map.
    */
    QMap<QString, QString> attribute_mappings() const;

    /**
    \brief Get the attributes of this widget.

    The returned map is a dictionary of (key, value) pairs, as defined by calls to add_attribute_value().
    \return The attribute map.
    */
    QVariantMap attribute_values() const;

    /**
    \brief Get widgets of 'expandable' widget.

    Returns the list of widget attached to this widget, which must be of 'expandable' type. This list is
    always empty for other widget types.
    */
    QPreviewWidgetList widgets() const;

    /**
    \brief Get a JSON representation of this widget.
    \return The JSON string.
    */
    QString data() const;

    /// @cond
    QVariantMap serialize() const;
    /// @endcond

private:
    std::unique_ptr<PreviewWidget> api_widget_;
    QPreviewWidgetImpl(PreviewWidget const& api_widget);

    friend class unity::scopes::qt::QPreviewWidget;
    friend class internal::QPreviewReplyImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity
