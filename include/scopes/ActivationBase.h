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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#ifndef UNITY_SCOPES_ACTIVATIONBASE_H
#define UNITY_SCOPES_ACTIVATIONBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <scopes/ActivationResponse.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ActivationBaseImpl;
}

/**
\brief TODO
*/
class UNITY_API ActivationBase
{
public:
    /// @cond
    NONCOPYABLE(ActivationBase);
    UNITY_DEFINES_PTRS(ActivationBase);
    ActivationBase();
    virtual ~ActivationBase() noexcept;
    /// @endcond

    virtual void cancelled();
    virtual ActivationResponse activate();

private:
    void cancel();
    std::unique_ptr<internal::ActivationBaseImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
