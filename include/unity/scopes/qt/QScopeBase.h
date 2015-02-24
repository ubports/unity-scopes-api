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

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <unity/scopes/qt/QSearchQueryBase.h>
#include <unity/scopes/qt/QPreviewQueryBase.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <QtCore/QString>
#include <QtCore/QObject>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;

namespace qt
{

namespace internal
{
class QScopeBaseImpl;
}

/**
\file QScopeBase.h
\class QScopeBase
\brief Base class for a scope implementation.

Scopes are accessed by the scopes run time as a shared library (one library per scope).
Each scope must implement a class that derives from QScopeBase, for example:

~~~
#include <unity/scopes/qt/QScopeBase.h>

class MyScope : public unity::scopes::qt::QScopeBase
{
public:
    MyScope();
    virtual ~MyScope();

    virtual void start(QString const& scope_id);       // Optional
    virtual void stop();                               // Optional
    // ...
};
~~~

In addition, the library must provide two functions with "C" linkage:
 - a create function that must return a pointer to the derived instance
 - a destroy function that is passed the pointer returned by the create function

If the create function throws an exception, the destroy function will not be called. If the create function returns
NULL, the destroy function _will_ be called with NULL as its argument.

Rather than hard-coding the names of the functions, use the UNITY_SCOPE_CREATE_FUNCTION and
UNITY_SCOPE_DESTROY_FUNCTION macros, for example:

~~~

// You must provide a function that creates your scope on the heap and
// pass this function to the QScopeBaseAPI constructor.

unity::scopes::qt::QScopeBase *create_my_scope()
{
    return new MyScope();
}

unity::scopes::ScopeBase*
UNITY_SCOPE_CREATE_FUNCTION()
{
    // You must return a dynamically allocated QScopeBaseAPI instance here.
    // In turn, that instance calls your creation function to instantiate
    // your scope in the correct Qt thread.
    return new QScopeBaseAPI(create_my_scope);
}

// The runtime, once it has stopped your scope, calls the destroy function.

void
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope)
{
    delete scope;
}
~~~

After the scopes run time has obtained a pointer to the class instance from the create function, it calls start(),
which allows the scope to initialize itself.
*/
class QScopeBase : public QObject
{
    Q_OBJECT
public:
    /// @cond
    NONCOPYABLE(QScopeBase);
    UNITY_DEFINES_PTRS(QScopeBase);

    QScopeBase(QObject* parent = 0);
    virtual ~QScopeBase();
    /// @endcond

    /**
     * Called once at startup
     */
    virtual void start(QString const&);

    /**
     * Called at shutdown
     */
    virtual void stop();

    /**
     * Called each time a new preview is requested
     */
    virtual QPreviewQueryBase::UPtr preview(const QResult&, const QActionMetadata&) = 0;

    /**
     * Called each time a new query is requested
     */
    virtual QSearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
                                          unity::scopes::SearchMetadata const&) = 0;

private:
    std::unique_ptr<internal::QScopeBaseImpl> p;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity
