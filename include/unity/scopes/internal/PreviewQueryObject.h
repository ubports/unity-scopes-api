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
 * Authored by: Michal Hruby <michal.hruby@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_PREVIEWQUERYOBJECT_H
#define UNITY_SCOPES_INTERNAL_PREVIEWQUERYOBJECT_H

#include <unity/scopes/internal/QueryObject.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>

namespace unity
{

namespace scopes
{
class PreviewQueryBase;

namespace internal
{

class PreviewQueryObject final : public QueryObject
{
public:
    UNITY_DEFINES_PTRS(PreviewQueryObject);

    PreviewQueryObject(std::shared_ptr<PreviewQueryBase> const& preview_base, MWReplyProxy const& reply, MWQueryCtrlProxy const& ctrl);
    virtual ~PreviewQueryObject();
    void run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept override;

private:
    std::shared_ptr<PreviewQueryBase> preview_base_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
