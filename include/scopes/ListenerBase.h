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

#ifndef UNITY_API_SCOPES_RECEIVERBASE_H
#define UNITY_API_SCOPES_RECEIVERBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <scopes/Category.h>
#include <scopes/Annotation.h>

#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class CategorisedResult;
class ActivationResponse;

/**
\brief Abstract base class to receive the results of a query.
TODO: fix doc
The scope application code must instantiate a class derived from ListenerBase and pass that instance as
a parameter to the Scope::query() method. Once a query is sent, the scopes run time repeatedly
invokes the push() method, once for each result returned by the query. Once a query is complete,
the finished() method is called once, to inform the caller that the query is complete.

Calls to push() and finished() are made by an arbitrary thread.

// TODO: add doc for thread pool and concurrent calls to push()
*/

class UNITY_API ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(ListenerBase);
    UNITY_DEFINES_PTRS(ListenerBase);

    virtual ~ListenerBase() noexcept;
    /// @endcond

    /**
    \brief Indicates the cause of a call to finished().
    The Error enumerator indicates that a query terminated abnormally, for example,
    because a scope could not be reached over the network, a query terminated
    abnormally, or explicitly reported an error.
    */
    enum Reason { Finished, Cancelled, Error };

    /**
    \brief Called once by the scopes run time after the final result for a query() was sent.
    Exceptions thrown from finished() are ignored.
    \param r Indicates the cause for the call to finished().
    \param error_message If r is set to Reason::Error, error_message contains further details.
           Otherwise, error_message is the empty string.
    */
    virtual void finished(Reason r, std::string const& error_message) = 0;

protected:
    /// @cond
    ListenerBase();
    /// @endcond
};

class UNITY_API SearchListener : public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(SearchListener);
    UNITY_DEFINES_PTRS(SearchListener);

    virtual ~SearchListener() noexcept;
    /// @endcond

    /**
    \brief Called once by the scopes run time for each result that is returned by a query().
    */
    virtual void push(CategorisedResult result) = 0;

    /**
    \brief Called once by the scopes run time for each annotation that is returned by a query().
    The default implementation does nothing.
    */
    virtual void push(Annotation annotation);

    /**
    \brief Called once by the scopes run time for each category that is returned by a query().
    The default implementation does nothing. Receipt of categories may be interleaved with
    the receipt of results, that is, there is no guarantee that the complete set of categories
    will be provided before the first query result.

    If push() throws an exception, the scopes run time calls finished() with an 'Error' reason.
    */
    virtual void push(Category::SCPtr category);

protected:
    /// @cond
    SearchListener();
    /// @endcond
};

class UNITY_API PreviewListener : public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(PreviewListener);
    UNITY_DEFINES_PTRS(PreviewListener);

    virtual ~PreviewListener() noexcept;
    /// @endcond

    /**
    \brief Called by the scopes runtime for each preview chunk that is returned by preview().
    */
    //virtual void push(PreviewWidgetList const&) = 0; // TODO: enable!

    /**
    \brief Called by the scopes runtime for each data field that is returned by preview().
    */
    virtual void push(std::string const& key, Variant const& value) = 0;

protected:
    /// @cond
    PreviewListener();
    /// @endcond
};

class UNITY_API ActivationListener: public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(ActivationListener);
    UNITY_DEFINES_PTRS(ActivationListener);

    ~ActivationListener() noexcept;
    /// @endcond

    /**
    \brief TODO
    */
    virtual void activation_response(ActivationResponse const& response);
    void finished(Reason r, std::string const& error_message) override;

protected:
    /// @cond
    ActivationListener();
    /// @endcond
};


/**
\brief Convenience function to convert a ListenerBase::Reason enumerator to a string.
\return Possible return values are "finished", "cancelled", and "error".
*/
char const* to_string(ListenerBase::Reason reason);

} // namespace scopes

} // namespace api

} // namespace unity

#endif
