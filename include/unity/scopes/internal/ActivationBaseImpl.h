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

#ifndef UNITY_INTERNAL_ACTIVATIONBASEIMPL_H
#define UNITY_INTERNAL_ACTIVATIONBASEIMPL_H

#include <unity/scopes/ActivationResponse.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ActivationBaseImpl final
{
public:
    NONCOPYABLE(ActivationBaseImpl);

    ActivationBaseImpl() = default;
    ~ActivationBaseImpl() = default;
    ActivationResponse activate();
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
