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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_PREVIEWLISTENERBASE_H
#define UNITY_SCOPES_PREVIEWLISTENERBASE_H

#include <unity/scopes/ListenerBase.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/Variant.h>

#include <string>

namespace unity
{

namespace scopes
{

/**
\brief Abstract base class for a scope to respond to previews.

An instance of this class must be return from Scope::preview().
*/

class PreviewListenerBase : public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(PreviewListenerBase);
    UNITY_DEFINES_PTRS(PreviewListenerBase);

    virtual ~PreviewListenerBase();
    /// @endcond

    /**
    \brief Called by the scopes runtime for each columns layout definition returned by preview().
    */
    virtual void push(ColumnLayoutList const& layouts) = 0;

    /**
    \brief Called by the scopes runtime for each preview chunk that is returned by preview().
    */
    virtual void push(PreviewWidgetList const&) = 0;

    /**
    \brief Called by the scopes runtime for each data field that is returned by preview().
    */
    virtual void push(std::string const& key, Variant const& value) = 0;

protected:
    /// @cond
    PreviewListenerBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
