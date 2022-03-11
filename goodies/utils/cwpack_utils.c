/*      CWPack/goodies - cwpack_utils.c   */
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


#include <math.h>
#include "cwpack_utils.h"




/*******************************   P A C K   **********************************/


void cw_pack_double_opt (cw_pack_context* pack_context, double d)
{
    int i = (int)d;
    if ((i == d) && (i >= INT32_MIN) && (i <= UINT32_MAX))
        cw_pack_signed(pack_context, i);
    else
    {
        float f = (float)d;
        if (f == d)
            cw_pack_float (pack_context, f);
        else
            cw_pack_double (pack_context, d);
    }
}


void cw_pack_float_opt (cw_pack_context* pack_context, float f)
{
    int i = (int)f;
    if ((i == f) && (i >= INT16_MIN) && (i <= UINT16_MAX))
        cw_pack_signed(pack_context, i);
    else
        cw_pack_float (pack_context, f);
}

void cw_pack_time_interval (cw_pack_context* pack_context, double ti)
{
    int64_t  sec = (int64_t)floor(ti);
    uint32_t nsec = (uint32_t)((ti - (double)sec) * 1000000000.0);
    cw_pack_time(pack_context, sec, nsec);
}

/*******************************   U N P A C K   ******************************/

#define NaN 0

float cw_unpack_next_float (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return NaN;

    switch (unpack_context->item.type) {
        case CWP_ITEM_POSITIVE_INTEGER:     return unpack_context->item.as.u64;
        case CWP_ITEM_NEGATIVE_INTEGER:     return unpack_context->item.as.i64;
        case CWP_ITEM_FLOAT:                return unpack_context->item.as.real;
        case CWP_ITEM_DOUBLE:               return (float)unpack_context->item.as.long_real;
        default:                            unpack_context->return_code = CWP_RC_TYPE_ERROR;
                                            return NaN;
    }
}

double cw_unpack_next_double (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return NaN;

    switch (unpack_context->item.type) {
        case CWP_ITEM_POSITIVE_INTEGER:     return unpack_context->item.as.u64;
        case CWP_ITEM_NEGATIVE_INTEGER:     return unpack_context->item.as.i64;
        case CWP_ITEM_FLOAT:                return unpack_context->item.as.real;
        case CWP_ITEM_DOUBLE:               return unpack_context->item.as.long_real;
        default:                            unpack_context->return_code = CWP_RC_TYPE_ERROR;
                                            return NaN;
    }
}

void cw_unpack_next_nil (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return;
    if (unpack_context->item.type == CWP_ITEM_NIL)
        return;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return;
}



bool cw_unpack_next_boolean (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return false;

    if (unpack_context->item.type == CWP_ITEM_BOOLEAN)
        return unpack_context->item.as.boolean;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return false;
}


int64_t cw_unpack_next_signed64 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= INT64_MAX)
            return unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    if (unpack_context->item.type == CWP_ITEM_NEGATIVE_INTEGER)
        return unpack_context->item.as.i64;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


int32_t cw_unpack_next_signed32 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= INT32_MAX)
            return (int)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }
    if (unpack_context->item.type == CWP_ITEM_NEGATIVE_INTEGER)
    {
        if (unpack_context->item.as.i64 >= INT32_MIN)
            return (int)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


int16_t cw_unpack_next_signed16 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= INT16_MAX)
            return (int16_t)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }
    if (unpack_context->item.type == CWP_ITEM_NEGATIVE_INTEGER)
    {
        if (unpack_context->item.as.i64 >= INT16_MIN)
            return (int16_t)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


int8_t cw_unpack_next_signed8 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= INT8_MAX)
            return (int8_t)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }
    if (unpack_context->item.type == CWP_ITEM_NEGATIVE_INTEGER)
    {
        if (unpack_context->item.as.i64 >= INT8_MIN)
            return (int8_t)unpack_context->item.as.i64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}



uint64_t cw_unpack_next_unsigned64 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        return unpack_context->item.as.u64;
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


uint32_t cw_unpack_next_unsigned32 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= UINT32_MAX)
            return (uint32_t)unpack_context->item.as.u64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


uint16_t cw_unpack_next_unsigned16 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= UINT16_MAX)
            return (uint16_t)unpack_context->item.as.u64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


uint8_t cw_unpack_next_unsigned8 (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)
        return 0;

    if (unpack_context->item.type == CWP_ITEM_POSITIVE_INTEGER)
    {
        if (unpack_context->item.as.u64 <= UINT8_MAX)
            return (uint8_t)unpack_context->item.as.u64;
        else
        {
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
            return 0;
        }
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


double cw_unpack_next_time_interval (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return NaN;

    if (unpack_context->item.type == CWP_ITEM_TIMESTAMP)
    {
        return (double)unpack_context->item.as.time.tv_sec + (double)unpack_context->item.as.time.tv_nsec/1000000000;
    }

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return NaN;
}

unsigned int cw_unpack_next_str_lengh (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return 0;

    if (unpack_context->item.type == CWP_ITEM_STR)
        return unpack_context->item.as.str.length;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


unsigned int cw_unpack_next_bin_lengh (cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return 0;

    if (unpack_context->item.type == CWP_ITEM_BIN)
        return unpack_context->item.as.bin.length;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}


unsigned int cw_unpack_next_array_size(cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return 0;

    if (unpack_context->item.type == CWP_ITEM_ARRAY)
        return unpack_context->item.as.array.size;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}

unsigned int cw_unpack_next_map_size(cw_unpack_context* unpack_context)
{
    cw_unpack_next (unpack_context);
    if (unpack_context->return_code)        return 0;

    if (unpack_context->item.type == CWP_ITEM_MAP)
        return unpack_context->item.as.map.size;

    unpack_context->return_code = CWP_RC_TYPE_ERROR;
    return 0;
}

