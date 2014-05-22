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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_EXECUTOR_H_
#define UNITY_SCOPES_INTERNAL_EXECUTOR_H_

#include <core/posix/exec.h>
#include <unity/util/DefinesPtrs.h>

#include <string>

namespace unity
{
namespace scopes
{
namespace internal
{

class Executor
{
public:
    UNITY_DEFINES_PTRS (Executor);

    Executor();

    virtual ~Executor() = default;

    virtual core::posix::ChildProcess exec (const std::string& fn,
            const std::vector<std::string>& argv,
            const std::map<std::string, std::string>& env,
            const core::posix::StandardStream& flags,
            const std::string& confinement_profile);

};

} // namespace internal
} // namespace scopes
} // namespace unity

#endif
