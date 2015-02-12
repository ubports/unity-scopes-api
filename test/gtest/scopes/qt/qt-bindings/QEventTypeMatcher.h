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
#include <gmock/gmock.h>

// class to match QEvent *
// It verifies that the event has the expected type and that
// it's been called in the expected thread
class QEventTypeMatcher : public testing::MatcherInterface<QEvent *>
{
public:
    QEventTypeMatcher(int type_to_check, void *thread_id)
        : type_to_check_(type_to_check), thread_id_(thread_id)
    {
    }

    virtual bool MatchAndExplain(QEvent * e, testing::MatchResultListener*) const
    {
        std::cout << "Type is: " << e->type() << std::endl;
        std::cout << "Thread id when matching is: " << QThread::currentThreadId() << std::endl;
        bool sameThread = true;
        if(thread_id_)
        {
            sameThread = thread_id_ == QThread::currentThreadId();
        }
        return ((e->type() == type_to_check_ || e->type() < 1000) && sameThread);
    }

    virtual void DescribeTo(::std::ostream* os) const {
        *os << "is an event of type " << type_to_check_;
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
        *os << "is NOT an event of type " << type_to_check_;
    }

    int type_to_check_;
    void *thread_id_;
};
