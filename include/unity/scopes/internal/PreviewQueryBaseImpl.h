/*
 * Copyright (C) 2014 Canonical Ltd
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

#ifndef UNITY_INTERNAL_PREVIEWQUERYBASEIMPL_H
#define UNITY_INTERNAL_PREVIEWQUERYBASEIMPL_H

#include <unity/scopes/internal/QueryBaseImpl.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ActionMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class PreviewQueryBaseImpl : public QueryBaseImpl
{
public:
    PreviewQueryBaseImpl(Result const& result, ActionMetadata const& metadata);

    Result result() const;
    ActionMetadata action_metadata() const;
    void cancel() override;
    bool valid() const override;

private:
    bool valid_;
    mutable std::mutex mutex_;
    const Result result_;
    const ActionMetadata action_metadata_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
