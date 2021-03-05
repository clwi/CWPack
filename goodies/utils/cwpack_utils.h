/*      CWPack/goodies - cwpack_utils.h   */
/*
 The MIT License (MIT)

 Copyright (c) 2017 Claes Wihlborg

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CWPack_utils_H__
#define CWPack_utils_H__


#include "cwpack.h"

/*******************************   P A C K   **********************************/

#define cw_pack_cstr(context,string) cw_pack_str (context, string, (uint32)strlen(string))

void cw_pack_float_opt (cw_pack_context* pack_context, float f);    /* Pack as signed if precision isn't destroyed */
void cw_pack_double_opt (cw_pack_context* pack_context, double d);  /* Pack as signed or float if precision isn't destroyed */
#define  cw_pack_real cw_pack_double_opt                            /* Backward compatibility */

#define cw_pack_timespec (pack_contextptr, timespecptr) cw_pack_time ((pack_contextptr), (int64_t)((timespecptr)->tv_sec), (uint32_t)((timespecptr)->tv_nsec))

void cw_pack_time_interval (cw_pack_context* pack_context, double ti); /* ti is seconds relative epoch */

/*****************************   U N P A C K   ********************************/

bool cw_unpack_next_boolean (cw_unpack_context* unpack_context);

int64_t cw_unpack_next_signed64 (cw_unpack_context* unpack_context);
int32_t cw_unpack_next_signed32 (cw_unpack_context* unpack_context);
int16_t cw_unpack_next_signed16 (cw_unpack_context* unpack_context);
int8_t cw_unpack_next_signed8 (cw_unpack_context* unpack_context);

uint64_t cw_unpack_next_unsigned64 (cw_unpack_context* unpack_context);
uint32_t cw_unpack_next_unsigned32 (cw_unpack_context* unpack_context);
uint16_t cw_unpack_next_unsigned16 (cw_unpack_context* unpack_context);
uint8_t cw_unpack_next_unsigned8 (cw_unpack_context* unpack_context);

float cw_unpack_next_float (cw_unpack_context* unpack_context);
double cw_unpack_next_double (cw_unpack_context* unpack_context);
double cw_unpack_next_time_interval (cw_unpack_context* unpack_context);
#define cw_unpack_next_real cw_unpack_next_double                           /* Backward compatibility */

unsigned int cw_unpack_next_str_lengh (cw_unpack_context* unpack_context);

unsigned int cw_unpack_next_array_size(cw_unpack_context* unpack_context);
unsigned int cw_unpack_next_map_size(cw_unpack_context* unpack_context);

#endif  /* CWPack_utils_H__ */

