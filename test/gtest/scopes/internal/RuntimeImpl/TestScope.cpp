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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "TestScope.h"

#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::scopes;

TestScope::TestScope(Behavior b)
    : behavior_(b)
    , started_(false)
{
}

void TestScope::start(string const&)
{
    lock_guard<mutex> lock(m_);
    started_ = true;
    cond_.notify_all();

    if (behavior_ == ThrowFromStart)
    {
        throw unity::ResourceException("Can't start");
    }
}

void TestScope::stop()
{
    if (behavior_ == ThrowFromStop)
    {
        throw unity::ResourceException("Can't stop");
    }
}

void TestScope::run()
{
    if (behavior_ == ThrowFromRun)
    {
        throw unity::ResourceException("Can't run");
    }
}

void TestScope::wait_until_started()
{
    unique_lock<mutex> lock(m_);
    cond_.wait(lock, [this]{ return started_; });
}

SearchQueryBase::UPtr TestScope::search(CannedQuery const&, SearchMetadata const&)
{
    // Never called
    abort();
    return nullptr;
}

PreviewQueryBase::UPtr TestScope::preview(Result const&, ActionMetadata const&)
{
    // Never called
    abort();
    return nullptr;
}
