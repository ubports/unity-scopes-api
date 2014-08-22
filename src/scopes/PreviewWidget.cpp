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

/*!
\class PreviewWidget

\brief A widget for a preview.

This class describes an individual widget used when constructing a preview for a result item.
Note that the data that applies to particular widget types is likely to change with different major
versions of Unity; therefore, attributes are of type Variant, that is, loosely typed.

When Unity requests a preview for a particular result, the scope is expected to construct the preview
by instantiating a PreviewWidget. Each widget has a free-form id, a type, and a number of attributes whose
names and types depend on the specific widget type (see \link previewwidgets Preview Widgets\endlink).

The attribute values can either be filled in directly before pushing the widget to Unity
(using add_attribute_value()), or they can be mapped from a result field in a
similar fashion to the components mapping when specifying a CategoryRenderer
(see add_attribute_mapping()). When using add_attribute_mapping(), the corresponding
attribute need not be present in the result; instead, its value can be pushed later
using the PreviewReply::push() method, which accepts the name of the field and its value as a Variant.

Preview widget can also be created entirely from a JSON string. See the documentation of unity::scopes::PreviewWidget::PreviewWidget(std::string const&) constructor for details.

Here is an example that creates a preview and illustrates three ways to associate a preview widget attribute
with its value:

\code{.cpp}
void MyPreview::run(PreviewReplyProxy const& reply)
{
    PreviewWidget w1("img", "image");
    // directly specify source URI for the image widget
    w1.add_attribute_value("source", Variant("http://www.example.org/graphics.png"));

    PreviewWidget w2("hdr", "header");
    // the result associated with this preview already had a title specified, so we'll just map it to the preview widget
    w2.add_attribute_mapping("title", "title");
    // description is not present in the result, but we'll push it later
    w2.add_attribute_mapping("summary", "description");

    PreviewWidgetList widgets;
    widgets.push_back(w1);
    widgets.push_back(w2);
    reply->push(widgets);

    // do a costly database lookup for the description
    std::string description = fetch_description(result().uri());

    reply->push("description", Variant(description));
}
\endcode
*/

