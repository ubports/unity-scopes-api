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

#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/CategoryRendererImpl.h>

namespace unity
{

namespace scopes
{

/*!
\class CategoryRenderer

This class specifies how is a particular category rendered by Unity. Note that the data is likely to change between major versions of Unity, and therefore the definition isn't strongly typed and provided by a scope author as a JSON string.

A Category contains all the information needed by Unity to render results provided by a scope author (by handling unity::scopes::SearchQueryBase::run()) in a way that gives the user as much useful information as possible. When pushing results to the query originator (unity::scopes::SearchReply::push()), each result needs to have a category associated, and this association determines what will the result look like.

The most important part of a category definition is the unity::scopes::CategoryRenderer instance. If you use the default constructor CategoryRenderer(), the renderer will use the following definition:

\verbatim
{
  "schema-version" : 1,
  "template" : {
    "category-layout" : "grid"
  },
  "components" : {
    "title" : "title",
    "art" : "art"
  }
}
\endverbatim

As specified by the \c "category-layout" key of the \c "template" dictionary, Unity will render results associated with this category in a grid layout. The \c "components" dictionary specifies which result fields are used by Unity. In case of this definition, each tile of the grid will map the "title" field from the result (set also by the call to unity::scopes::Result::set_title()) as title for the grid tile, and "art" field from the result (see unity::scopes::Result::set_art()) as the icon for the grid tile.

To sum up, the \c "template" dictionary contains information to determine the correct renderer and its parameters, and the \c "components" dictionary contains mapping specifying which fields of the results are used by the renderer, where keys of the dictionary are understood by Unity and values specify either field name of the results directly (<tt>{"title": "album_name"}</tt> would mean that Unity will use <tt>result["album_name"]</tt> as a title for the grid tile), or the value can specify extra hints for the renderer as well as the result field name (<tt>{"art": {"field": "icon", "aspect-ratio": 1.3}}</tt>).

\section jsonschema1 JSON structure (v1)

When using <tt>{"schema-version": 1}</tt>, the following keys are understood:

\subsection template1 template keys

\arg \c category-layout Specifies renderer type; possible values: \c "grid" (default), \c "carousel", \c "organic-grid", \c "journal", \c "vertical-journal"
\arg \c card-layout Specifies layout of the individual result cards; possible values: \c "vertical" (default), \c "horizontal"
\arg \c card-size Size of the result cards; possible values: \c "small", \c "medium" (default), \c "large"; when using <tt>"category-layout": "vertical-journal"</tt> any integer between 12 and 38
\arg \c overlay Overlay text data on top of the art; boolean, default false
\arg \c collapsed-rows Number of result rows displayed while the category is collapsed; possible values: 1 or 2 (only affects grid and journal)

\subsection components1 components keys

\arg \c title Card title, string
\arg \c art Card art (primary graphics), uri; can contain subkeys: \c "aspect-ratio" (double specifying the aspect ratio of the graphics, default: 1.0) and \c "fill-mode" (\c "crop" (default), or \c "fit")
\arg \c subtitle Subtitle of a card, string
\arg \c mascot Mascot (secondary graphics), uri
\arg \c emblem Emblem, uri
\arg \c summary Text summary, string
\arg \c attribute-1 string
\arg \c attribute-2 string
\arg \c attribute-3 string

\section example Example

In the following example a category named "Recommended" containing three components is created (title, art and subtitle), and a result providing values for these components is pushed.

Note that the scope is free to set any other extra result fields even if they are not used by the renderer (and therefore not specified in the \c "components" dictionary), such fields will be preserved and available to the scope when handling result-specific methods (for example unity::scopes::ScopeBase::preview()).

\code{.cpp}
// use raw string literal, so we don't have to escape all the quotes
std::string CATEGORY_DEFINITION = R"(
{
  "schema-version" : 1,
  "template" : {
    "category-layout" : "carousel",
    "card-size": "small"
  },
  "components" : {
    "title" : "title",
    "art" : {
      "field": "art",
      "aspect-ratio": 1.3
    },
    "subtitle": "publisher"
  }
}
)";

void MyQuery::run(SearchReplyProxy const& reply)
{
    auto category = reply->register_category("recommended", "Recommended", icon, CategoryRenderer(CATEGORY_DEFINITION));
    // push a sample result
    CategorisedResult result(category); // create a result item in "recommended" category
    result.set_uri("http://www.example.org");
    result.set_title("Example Result");
    result.set_art("http://www.example.org/graphics.png");
    result.set_dnd_uri("http://www.example.org");
    result["publisher"] = "Example.org";

    reply->push(result); // send result to the client
}
\endcode

*/

//! @cond

CategoryRenderer::CategoryRenderer(std::string const& json_text)
    : p(new internal::CategoryRendererImpl(json_text))
{
}

CategoryRenderer::~CategoryRenderer() = default;

CategoryRenderer::CategoryRenderer(CategoryRenderer const& other)
    : p(new internal::CategoryRendererImpl(*(other.p)))
{
}

CategoryRenderer& CategoryRenderer::operator=(CategoryRenderer const& other)
{
    if (this != &other)
    {
        p.reset(new internal::CategoryRendererImpl(*(other.p)));
    }
    return *this;
}

CategoryRenderer::CategoryRenderer(CategoryRenderer&&) = default;
CategoryRenderer& CategoryRenderer::operator=(CategoryRenderer&&) = default;

CategoryRenderer CategoryRenderer::from_file(std::string const& path)
{
    return internal::CategoryRendererImpl::from_file(path);
}

std::string CategoryRenderer::data() const
{
    return p->data();
}

bool operator==(const CategoryRenderer& lhs, const CategoryRenderer& rhs)
{
    return lhs.data() == rhs.data();
}

//! @endcond

} // namespace scopes

} // namespace unity
