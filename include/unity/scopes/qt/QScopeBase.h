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

#include <unity/scopes/qt/QSearchQueryBase.h>
#include <unity/scopes/qt/QPreviewQueryBase.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <QtCore/QString>

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
Each scope must implement a class that derives from ScopeBase, for example:

~~~
#include <unity/scopes/qt/QScopeBase.h>

class MyScope : public unity::scopes::qt::QScopeBase
{
public:
    MyScope();
    virtual ~MyScope();

    virtual void start(QString const& scope_id);   // Optional
    virtual void stop();                               // Optional
    virtual void run();                                // Optional
    // ...
};
~~~

In addition, the library must provide two functions with "C" linkage:
 - a create function that must return a pointer to the derived instance
 - a destroy function that is passed the pointer returned by the create function

Typically, the create and destroy functions will simply call `new` and `delete`, respectively. (However,
there is no requirement that the derived class instance must be heap-allocated.)
If the create function throws an exception, the destroy function will not be called. If the create function returns
NULL, the destroy function _will_ be called with NULL as its argument.

Rather than hard-coding the names of the functions, use the UNITY_SCOPE_CREATE_FUNCTION and
UNITY_SCOPE_DESTROY_FUNCTION macros, for example:

~~~
unity::scopes::ScopeBase*
UNITY_SCOPE_CREATE_FUNCTION()
{
    // instantiate MyScope
    MyScope* scope = new MyScope();

    // Initialize scope. This line is mandatory, you should pass your scope to
    // the class QScopeBaseAPI, which will initialize the internal structures used
    // by the scopes API
    return new QScopeBaseAPI(*scope);
}

void
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope)
{
    delete scope;       // Example only, heap-allocation is not mandatory
}
~~~

After the scopes run time has obtained a pointer to the class instance from the create function, it calls start(),
which allows the scope to initialize itself. This is followed by a call to run().
All calls to the methods of this class will be done from the main QThread.

The scope implementation, if it does not return from run(), is expected to return from run() in response to a
call to stop() in a timely manner.
*/
class QScopeBase
{
public:
    /// @cond
    NONCOPYABLE(QScopeBase);
    UNITY_DEFINES_PTRS(QScopeBase);
    /// @endcond

    QScopeBase();
    virtual ~QScopeBase();

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
    friend class internal::QScopeBaseImpl;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity
