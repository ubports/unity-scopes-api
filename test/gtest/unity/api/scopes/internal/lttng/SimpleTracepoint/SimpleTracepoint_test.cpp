/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <TestProvider_tp.h>

#include <fstream>
#include <gtest/gtest.h>
#include <string>

// Basic trace test.

TEST(SimpleTracepoint, basic_trace_test)
{
  system("rm -R -f ./lttng-trace");
  system("lttng create trace_session -o ./lttng-trace");
  system("lttng enable-event -a -s trace_session -u");
  system("lttng start trace_session");

  simple_tracepoint( test_provider, event1, 0, 1, 2.3, "four" );
  simple_tracepoint( test_provider, event2, 5.6, 7 );

  system("lttng stop");
  system("lttng view -t ./lttng-trace > ./lttng-trace/trace.txt");
  system("lttng destroy trace_session");

  std::ifstream trace_file("./lttng-trace/trace.txt");
  std::string trace((std::istreambuf_iterator<char>(trace_file)),
                     std::istreambuf_iterator<char>());

  EXPECT_NE( std::string::npos, trace.find("test_provider:event1:") );
  EXPECT_NE( std::string::npos, trace.find("{ a = 0, b = 1, c = 2.3, d = \"four\" }") );
  EXPECT_NE( std::string::npos, trace.find("test_provider:event2:") );
  EXPECT_NE( std::string::npos, trace.find("{ a = 5.6, b = 7 }") );
}
