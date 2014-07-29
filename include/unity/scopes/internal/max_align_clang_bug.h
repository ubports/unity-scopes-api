/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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

#ifndef MAX_ALIGN_CLANG_BUG_H
#define MAX_ALIGN_CLANG_BUG_H

/* Work-around for bug in clang: http://reviews.llvm.org/rL201729 */

#ifdef __clang__
#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
#ifndef __CLANG_MAX_ALIGN_T_DEFINED

typedef struct {
  long long __clang_max_align_nonce1
       __attribute__((__aligned__(__alignof__(long long))));
long double __clang_max_align_nonce2
       __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;

#define __CLANG_MAX_ALIGN_T_DEFINED

#endif  /* __CLANG_MAX_ALIGN_T_DEFINED */
#endif  /* __STDC_VERSION__ */
#endif  /* __clang__ */

#endif
