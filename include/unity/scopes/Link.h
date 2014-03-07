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

#ifndef UNITY_SCOPES_LINK_H
#define UNITY_SCOPES_LINK_H

#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{
class LinkImpl;
class AnnotationImpl;
}

/**
 * \brief A hyperlink (label and canned query).
 */
class Link final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Link);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Link(Link const& other);
    Link(Link&&);
    Link& operator=(Link const& other);
    Link& operator=(Link&&);
    //@}

    /// @cond
    ~Link();
    /// @endcond

    /**
     * \brief Returns the label of this Link instance.
     * \return The label of this link instance.
     */
    std::string label() const;

    /**
     * \brief Returns a CannedQuery instance.
     * \return The canned query.
     */
    CannedQuery query() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    Link(std::string const& label, CannedQuery const& query);
    Link(VariantMap const& variant_map);
    std::unique_ptr<internal::LinkImpl> p;

    friend class internal::AnnotationImpl;
};

} // namespace scopes

} // namespace unity

#endif
