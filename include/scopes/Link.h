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

#ifndef UNITY_API_SCOPES_LINK_H
#define UNITY_API_SCOPES_LINK_H

#include <unity/SymbolExport.h>
#include <unity/util/DefinesPtrs.h>
#include <scopes/Query.h>
#include <scopes/Variant.h>
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
 * \brief Represents a hyperlink (a label and canned Query)
 */
class UNITY_API Link final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Link);
    /// @endcond

    /**
     * \brief Returns label of this Link instance.
     * \return a label
     */
    std::string label() const;

    /**
     * \brief Returns a canned Query instance.
     * \return a canned Query
     */
    Query query() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond
private:
    Link(std::string const& label, Query const& query);
    Link(VariantMap const& variant_map);
    std::shared_ptr<internal::LinkImpl> p;

    friend class internal::AnnotationImpl;
};

} // namespace scopes

} // namespace unity

#endif