/**
\page previewwidgets Preview widget types
\section previewtypes Recognized preview widget types

The following widget types are recognized by Unity:
\arg \c audio
\arg \c video
\arg \c image
\arg \c gallery
\arg \c header
\arg \c actions
\arg \c progress
\arg \c text
\arg \c rating-input
\arg \c reviews
\arg \c expandable

\subsection audio audio widget

The audio widget displays a list of tracks with play/pause controls.

List of attributes:

\arg \c tracks A composite attribute containing an array of tuples, where each tuple has up to four
fields: \c title (mandatory string), \c subtitle (optional string), \c source (mandatory URI),
and \c length (optional integer specifying the track length in seconds)

You can construct composite attributes with unity::scopes::VariantBuilder:

\code{.cpp}
{
    PreviewWidget w1("tracks", "audio");

    VariantBuilder builder;
    builder.add_tuple({
        {"title", Variant("Track #1")},
        {"source", Variant("file:///tmp/song1.mp3")},
        {"length", Variant(194)}
    });
    builder.add_tuple({
        {"title", Variant("Track #2")},
        {"source", Variant("file:///tmp/song2.mp3")},
        {"length", Variant(207)}
    });

    w1.add_attribute_value("tracks", builder.end());
    ...
}
\endcode

\subsection video video widget

The video widget displays a still from a video and allows playing the video.

List of attributes:

\arg \c source A URI pointing to the video
\arg \c screenshot A URI pointing to a screenshot of the video (optional)

\code{.cpp}
{
    PreviewWidget w1("video1", "video");

    w1.add_attribute_value("source", Variant("file:///tmp/video1.mp4"));
    ...
}
\endcode

\subsection image image widget

The image widget displays a single image.

List of attributes:

\arg \c source A URI pointing to the image
\arg \c zoomable A boolean specifying whether the image is zoomable (default: \c false)

\code{.cpp}
{
    PreviewWidget w1("img1", "image");

    w1.add_attribute_value("source", Variant("file:///tmp/image.jpg"));
    ...
}
\endcode

\subsection gallery gallery widget

The gallery widget displays a set of images.

List of attributes:

\arg \c sources An array of image URIs

\code{.cpp}
{
    PreviewWidget w1("gal", "gallery");

    VariantArray arr;
    arr.push_back(Variant("file:///tmp/image1.jpg"));
    arr.push_back(Variant("file:///tmp/image2.jpg"));
    arr.push_back(Variant("file:///tmp/image3.jpg"));
    w1.add_attribute_value("sources", Variant(arr));
    ...
}
\endcode

\subsection header header widget

The header widget displays basic infomation about the result.

List of attributes:

\arg \c title A string specifying the title
\arg \c subtitle A string specifying the subtitle
\arg \c mascot A URI specifying the mascot
\arg \c emblem A URI specifying the emblem

\code{.cpp}
{
    PreviewWidget w1("hdr", "header");

    w1.add_attribute_value("title", Variant("Result item"));
    w1.add_attribute_value("mascot", Variant("file:///tmp/mascot.png"));
    ...
}
\endcode

\subsection actions actions widget

The actions widget displays one or more buttons.

List of attributes:

\arg \c actions A composite attribute containing an array of tuples, where each tuple has at least
these fields: \c id (a mandatory string that is passed to unity::scopes::ScopeBase::activate_preview_action()),
\c label (mandatory string), and \c icon (optional URI).

You can construct composite attributes with unity::scopes::VariantBuilder:

\code{.cpp}
{
    PreviewWidget w1("buttons", "actions");

    VariantBuilder builder;
    builder.add_tuple({
        {"id", Variant("open")},
        {"label", Variant("Open")}
    });
    builder.add_tuple({
        {"id", Variant("download")},
        {"label", Variant("Download")}
    });

    w1.add_attribute_value("actions", builder.end());
    ...
}
\endcode

\subsection progress progress widget

The progress widget displays the progress of an action, such as download progress.

On completion, the scope receives a preview action activation with the id \c "finished" or \c "failed",
depending on the outcome of the operation.

List of attributes:

\arg \c source A tuple with keys understood by a progress provider

\code{.cpp}
{
    PreviewWidget w1("download", "progress");

    VariantMap tuple;
    tuple["dbus-name"] = "com.canonical.DownloadManager";
    tuple["dbus-object"] = "/com/canonical/download/obj1";
    w1.add_attribute_value("source", Variant(tuple));
    ...
}
\endcode

\subsection text text widget

A text widget can be used for text of any length (without formatting).

List of attributes:

\arg \c title Optional string
\arg \c text A string containing the text

\code{.cpp}
{
    PreviewWidget w1("summary", "text");

    w1.add_attribute_value("text", Variant("Lorem Ipsum ..."));
    ...
}
\endcode

\subsection rating-input rating-input widget

The rating-input widget allows users to rate content. It consists of two types of widget:
a star-based rating and an input field for the user to enter his/her review.
It is possible to hide each widget as well as to require them to be filled in.

When a user presses the "Send" button, the scope receives a preview action activation with the id \c "rated".

List of attributes:

\arg \c rating-label String for the star-based rating (default: "Rate this")
\arg \c review-label String for the review input (default: "Add a review")
\arg \c submit-label String for the confirmation button (default: "Send")
\arg \c rating-icon-empty URI for an empty rating icon
\arg \c rating-icon-full URI for a full rating icon
\arg \c visible String specifying which of the two widgets are visible (\c "rating", \c "review" or default:\c "both")
\arg \c required String specifying which of the two widgets are required to be filled in (\c "rating", \c "review" or default:\c "both")

\code{.cpp}
{
    PreviewWidget w1("rating", "rating-input");

    w1.add_attribute_value("visible", Variant("rating"));
    w1.add_attribute_value("required", Variant("rating"));
    w1.add_attribute_value("rating-icon-empty", Variant("file:///tmp/star-empty.svg"));
    w1.add_attribute_value("rating-icon-full", Variant("file:///tmp/star-full.svg"));
    ...
}
\endcode

\subsection reviews reviews widget

The reviews widget is used to display previously-rated content.

List of attributes:

\arg \c rating-icon-empty URI for an empty rating icon
\arg \c rating-icon-half URI for an half-full rating icon
\arg \c rating-icon-full URI for a full rating icon
\arg \c reviews A composite attribute containing an array of tuples, where each tuple has up to three fields: \c rating (optional integer specifying the number of stars), \c author (mandatory string) and \c review (optional string).

You can construct composite attributes with unity::scopes::VariantBuilder:

\code{.cpp}
{
    PreviewWidget w1("summary", "reviews");

    w1.add_attribute_value("rating-icon-empty", Variant("file:///tmp/star-empty.svg"));
    w1.add_attribute_value("rating-icon-full", Variant("file:///tmp/star-full.svg"));

    VariantBuilder builder;
    builder.add_tuple({
        {"author", Variant("John Doe")},
        {"rating", Variant(3)}
    });
    builder.add_tuple({
        {"author", Variant("Mr. Smith")},
        {"rating", Variant(5)}
    });

    w1.add_attribute_value("reviews", builder.end());
    ...
}
\endcode

\subsection expandable expandable widget

The expandable widget is used to group several widgets into an expandable pane.

List of attributes:
\arg \c title A string specifying the title
\arg \c collapsed-widgets Optional number of collapsed widgets (0 makes all of them visible)

\code
    PreviewWidget expandable("exp", "expandable");
    expandable.add_attribute_value("title", Variant("This is an expandable widget"));
    expandable.add_attribute_value("collapsed-widgets", Variant(0));

    PreviewWidget w1("w1", "text");
    w1.add_attribute_value("title", Variant("Subwidget 1"));
    w1.add_attribute_value("text", Variant("A text"));
    PreviewWidget w2("w2", "text");
    w2.add_attribute_value("title", Variant("Subwidget 2"));
    w2.add_attribute_value("text", Variant("A text"));
    expandable.add_widget(w1);
    expandable.add_widget(w2);
    ...
\endcode
*/

//! @cond

PreviewWidget::PreviewWidget(std::string const& definition)
    : p(new internal::PreviewWidgetImpl(internal::PreviewWidgetImpl::from_json(definition)))
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
PreviewWidget::~PreviewWidget() = default;

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

void PreviewWidget::add_attribute_value(std::string const& key, Variant const& value)
{
    p->add_attribute_value(key, value);
}

void PreviewWidget::add_attribute_mapping(std::string const& key, std::string const& field_name)
{
    p->add_attribute_mapping(key, field_name);
}

void PreviewWidget::add_widget(PreviewWidget const& widget)
{
    p->add_widget(widget);
}

std::string PreviewWidget::id() const
{
    return p->id();
}

std::string PreviewWidget::widget_type() const
{
    return p->widget_type();
}

std::map<std::string, std::string> PreviewWidget::attribute_mappings() const
{
    return p->attribute_mappings();
}

VariantMap PreviewWidget::attribute_values() const
{
    return p->attribute_values();
}

PreviewWidgetList PreviewWidget::widgets() const
{
    return p->widgets();
}

std::string PreviewWidget::data() const
{
    return p->data();
}

//! @endcond

} // namespace scopes

} // namespace unity
