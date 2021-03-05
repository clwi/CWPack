/*      CWPack/goodies - numeric_extensions.c   */
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


#include "cwpack_internals.h"
#include "numeric_extensions.h"


#define cw_storeN(n,op,ant)                     \
{                                               \
    cw_pack_reserve_space(n);                   \
    *p++ = (uint8_t)op;     \
    *p++ = (uint8_t)type;   \
    cw_store##ant(i);                           \
    return;                           \
}

#define cw_store8(i) *p = (uint8_t)i;


void cw_pack_ext_integer (cw_pack_context* pack_context, int8_t type, int64_t i)
{
    if (pack_context->return_code)
        return;

    uint8_t *p;

    if (i >= 0)
    {
        if (i < 128)
            cw_storeN(3,0xd4,8);

        if (i < 32768)
            cw_storeN(4,0xd5,16);

        if (i < 0x80000000LL)
            cw_storeN(6,0xd6,32);

        cw_storeN(10,0xd7,64);
    }

    if (i >= -128)
        cw_storeN(3,0xd4,8);

    if (i >= -32768)
        cw_storeN(4,0xd5,16);

    if (i >= (int64_t)0xffffffff80000000LL)
        cw_storeN(6,0xd6,32);

    cw_storeN(10,0xd7,64);
}


void cw_pack_ext_float (cw_pack_context* pack_context, int8_t type, float f)
{
    if (pack_context->return_code)
        return;

    uint8_t *p;

    cw_pack_reserve_space(6);
    *p++ = (uint8_t)0xd6;
    *p++ = (uint8_t)type;

    uint32_t tmp = *((uint32_t*)&f);
    cw_store32(tmp);
}


void cw_pack_ext_double (cw_pack_context* pack_context, int8_t type, double d)
{
    if (pack_context->return_code)
        return;

    uint8_t *p;

    cw_pack_reserve_space(10);
    *p++ = (uint8_t)0xd7;
    *p++ = (uint8_t)type;

    uint64_t tmp = *((uint64_t*)&d);
    cw_store64(tmp);
}


int64_t get_ext_integer (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code != CWP_RC_OK)
    {
        return 0;
    }

    uint16_t    tmpu16;
    uint32_t    tmpu32;
    uint64_t    tmpu64;

    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        unpack_context->return_code = CWP_RC_TYPE_ERROR;
        return 0;
    }

    switch (unpack_context->item.as.ext.length) {
        case 0:
            return 0;

        case 1:
            return *(int8_t*)unpack_context->item.as.ext.start;

        case 2:
            cw_load16(unpack_context->item.as.ext.start);
            return (int16_t)tmpu16;

        case 4:
            cw_load32(unpack_context->item.as.ext.start);
            return (int32_t)tmpu32;

        case 8:
            cw_load64(unpack_context->item.as.ext.start,tmpu64);
            return (int64_t)tmpu64;

        default:
            unpack_context->return_code = CWP_RC_VALUE_ERROR;
    }
    return 0;
}


float get_ext_float (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code != CWP_RC_OK)
    {
        return 0;
    }

    uint32_t    tmpu32;

    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        unpack_context->return_code = CWP_RC_TYPE_ERROR;
        return 0.0;
    }

    if (unpack_context->item.as.ext.length != 4)
    {
        unpack_context->return_code = CWP_RC_VALUE_ERROR;
        return 0.0;
    }

    cw_load32(unpack_context->item.as.ext.start);
    return *(float*)&tmpu32;
}


double get_ext_double (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code != CWP_RC_OK)
    {
        return 0;
    }

    uint64_t    tmpu64;

    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        unpack_context->return_code = CWP_RC_TYPE_ERROR;
        return 0.0;
    }

    if (unpack_context->item.as.ext.length != 8)
    {
        unpack_context->return_code = CWP_RC_VALUE_ERROR;
        return 0.0;
    }

    cw_load64(unpack_context->item.as.ext.start,tmpu64);
    return *(double*)&tmpu64;
}

