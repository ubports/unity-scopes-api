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

#include <gtest/gtest.h>

#include <QtCore/QMap>
#include <QtCore/QEvent>
#include <QtCore/QThread>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

// Verifies that the events are received as expected and in the right thread
class BasicEventsChecker
{
public:
    BasicEventsChecker() : wait_for_event_(-1),
                           event_found_(false),
                           thread_id_(nullptr)
    {
    }

    virtual ~BasicEventsChecker() = default;

    void setThreadId(void *thread_id)
    {
        thread_id_ = thread_id;
    }

    bool check_event(QEvent* e)
    {
        // This method is executed always in the Qt thread
        int iEventType = static_cast<int>(e->type());
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(event_counter_map_.find(iEventType) == event_counter_map_.end())
            {
                event_counter_map_[iEventType] = 1;
            }
            else
            {
                event_counter_map_[iEventType]++;
            }
        }

        // verify if we were actively waiting for an event
        if(wait_for_event_!=-1)
        {
            if(wait_for_event_ == iEventType)
            {
                // we were waiting for this event
                event_found_ = true;
            }
            EXPECT_EQ(wait_for_event_, iEventType);
        }

        // finally verify the thread
        if(thread_id_)
        {
            EXPECT_EQ(thread_id_, QThread::currentThreadId());
        }
        return true;
    }

    int nbEventCalls(int event)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(event_counter_map_.find(event)==event_counter_map_.end())
        {
            return 0;
        }
        else
        {
            return event_counter_map_[event];
        }
    }

    bool waitForEvent(int event, int timeout)
    {
        int INTERVAL_WAIT = 10;
        std::chrono::milliseconds dura(INTERVAL_WAIT);
        wait_for_event_ = event;
        event_found_ = false;
        while(!event_found_ && timeout > 0)
        {
            std::this_thread::sleep_for(dura);
            timeout -= INTERVAL_WAIT;
        }
        if(event_found_)
        {
            // we received the expected event
            wait_for_event_ = -1;
            return true;
        }
        else
        {
            // timeout was reached
            return false;
        }
    }

    std::atomic<int> wait_for_event_;
    std::atomic<bool> event_found_;
    std::atomic<void *> thread_id_;

    QMap<int,int> event_counter_map_;
    // mutex to control the accesses to the map
    std::mutex mutex_;
};
