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

#ifndef SCOPELOADERTEST_COUNTERS
#define SCOPELOADERTEST_COUNTERS

#include "TestExports.h"

int EXPORT num_create();
void EXPORT inc_create();

int EXPORT num_destroy();
void EXPORT inc_destroy();

int EXPORT num_start();
void EXPORT inc_start();

int EXPORT num_run();
void EXPORT inc_run();

int EXPORT num_stop();
void EXPORT inc_stop();

void EXPORT reset_counters();

#endif
