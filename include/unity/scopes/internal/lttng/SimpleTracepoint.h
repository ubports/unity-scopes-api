/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef SIMPLETRACEPOINT_H_
#define SIMPLETRACEPOINT_H_

#include <lttng/tracepoint.h>

/* simple_tracepoint */

/*
 * The STAP_PROBEV() macro from sdt.h (SystemTap) used by
 * the tracepoint() macro from lttng/tracepoint.h fails to
 * build with clang at the moment. Disable tracepoints
 * when building with clang until this is resolved.
 *
 * See: http://sourceware.org/bugzilla/show_bug.cgi?id=13974
 */

#ifdef __clang__
/* template< typename... T > void simple_tracepoint_unused_args( T&&... ) {} */
#define simple_tracepoint( c, e, ... ) simple_tracepoint_unused_args( __VA_ARGS__ )
#else
#define simple_tracepoint( c, e, ... ) tracepoint( c, e, __VA_ARGS__ )
#endif

/* SIMPLE_TRACEPOINT */

#define SIMPLE_TRACEPOINT( name, loglevel, ... )\
  TRACEPOINT_EVENT(\
    TRACEPOINT_PROVIDER,\
    name,\
    TP_ARGS( _STP_ARGS( __VA_ARGS__ ) ),\
    TP_FIELDS(\
      _STP_FIELDS( __VA_ARGS__ )\
    )\
  )\
  TRACEPOINT_LOGLEVEL( TRACEPOINT_PROVIDER, name, loglevel )

/* stp argument definitions */

#define stp_integer( arg_type, arg_name ) (\
  ( arg_type, arg_name ),\
  ctf_integer, ( arg_type, arg_name, arg_name ) )

#define stp_integer_hex( arg_type, arg_name ) (\
  ( arg_type, arg_name ),\
  ctf_integer_hex, ( arg_type, arg_name, arg_name ) )

#define stp_integer_network( arg_type, arg_name ) (\
  ( arg_type, arg_name ),\
  ctf_integer_network, ( arg_type, arg_name, arg_name ) )

#define stp_integer_network_hex( arg_type, arg_name ) (\
  ( arg_type, arg_name ),\
  ctf_integer_network_hex, ( arg_type, arg_name, arg_name ) )

#define stp_array( arg_type, elem_type, arg_name, arg_size ) (\
  ( arg_type, arg_name ),\
  ctf_array, ( elem_type, arg_name, arg_name, arg_size ) )

#define stp_array_text( arg_type, elem_type, arg_name, arg_size ) (\
  ( arg_type, arg_name ),\
  ctf_array_text, ( elem_type, arg_name, arg_name, arg_size ) )

#define stp_sequence( arg_type, elem_type, arg_name, arg_size ) (\
  ( arg_type, arg_name ),\
  ctf_sequence, ( elem_type, arg_name, arg_name, size_t, arg_size ) )

#define stp_sequence_text( arg_type, elem_type, arg_name, arg_size ) (\
  ( arg_type, arg_name ),\
  ctf_sequence_text, ( elem_type, arg_name, arg_name, size_t, arg_size ) )

#define stp_string( arg_name ) (\
  ( const char*, arg_name ),\
  ctf_string, ( arg_name, arg_name ) )

#define stp_float( arg_type, arg_name ) (\
  ( arg_type, arg_name ),\
  ctf_float, ( arg_type, arg_name, arg_name ) )

/* Internal */

#define _STP_ARGS( ... )\
  _TP_COMBINE_TOKENS( _STP_ARGS_, _TP_NARGS( 0, __VA_ARGS__ ) ) ( __VA_ARGS__ )

#define _STP_FIELDS( ... )\
  _TP_COMBINE_TOKENS( _STP_FIELDS_, _TP_NARGS( 0, __VA_ARGS__ ) ) ( __VA_ARGS__ )

#define ___TPA( arg, arg2 ) arg, arg2
#define __TPA( args, ctf, fields ) ___TPA args
#define _TPA( args_ctf_fields ) __TPA args_ctf_fields

#define __TPF( args, ctf, fields ) ctf fields
#define _TPF( args_ctf_fields ) __TPF args_ctf_fields

#define _STP_ARGS_1( a )\
  _TPA( a )
#define _STP_ARGS_2( a, b )\
  _TPA( a ), _TPA( b )
#define _STP_ARGS_3( a, b, c )\
  _TPA( a ), _TPA( b ), _TPA( c )
#define _STP_ARGS_4( a, b, c, d )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d )
#define _STP_ARGS_5( a, b, c, d, e )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e )
#define _STP_ARGS_6( a, b, c, d, e, f )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e ), _TPA( f )
#define _STP_ARGS_7( a, b, c, d, e, f, g )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e ), _TPA( f ), _TPA( g )
#define _STP_ARGS_8( a, b, c, d, e, f, g, h )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e ), _TPA( f ), _TPA( g ), _TPA( h )
#define _STP_ARGS_9( a, b, c, d, e, f, g, h, i )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e ), _TPA( f ), _TPA( g ), _TPA( h ), _TPA( i )
#define _STP_ARGS_10( a, b, c, d, e, f, g, h, i, j )\
  _TPA( a ), _TPA( b ), _TPA( c ), _TPA( d ), _TPA( e ), _TPA( f ), _TPA( g ), _TPA( h ), _TPA( i ), _TPA( j )

#define _STP_FIELDS_1( a )\
  _TPF( a )
#define _STP_FIELDS_2( a, b )\
  _TPF( a ) _TPF( b )
#define _STP_FIELDS_3( a, b, c )\
  _TPF( a ) _TPF( b ) _TPF( c )
#define _STP_FIELDS_4( a, b, c, d )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d )
#define _STP_FIELDS_5( a, b, c, d, e )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e )
#define _STP_FIELDS_6( a, b, c, d, e, f )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e ) _TPF( f )
#define _STP_FIELDS_7( a, b, c, d, e, f, g )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e ) _TPF( f ) _TPF( g )
#define _STP_FIELDS_8( a, b, c, d, e, f, g, h )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e ) _TPF( f ) _TPF( g ) _TPF( h )
#define _STP_FIELDS_9( a, b, c, d, e, f, g, h, i )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e ) _TPF( f ) _TPF( g ) _TPF( h ) _TPF( i )
#define _STP_FIELDS_10( a, b, c, d, e, f, g, h, i, j )\
  _TPF( a ) _TPF( b ) _TPF( c ) _TPF( d ) _TPF( e ) _TPF( f ) _TPF( g ) _TPF( h ) _TPF( i ) _TPF( j )

#endif /* SIMPLETRACEPOINT_H_ */
