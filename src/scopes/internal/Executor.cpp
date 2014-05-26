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

#include <unity/scopes/internal/Executor.h>

#include <cerrno>
#include <sys/apparmor.h>

using namespace unity::scopes::internal;

Executor::Executor()
{
}

core::posix::ChildProcess Executor::exec(const std::string& fn,
        const std::vector<std::string>& argv,
        const std::map<std::string, std::string>& env,
        const core::posix::StandardStream& flags,
        const std::string& confinement_profile)
{
    std::function<void()> child_setup = [confinement_profile]()
    {
        if (!confinement_profile.empty())
        {
            if (aa_change_profile(confinement_profile.c_str()) < 0)
            {
                switch (errno)
                {
                    case ENOENT:
                    case EACCES:
                        throw std::runtime_error("AppArmor profile does not exist");
                        break;
                    case EINVAL:
                        throw std::runtime_error("AppArmor interface not available");
                        break;
                    default:
                        throw std::runtime_error("Unknown AppArmor error");
                }
            }
        }


    };
    return core::posix::exec(fn, argv, env, flags, child_setup);
}
