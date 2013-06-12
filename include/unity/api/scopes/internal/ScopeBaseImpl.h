/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_SCOPEBASEIMPL_H
#define UNITY_API_SCOPES_SCOPEBASEIMPL_H

#include <unity/api/scopes/ScopeBase.h>

// Convenience definitions for looking up the create and destroy functions in the symbol table
// of a dynamically loaded scope.
// UNITY_API_SCOPE_CREATE_SYMSTR and UNITY_API_SCOPE_DESTROY_SYMSTR expand to a string literal containing the name
// of the create and destroy function, respectively.

#define UNITY_API_SCOPE_STR(sym) #sym
#define UNITY_API_SCOPE_XSTR(sym) UNITY_API_SCOPE_STR(sym)

#define UNITY_API_SCOPE_CREATE_SYMSTR UNITY_API_SCOPE_XSTR(UNITY_API_SCOPE_CREATE_FUNCTION)
#define UNITY_API_SCOPE_DESTROY_SYMSTR UNITY_API_SCOPE_XSTR(UNITY_API_SCOPE_DESTROY_FUNCTION)

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ScopeBaseImpl : private util::NonCopyable
{
public:
    ScopeBaseImpl();
    virtual ~ScopeBaseImpl();

    virtual void stop();
    void run();

    void runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept;
};

} // namespace internal

// Convenience typedefs for the create and destroy function pointers.

typedef decltype(&UNITY_API_SCOPE_CREATE_FUNCTION) CreateFunction;
typedef decltype(&UNITY_API_SCOPE_DESTROY_FUNCTION) DestroyFunction;

} // namespace scopes

} // namespace api

} // namespace unity


#endif
