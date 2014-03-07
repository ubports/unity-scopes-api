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
Note that the data is likely to change between major versions of Unity, and therefore many of
the definitions are Variant-based.

When Unity requests a preview for a particular result, the scope is expected to construct it
from a few basic building blocks by instantiating a PreviewWidget. Each widget is identified
by a free-form id, type of the widget, and a number of attributes associated with a specific widget type.

The attribute values can be either filled in directly before pushing the widget to Unity
(using add_attribute_value()), or they can be mapped from a result field in a
similar fashion to the components mapping when specifying a CategoryRenderer
(see add_attribute_mapping()). When using the add_attribute_mapping() method, the
data is not required to be present in the result and the value can be pushed later
using the PreviewReply::push() method, which accepts the name of the field and its value as a Variant.

An example creating a basic preview demonstrating the three ways to associate preview widget attribute with a value:

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
    std::string description = fetch_description(result.uri());

    reply->push("description", Variant(description));
}
\endcode

\section previewtypes Recognized preview widget types and their attributes

The following widgets types are recognized by Unity:
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

\subsection audio audio widget

The audio widget displays a list of tracks with play / pause controls.

List of attributes:

\arg \c tracks A composite attribute containing an array of tuples, where each tuple has up to four fields: \c title (mandatory string), \c subtitle (optional string), \c source (mandatory uri) and \c length (optional integer specifying the track length in seconds)

Composite attributes are easy to construct using the unity::scopes::VariantBuilder helper class:

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

\arg \c source A uri pointing to the video
\arg \c screenshot A uri pointing to a screenshot of the video (optional)

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

\arg \c source A uri pointing to the image
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

\arg \c sources An array of image uris

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

\arg \c title A string specifying title
\arg \c subtitle A string specifying subtitle
\arg \c mascot A uri specifying mascot
\arg \c emblem A uri specifying emblem
\arg \c attribute-1 TBD
\arg \c attribute-2 TBD
\arg \c attribute-3 TBD

\code{.cpp}
{
    PreviewWidget w1("hdr", "header");

    w1.add_attribute_value("title", Variant("Result item"));
    w1.add_attribute_value("mascot", Variant("file:///tmp/mascot.png"));
    ...
}
\endcode

\subsection actions actions widget

The actions widget displays a single or multiple buttons the user can invoke.

List of attributes:

\arg \c actions A composite attribute containing an array of tuples, where each tuple has at least these fields: \c id (mandatory string which will be passed to unity::scopes::ScopeBase::activate_preview_action()), \c label (mandatory string) and \c icon (optional uri).

Composite attributes are easy to construct using the unity::scopes::VariantBuilder helper class:

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

The progress widget displays progress of an action (for example download progress).

Upon completion the scope will receive preview action activation with id \c "finished" or \c "failed" depending on the outcome of the operation.

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

The text widget can be used for any length of text, although it may not be formatted.

List of attributes:

\arg \c title Optional string
\arg \c text String containing the summary text

\code{.cpp}
{
    PreviewWidget w1("summary", "text");

    w1.add_attribute_value("text", Variant("Lorem Ipsum ..."));
    ...
}
\endcode

\subsection rating-input rating-input widget

The rating-input widget is used when the user can rate the content. It consists of two types of widget - a star-based rating and an input field where the user can type his/her review. It is possible to hide either and/or require both to be filled in.

When user presses the send button, the scope will receive preview action activation with id \c "rated".

List of attributes:

\arg \c rating-label String for the star-based rating (default: "Rate this")
\arg \c review-label String for the review input (default: "Add a review")
\arg \c submit-label String for the confirmation button (default: "Send")
\arg \c rating-icon-empty Uri for an empty rating icon
\arg \c rating-icon-full Uri for a full rating icon
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

The reviews widget is used to display a previously-rated content.

List of attributes:

\arg \c rating-icon-empty Uri for an empty rating icon
\arg \c rating-icon-half Uri for an half-full rating icon
\arg \c rating-icon-full Uri for a full rating icon
\arg \c reviews A composite attribute containing an array of tuples, where each tuple has up to three fields: \c rating (optional integer specifying the number of stars), \c author (mandatory string) and \c review (optional string).

Composite attributes are easy to construct using the unity::scopes::VariantBuilder helper class:

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

*/

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

std::string PreviewWidget::data() const
{
    return p->data();
}

//! @endcond

} // namespace scopes

} // namespace unity
