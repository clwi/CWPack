//
//  numeric_extensions.c
//  CWPack
//
//  Created by Claes Wihlborg on 2017-01-16.
//  Copyright © 2017 Claes Wihlborg. All rights reserved.
//


#include "cwpack.h"
#include "cwpack_defines.h"
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


int get_ext_integer (cw_unpack_context* unpack_context, int64_t* value)
{
    uint16_t    tmpu16;
    uint32_t    tmpu32;
    uint64_t    tmpu64;
    
    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        return NUMEXT_ERROR_NOT_EXT;
    }

    switch (unpack_context->item.as.ext.length) {
        case 1:
            *value = (int64_t)*unpack_context->item.as.ext.start;
            break;
            
        case 2:
            cw_load16(unpack_context->item.as.ext.start);
            *value = (int16_t)tmpu16;
            break;
            
        case 4:
            cw_load32(unpack_context->item.as.ext.start);
            *value = (int32_t)tmpu32;
            break;
            
        case 8:
            cw_load64(unpack_context->item.as.ext.start);
            *value = (int64_t)tmpu64;
            break;
            
        default:
            return NUMEXT_ERROR_WRONG_LENGTH;
    }
    return 0;
}


int get_ext_float (cw_unpack_context* unpack_context, float* value)
{
    uint32_t    tmpu32;

    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        return NUMEXT_ERROR_NOT_EXT;
    }
    
    if (unpack_context->item.as.ext.length != 4)
    {
        return NUMEXT_ERROR_WRONG_LENGTH;
    }
    
    cw_load32(unpack_context->item.as.ext.start);
    *value = *(float*)&tmpu32;
    return 0;
}


int get_ext_double (cw_unpack_context* unpack_context, double* value)
{
    uint64_t    tmpu64;

    if (unpack_context->item.type > CWP_ITEM_MAX_USER_EXT)
    {
        return NUMEXT_ERROR_NOT_EXT;
    }
    
    if (unpack_context->item.as.ext.length != 8)
    {
        return NUMEXT_ERROR_WRONG_LENGTH;
    }
    
    cw_load64(unpack_context->item.as.ext.start);
    *value = *(double*)&tmpu64;
    return 0;
}

