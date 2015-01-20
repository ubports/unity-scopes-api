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

#include <unity/scopes/qt/internal/QScopeBaseAPIImpl.h>
#include <unity/scopes/qt/QScopeBase.h>
#include <unity/scopes/qt/QSearchQueryBaseAPI.h>
#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>

#include <QtCore/QCoreApplication>

using namespace unity::scopes::qt::internal;
namespace sc = unity::scopes;

QScopeBaseAPIImpl::QScopeBaseAPIImpl(QScopeBase& qtscope)
    : qtscope_impl_(qtscope)
{
}

QScopeBaseAPIImpl::~QScopeBaseAPIImpl()
{
}

void QScopeBaseAPIImpl::start(std::string const& scope_id)
{
    qtscope_impl_.start(QString::fromUtf8(scope_id.c_str()));

    // start the QT thread
    // TODO change to make_unique when using C++14
    qtthread_ = std::unique_ptr<std::thread>(new std::thread(&QScopeBaseAPIImpl::startQtThread, this));
}

void QScopeBaseAPIImpl::stop()
{
    qtscope_impl_.stop();
    // exit the QCoreApplication
    qtapp_->quit();
}

sc::PreviewQueryBase::UPtr QScopeBaseAPIImpl::preview(const sc::Result& result, const sc::ActionMetadata& metadata)
{
    // Boilerplate construction of Preview
    return sc::PreviewQueryBase::UPtr(new QPreviewQueryBaseAPI(qtapp_, qtscope_impl_, result, metadata));
}

/**
 * Called each time a new query is requested
 */
sc::SearchQueryBase::UPtr QScopeBaseAPIImpl::search(sc::CannedQuery const& query, sc::SearchMetadata const& metadata)
{
    // Boilerplate construction of Query
    return sc::SearchQueryBase::UPtr(new QSearchQueryBaseAPI(qtapp_, qtscope_impl_, query, metadata));
}

void QScopeBaseAPIImpl::startQtThread()
{
    if (!QCoreApplication::instance())
    {
        int argc = 0;
        char* argv = NULL;
        qtapp_ = std::make_shared<QCoreApplication>(argc, &argv);
        qtapp_->exec();
    }
}
