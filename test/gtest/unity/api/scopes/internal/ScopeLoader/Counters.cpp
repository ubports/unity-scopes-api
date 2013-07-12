/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "Counters.h"
#include <mutex>

using namespace std;

namespace
{

int create_calls;
int destroy_calls;
int start_calls;
int stop_calls;
std::mutex count_mutex;

}

// Helper functions to read and increment the number of calls under protection of a mutex.
// We use this for the tests to track how often each function was called from the scope thread.

int num_create()
{
    lock_guard<mutex> lock(count_mutex);
    return create_calls;
}

void inc_create()
{
    lock_guard<mutex> lock(count_mutex);
    ++create_calls;
}

int num_destroy()
{
    lock_guard<mutex> lock(count_mutex);
    return destroy_calls;
}

void inc_destroy()
{
    lock_guard<mutex> lock(count_mutex);
    ++destroy_calls;
}

int num_start()
{
    lock_guard<mutex> lock(count_mutex);
    return start_calls;
}

void inc_start()
{
    lock_guard<mutex> lock(count_mutex);
    ++start_calls;
}

int num_stop()
{
    lock_guard<mutex> lock(count_mutex);
    return stop_calls;
}

void inc_stop()
{
    lock_guard<mutex> lock(count_mutex);
    ++stop_calls;
}

void reset_counters()
{
    lock_guard<mutex> lock(count_mutex);
    create_calls = 0;
    destroy_calls = 0;
    start_calls = 0;
    stop_calls = 0;
}
