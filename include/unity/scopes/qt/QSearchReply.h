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

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>

#include <unity/scopes/qt/QCategory.h>
#include <unity/scopes/qt/QDepartment.h>

#include <QtCore/QString>

#include <memory>

namespace unity
{

namespace scopes
{

namespace qt
{
class QSearchQueryBase;
class QCategorisedResult;

namespace internal
{
class QSearchReplyImpl;
}

/**
\brief Allows the results of a search query to be sent to the query source.
*/
class QSearchReply : public QObject
{
    Q_OBJECT
public:
    /// @cond
    NONCOPYABLE(QSearchReply);
    UNITY_DEFINES_PTRS(QSearchReply);
    /// @endcond
    /**
    \brief Destroys a QSearchReply.
    */
    virtual ~QSearchReply();

    /**
     \brief Register departments for the current search reply and provide the current department.

     Departments are optional. If scope supports departments, it is expected to register departments on every search as
     follows:

      <ul>
      <li>create a Department node for current department and attach to it a list of its subdepartments (unless current
     department is a leaf department) using
      unity::scopes::Department::set_subdepartments() method. For every subdepartment on the list set
     "has_subdepartments" flag if applicable.
      <li>provide an alternate label for current department with unity::scopes::Department::set_alternate_label().
      <li>create a Department node for parent of current department (if applicable - not when in root department), and
     attach current Department node to it with
      unity::scopes::Department::set_subdepartments() method.
      <li>register the parent department with unity::scopes::qt::QSearchReply::register_departments().
      </ul>

     For example, assuming the user is visiting a "History" department in "Books", and "History" has sub-departments
     such as "World War Two" and "Ancient", the code
     that registers departments for current search in "History" may look like this:
     \code{.cpp}
     unity::scopes::qt::QDepartment::SPtr books = move(unity::scopes::qt::QDepartment::create("books", query, "Books"));
     // the
     parent of "History"
     unity::scopes::Department::SPtr history = move(unity::scopes::qt::QDepartment::create("history", query,
     "History"));
     unity::scopes::DepartmentList history_depts({
                                                 move(unity::scopes::qt::QDepartment::create("ww2", query, "World War
     Two")),
                                                 move(unity::scopes::qt::QDepartment::create("ancient", query,
     "Ancient"))});
     history->set_subdepartments(history_depts);
     books->set_subdepartments({history});
     reply->register_departments(books);
     \endcode

     Current department should be the department returned by unity::scopes::CannedQuery::department_id(). Empty
     department id denotes
     the root deparment.

     \param parent The parent department of current department, or current one if visiting root department.
     \throws unity::LogicException if departments are invalid (nullptr passed, current department not present in the
     parent's tree, duplicated department ids present in the tree).
     */
    virtual void register_departments(QDepartment::SCPtr const& parent);

    /**
    \brief Register new category and send it to the source of the query.

    \param id The identifier of the category
    \param title The title of the category
    \param icon The icon of the category
    \param renderer_template The renderer template to be used for results in this category

    \return The category instance
    \throws unity::scopes::InvalidArgumentException if category with that id has already been registered.
    */
    virtual QCategory::SCPtr register_category(
        QString const& id,
        QString const& title,
        QString const& icon,
        unity::scopes::CategoryRenderer const& renderer_template = unity::scopes::CategoryRenderer());

    /**
    \brief Sends a single result to the source of a query.

    Any calls to push() after finished() was called are ignored.
    \return The return value is true if the result was accepted, false otherwise.
    A false return value can be due to finished() having been called earlier,
    or the client that sent the query having cancelled that query. The return
    value is false also if the query has a cardinality limit and is reached
    or exceeded. (The return value is false for the last valid push and
    subsequent pushes.)
    */
    virtual bool push(QCategorisedResult const& result);

    /**
    \brief Informs the source of a query that the query was terminated due to an error.

    Multiple calls to error() and calls to finished() after error() was called are ignored.
    \param ex An exception_ptr indicating the cause of the error. If ex is a `std::exception`,
              the return value of `what()` is made available to the query source. Otherwise,
              the query source receives `"unknown exception"`.
    */
    virtual void error(std::exception_ptr ex);

private:
    QSearchReply(unity::scopes::SearchReplyProxy& reply, QObject* parent = 0);

    std::unique_ptr<internal::QSearchReplyImpl> p;
    friend class internal::QSearchReplyImpl;
    friend class QSearchQueryBaseAPI;
};

}  // namespace unity

}  // namespace scopes

}  // namespace unity
