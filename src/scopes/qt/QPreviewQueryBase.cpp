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
#include <unity/scopes/qt/QPreviewQueryBase.h>

#include <unity/scopes/qt/internal/QPreviewQueryBaseImpl.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::qt;

/// @cond
QPreviewQueryBase::QPreviewQueryBase(QObject *parent)
    : QObject(parent), p(new internal::QPreviewQueryBaseImpl())
{
}

QPreviewQueryBase::~QPreviewQueryBase() = default;

QResult QPreviewQueryBase::result() const
{
    return p->result();
}

QActionMetadata QPreviewQueryBase::action_metadata() const
{
    return p->action_metadata();
}

void QPreviewQueryBase::init(QPreviewQueryBaseAPI* query_api)
{
    p->init(query_api);
}

/// @endcond
