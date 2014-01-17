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

namespace internal
{
    class PreviewWidgetImpl;
}

class UNITY_API PreviewWidget
{
public:
/// @cond
    UNITY_DEFINES_PTRS(PreviewWidget);
/// @endcond

    PreviewWidget(std::string const& definition);

    std::string data() const;
    VariantMap serialize() const;

private:
    PreviewWidget(VariantMap const& variant_map);

    std::shared_ptr<internal::PreviewWidgetImpl> p;
};

typedef std::list<PreviewWidget> PreviewWidgetList;

} // namespace scopes

} // namespace unity

#endif
