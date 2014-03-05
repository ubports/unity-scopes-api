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

#ifndef UNITY_SCOPES_CATEGORYRENDERER_H
#define UNITY_SCOPES_CATEGORYRENDERER_H

#include <string>
#include <memory>
#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace scopes
{

namespace internal
{
    class CategoryRendererImpl;
}

/**
  \brief A default template for generic use.
 */
constexpr const char* DEFAULT_RENDERER
{
    R"(
    {
        "schema-version":1,
        "template":
        {
            "category-layout":"grid"
        },
        "components":
        {
            "title":"title",
            "art":"art"
        }
    }
    )"
};

/**
  \brief A template suitable for displaying music results.
 */
constexpr const char* MUSIC_GRID_RENDERER
{
    R"(
        {
            "schema-version":1,
            "template":
            {
                "category-layout":"grid"
            },
            "components":
            {
                "title":"title",
                "subtitle":"subtitle",
                "art":"art"
            }
        }
    )"
};

/**
\brief A category renderer template in JSON format.
*/

class CategoryRenderer
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(CategoryRenderer);
    /// @endcond

    /**
    \brief Creates a CategoryRenderer from a JSON string.
    */
    // TODO: document exception thrown for invalid string.
    explicit CategoryRenderer(std::string const& json_text = DEFAULT_RENDERER);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    CategoryRenderer(CategoryRenderer const&);
    CategoryRenderer& operator=(CategoryRenderer const&);

    CategoryRenderer(CategoryRenderer&&);
    CategoryRenderer& operator=(CategoryRenderer&&);
    virtual ~CategoryRenderer();
    //@}

    /**
    \brief Creates a CategoryRenderer from a text file.
    \return The CategoryRenderer corresponding to the information in the file.
    */
    // TODO: document exception thrown for invalid string.
    static CategoryRenderer from_file(std::string const& path);

    /**
    \brief Returns complete renderer template definition in JSON format.
    \return The renderer template (JSON).
    */
    std::string data() const;

private:
    std::unique_ptr<internal::CategoryRendererImpl> p;

    friend class internal::CategoryRendererImpl;
};

/// @cond
bool operator==(const CategoryRenderer&, const CategoryRenderer&);
/// @endcond

} // namespace scopes

} // namespace unity

#endif
