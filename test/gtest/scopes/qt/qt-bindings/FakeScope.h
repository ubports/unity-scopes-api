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

#pragma once

#include <unity/scopes/qt/QScopeBase.h>
#include <unity/scopes/qt/QPreviewQueryBase.h>
#include <unity/scopes/qt/QSearchQueryBase.h>

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/SearchMetadata.h>

#include <iostream>

class QQuery : public unity::scopes::qt::QSearchQueryBase
{
public:
    virtual void run(unity::scopes::qt::QSearchReplyProxy const&) override
    {
        std::cout << "QQuery::run()" << std::endl;
    }

    virtual void cancelled() override
    {
        std::cout << "QQuery::cancelled()" << std::endl;
    }
};

class QPreview : public unity::scopes::qt::QPreviewQueryBase
{
public:
    virtual void run(unity::scopes::qt::QPreviewReplyProxy const&) override
    {
        std::cout << "QQuery::run()" << std::endl;
    }

    virtual void cancelled() override
    {
        std::cout << "QQuery::cancelled()" << std::endl;
    }
};

class QScope : public unity::scopes::qt::QScopeBase
{
public:
    virtual void start(QString const&) override
    {
        std::cout << "QScope::start()" << std::endl;
    }

    virtual unity::scopes::qt::QPreviewQueryBase::UPtr preview(const unity::scopes::qt::QResult&,
                                                               const unity::scopes::qt::QActionMetadata&) override
    {
        return unity::scopes::qt::QPreviewQueryBase::UPtr(new QPreview());
    }

    virtual unity::scopes::qt::QSearchQueryBase::UPtr search(unity::scopes::CannedQuery const&,
                                                             unity::scopes::SearchMetadata const&) override
    {
        return unity::scopes::qt::QSearchQueryBase::UPtr(new QQuery());
    }
};
