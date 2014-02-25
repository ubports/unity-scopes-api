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

class PreviewWidget
{
public:
/// @cond
    UNITY_DEFINES_PTRS(PreviewWidget);
/// @endcond

    PreviewWidget(std::string const& id, std::string const &widget_type);
    PreviewWidget(std::string const& definition);
    PreviewWidget(PreviewWidget const& other);
    PreviewWidget(PreviewWidget&& other);
    virtual ~PreviewWidget();

    PreviewWidget& operator=(PreviewWidget const& other);
    PreviewWidget& operator=(PreviewWidget&& other);

    /**
     \brief Adds an attribute definition and its value.
     */
    void add_attribute(std::string const& key, Variant const& value);
    /**
     \brief Adds an attribute definition by using component mapping.

     If an attribute value is either not known, or the value is already present
     in a result field, this method will create a mapping between the attribute
     name and given field name.

     In the former case you'll be expected to push the attribute value using
     unity::scopes::PreviewReply::push(), otherwise the value is automatically
     mapped from the result.
     */
    void add_component(std::string const& key, std::string const& field_name);

    std::string id() const;
    std::string widget_type() const;
    std::map<std::string, std::string> components() const;
    VariantMap attributes() const;
    std::string data() const;

    VariantMap serialize() const;

private:
    std::unique_ptr<internal::PreviewWidgetImpl> p;
    PreviewWidget(internal::PreviewWidgetImpl *impl);
    friend class internal::PreviewWidgetImpl;
};

/*! \typedef PreviewWidgetList
\brief List of preview widgets (see unity::scopes::PreviewWidget)
*/
typedef std::list<PreviewWidget> PreviewWidgetList;

} // namespace scopes

} // namespace unity

#endif
