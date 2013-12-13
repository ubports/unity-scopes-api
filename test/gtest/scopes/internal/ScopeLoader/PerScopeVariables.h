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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef SCOPELOADERTEST_PERSCOPEVARIABLES
#define SCOPELOADERTEST_PERSCOPEVARIABLES

#include "TestExports.h"

int EXPORT get_scopeA_var();
void EXPORT set_scopeA_var(int val);

int EXPORT get_scopeB_var();
void EXPORT set_scopeB_var(int val);

void EXPORT clear_vars();

#endif
