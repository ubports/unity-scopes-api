/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#include <unity/scopes/qt/internal/QPreviewQueryBaseImpl.h>
#include <unity/scopes/qt/internal/QActionMetadataImpl.h>

#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/Result.h>

#include <cassert>

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;
using namespace unity::scopes;
using namespace std;

QPreviewQueryBaseImpl::QPreviewQueryBaseImpl()
    : preview_api_(nullptr)
{
}

QResult QPreviewQueryBaseImpl::result() const
{
    assert(preview_api_);
    return QResult(preview_api_->result());
}

QActionMetadata QPreviewQueryBaseImpl::action_metadata() const
{
    assert(preview_api_);
    return QActionMetadata(new internal::QActionMetadataImpl(preview_api_->action_metadata()));
}

void QPreviewQueryBaseImpl::init(QPreviewQueryBaseAPI* preview_api)
{
    preview_api_ = preview_api;
}
