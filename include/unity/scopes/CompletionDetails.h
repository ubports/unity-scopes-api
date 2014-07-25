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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_COMPLETIONDETAILS_H
#define UNITY_SCOPES_COMPLETIONDETAILS_H

#include <unity/util/NonCopyable.h>

#include <memory>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class CompletionDetailsImpl;

}

/**
 \brief blah.
*/

class CompletionDetails final
{
public:
    /// @cond
    NONCOPYABLE(CompletionDetails);
    /// @endcond

    /// @cond
    ~CompletionDetails();
    /// @endcond

private:
    std::unique_ptr<internal::CompletionDetailsImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
