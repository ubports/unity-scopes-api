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

#ifndef UNITY_API_SCOPES_PREVIEW_WIDGET_IMPL_H
#define UNITY_API_SCOPES_PREVIEW_WIDGET_IMPL_H

#include <scopes/PreviewWidget.h>
#include <unity/SymbolExport.h>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class PreviewWidgetImpl
{
public:
    PreviewWidgetImpl(std::string const& json_text);
    PreviewWidgetImpl(VariantMap const& variant_map);

    std::string data() const;

    VariantMap serialize() const;

private:
    std::string data_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
