/*      CWPack/test cwpack_module_test.c   */
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



#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "cwpack.h"
#include "cwpack_config.h"
#include "cwpack_utils.h"


cw_pack_context pack_ctx;
cw_unpack_context unpack_ctx;
char TEST_area[70000];
uint8_t outbuffer[70000];

int error_count;

static void ERROR(const char* msg)
{
    error_count++;
    printf("ERROR: %s\n", msg);
}


static void ERROR1(const char* msg, int i)
{
    error_count++;
    printf("ERROR: %s%d\n", msg, i);
}

static void ERROR2(const char* msg, int i, int j)
{
    error_count++;
    printf("ERROR: %s%d != %d\n", msg, i, j);
}


static char char2hex (char c)
{
    if (c <= '9')
        return c - '0';
    if (c <= 'F')
        return c - 'A' + 10;
    
    return c - 'a' + 10;
}


static void check_pack_result(const char* expected_header, unsigned long data_length)__attribute__ ((optnone))
{
    // expected contains the result in HEX
    unsigned long header_length = strlen(expected_header) / 2;
    if (pack_ctx.current - outbuffer == (long)(header_length + data_length))
    {
        
        if (header_length*2 == strlen(expected_header))
        {
            unsigned long i;
            uint8_t *p = (uint8_t*)outbuffer;
            const char *ucp = expected_header;
            for (i = 0; i < header_length; i++)
            {
                long hex = 0;
                int j;
                for(j=0; j<2; j++)
                {
                    char uc = *(ucp++);
                    if (uc >= '0' && uc <= '9')
                    {
                        hex <<= 4;
                        hex += (uc - '0');
                        continue;
                    }
                    if (uc >= 'a' && uc <='f')
                    {
                        hex <<= 4;
                        hex += (uc - 'a' + 10);
                        continue;
                    }
                    if (uc < 'A' || uc >'F')
                    {
                        ERROR("Not a HEX value");
                    }
                    hex <<= 4;
                    hex += (uc - 'A' + 10);
                }
                
                
                if (*p++ != hex)
                {
                    ERROR("Different header value");
                }
            }
            
            if (data_length > 0)
            {
                ucp = TEST_area;
                for (i = 0; i < data_length; i++)
                    if (*p++ != *ucp++)
                    {
                        ERROR("Different data value");
                    }
            }
        }
        else
            ERROR("Odd header");
    }
    else
        ERROR("Wrong total length");
}


static void check_unpack(int val, int result)
{
    cw_unpack_next(&unpack_ctx);
    if (unpack_ctx.return_code != result)
        ERROR2("rc=", unpack_ctx.return_code, result);
    if (unpack_ctx.return_code == 0 && unpack_ctx.item.type != CWP_ITEM_POSITIVE_INTEGER && unpack_ctx.item.as.i64 != val)
        ERROR("Wrong item");
}




int main(int argc, const char * argv[])
{
    printf("CWPack module test started.\n");
    error_count = 0;
    
    bool endian_switch_found = false;
#ifdef COMPILE_FOR_BIG_ENDIAN
    printf("Compiled for big endian.\n");
    endian_switch_found = true;
#endif
    
#ifdef COMPILE_FOR_LITTLE_ENDIAN
    printf("Compiled for little endian.\n");
    endian_switch_found = true;
#endif
    
    if (!endian_switch_found)
        printf("Compiled for all endians.\n");
    
    const char *endianness = "1234";
    switch (*(uint32_t*)endianness)
    {
        case 0x31323334UL:
            printf("Running on big endian hardware.\n\n");
            break;
            
        case 0x34333231UL:
            printf("Running on little endian hardware.\n\n");
            break;
            
        default:
            printf("Running on neither little nor big endian hardware.\n\n");
            break;
    }
    
   
    //*******************   TEST cwpack pack  ****************************
    
    
#define TESTP(call,data,result)                     \
    pack_ctx.current = outbuffer;                    \
    cw_pack_##call (&pack_ctx, data);                \
    if(pack_ctx.return_code)                         \
        ERROR("In pack");                           \
    check_pack_result(result,0)
    
    
    
    cw_pack_context_init (&pack_ctx, outbuffer, 70000, 0);
    if (pack_ctx.return_code == CWP_RC_WRONG_BYTE_ORDER)
    {
        ERROR("***** Compiled for wrong byte order, test terminated *****\n\n");
        exit(1);
    }
    
    unsigned int ui;
    for (ui=0; ui<70000; ui++)
    {
        TEST_area[ui] = ui & 0x7fUL;
    }
    
    
    // TESTP NIL
    cw_pack_nil(&pack_ctx);
    check_pack_result("c0",0);
    
    // TESTP boolean
    pack_ctx.current = outbuffer;
    cw_pack_true(&pack_ctx);
    check_pack_result("c3",0);
    pack_ctx.current = outbuffer;
    cw_pack_false(&pack_ctx);
    check_pack_result("c2",0);
    TESTP(boolean,0,"c2");
    TESTP(boolean,1,"c3");
    
    // TESTP unsigned int
    TESTP(unsigned,0,"00");
    TESTP(unsigned,127,"7f");
    TESTP(unsigned,128,"cc80");
    TESTP(unsigned,255,"ccff");
    TESTP(unsigned,256,"cd0100");
    TESTP(unsigned,65535,"cdffff");
    TESTP(unsigned,65536,"ce00010000");
    TESTP(unsigned,500000000,"ce1dcd6500");
    TESTP(unsigned,0xffffffffUL,"ceffffffff");
    TESTP(unsigned,0x100000000ULL,"cf0000000100000000");
    TESTP(unsigned,0xffffffffffffffffULL,"cfffffffffffffffff");
    
    // TESTP signed int
    TESTP(signed,-1,"ff");
    TESTP(signed,-32,"e0");
    TESTP(signed,-33,"d0df");
    TESTP(signed,-128,"d080");
    TESTP(signed,-129,"d1ff7f");
    TESTP(signed,-32768,"d18000");
    TESTP(signed,-32769,"d2ffff7fff");
    
    // TESTP real
    float f1 = (float)3.14;
    TESTP(float,0.0,"ca00000000");
    TESTP(float,f1,"ca4048f5c3");
    TESTP(float,37.25,"ca42150000");
    TESTP(double,0.0,"cb0000000000000000");
    TESTP(double,f1,"cb40091eb860000000");
    TESTP(double,3.14,"cb40091eb851eb851f");
    TESTP(double,37.25,"cb4042a00000000000");
    TESTP(double_opt,37.25,"ca42150000");
    TESTP(double_opt,f1,"ca4048f5c3");
    TESTP(double_opt,3.14,"cb40091eb851eb851f");
    TESTP(double_opt,-32,"e0");
    
    // TESTP array
    TESTP(array_size,0,"90");
    TESTP(array_size,15,"9f");
    TESTP(array_size,16,"dc0010");
    TESTP(array_size,65535,"dcffff");
    TESTP(array_size,65536,"dd00010000");
    
    // TESTP map
    TESTP(map_size,0,"80");
    TESTP(map_size,15,"8f");
    TESTP(map_size,16,"de0010");
    TESTP(map_size,65535,"deffff");
    TESTP(map_size,65536,"df00010000");
    
    
#define TESTP_AREA(call,len,header)                     \
    pack_ctx.current = outbuffer;                        \
    cw_pack_##call (&pack_ctx, TEST_area, len);          \
    if(pack_ctx.return_code)                             \
        ERROR("In pack");                               \
    check_pack_result(header, len)
    
    // TESTP str
    TESTP_AREA(str,0,"a0");
    TESTP_AREA(str,31,"bf");
    TESTP_AREA(str,32,"d920");
    TESTP_AREA(str,255,"d9ff");
    TESTP_AREA(str,256,"da0100");
    TESTP_AREA(str,65535,"daffff");
    TESTP_AREA(str,65536,"db00010000");
    
    // TESTP bin
    TESTP_AREA(bin,0,"c400");
    TESTP_AREA(bin,255,"c4ff");
    TESTP_AREA(bin,256,"c50100");
    TESTP_AREA(bin,65535,"c5ffff");
    TESTP_AREA(bin,65536,"c600010000");
    
#define TESTP_EXT(call,type,len,header)                 \
    pack_ctx.current = outbuffer;                        \
    cw_pack_##call (&pack_ctx, type, TEST_area, len);    \
    if(pack_ctx.return_code)                             \
        ERROR("In pack");                               \
    check_pack_result(header, len)
    
    // TESTP ext
    TESTP_EXT(ext,15,1,"d40f");
    TESTP_EXT(ext,16,2,"d510");
    TESTP_EXT(ext,17,3,"c70311");
    TESTP_EXT(ext,18,4,"d612");
    TESTP_EXT(ext,19,8,"d713");
    TESTP_EXT(ext,20,16,"d814");
    TESTP_EXT(ext,21,255,"c7ff15");
    TESTP_EXT(ext,21,256,"c8010015");
    TESTP_EXT(ext,21,65535,"c8ffff15");
    TESTP_EXT(ext,21,65536,"c90001000015");
    
    pack_ctx.current = outbuffer;
    cw_pack_time(&pack_ctx, 1, 0);
    check_pack_result("d6ff00000001", 0);
    pack_ctx.current = outbuffer;
    cw_pack_time(&pack_ctx, 1, 2);
    check_pack_result("d7ff0000000800000001", 0);
    pack_ctx.current = outbuffer;
    cw_pack_time(&pack_ctx, -1, 500000000);
    check_pack_result("c70cff1dcd6500ffffffffffffffff", 0);

    TESTP(time_interval,-0.5,"c70cff1dcd6500ffffffffffffffff");
    
    //*******************   TEST cwpack unpack   **********************
    
    char inputbuf[30];
    
    
#define TESTUP(buffer,etype)                                                            \
{                                                                                       \
    unsigned long len = strlen(buffer)/2;                                               \
    for (ui = 0; ui < len; ui++)                                                        \
    inputbuf[ui] = (uint8_t)(char2hex(buffer[2*ui])<<4) + char2hex(buffer[2*ui +1]);    \
    cw_unpack_context_init (&unpack_ctx, inputbuf, len+blob_length, 0);                 \
    cw_unpack_next(&unpack_ctx);                                                       \
    if (unpack_ctx.item.type != CWP_ITEM_##etype)                                       \
        ERROR("In unpack, type error");                                                 \
}
    
#define TESTUP_VAL(buffer,etype,var,val)                    \
    TESTUP(buffer,etype);                                   \
    if (unpack_ctx.item.as.var != val)                      \
        ERROR("In unpack, value error");

    unsigned long blob_length = 0;
    
    // TESTUP NIL
    TESTUP("c0",NIL);
    
    // TESTUP boolean
    TESTUP_VAL("c2",BOOLEAN,boolean,false);
    TESTUP_VAL("c3",BOOLEAN,boolean,true);
    
    // TESTUP unsigned int
    TESTUP_VAL("00",POSITIVE_INTEGER,u64,0)
    TESTUP_VAL("7f",POSITIVE_INTEGER,u64,127)
    TESTUP_VAL("cc80",POSITIVE_INTEGER,u64,128)
    TESTUP_VAL("ccff",POSITIVE_INTEGER,u64,255)
    TESTUP_VAL("cd0100",POSITIVE_INTEGER,u64,256)
    TESTUP_VAL("cdffff",POSITIVE_INTEGER,u64,65535)
    TESTUP_VAL("ce00010000",POSITIVE_INTEGER,u64,65536)
    TESTUP_VAL("ceffffffff",POSITIVE_INTEGER,u64,0xffffffffUL)
    TESTUP_VAL("cf0000000100000000",POSITIVE_INTEGER,u64,0x100000000ULL)
    TESTUP_VAL("cfffffffffffffffff",POSITIVE_INTEGER,u64,0xffffffffffffffffULL)
    
    // TESTUP signed int
    TESTUP_VAL("ff",NEGATIVE_INTEGER,i64,-1)
    TESTUP_VAL("e0",NEGATIVE_INTEGER,i64,-32)
    TESTUP_VAL("d0df",NEGATIVE_INTEGER,i64,-33)
    TESTUP_VAL("d080",NEGATIVE_INTEGER,i64,-128)
    TESTUP_VAL("d1ff7f",NEGATIVE_INTEGER,i64,-129)
    TESTUP_VAL("d18000",NEGATIVE_INTEGER,i64,-32768)
    TESTUP_VAL("d2ffff7fff",NEGATIVE_INTEGER,i64,-32769)
    TESTUP_VAL("d3ffffffff7fffffff",NEGATIVE_INTEGER,i64,-2147483649)
    
    // TESTUP real
    //    float f1 = 3.14;
    TESTUP_VAL("ca00000000",FLOAT,real,0.0)
    TESTUP_VAL("ca4048f5c3",FLOAT,real,f1)
    TESTUP_VAL("cb0000000000000000",DOUBLE,long_real,0.0)
    TESTUP_VAL("cb40091eb860000000",DOUBLE,long_real,f1)
    TESTUP_VAL("cb40091eb851eb851f",DOUBLE,long_real,3.14)
    
    // TESTUP array
    TESTUP_VAL("90",ARRAY,array.size,0)
    TESTUP_VAL("9f",ARRAY,array.size,15)
    TESTUP_VAL("dc0010",ARRAY,array.size,16)
    TESTUP_VAL("dcffff",ARRAY,array.size,65535)
    TESTUP_VAL("dd00010000",ARRAY,array.size,65536)
    
    // TESTUP map
    TESTUP_VAL("80",MAP,map.size,0)
    TESTUP_VAL("8f",MAP,map.size,15)
    TESTUP_VAL("de0010",MAP,map.size,16)
    TESTUP_VAL("deffff",MAP,map.size,65535)
    TESTUP_VAL("df00010000",MAP,map.size,65536)
    
    // TESTUP timeStamp
    TESTUP_VAL("d6ff00000001",TIMESTAMP,time.tv_sec,1);
    TESTUP_VAL("d6ff00000001",TIMESTAMP,time.tv_nsec,0);
    TESTUP_VAL("d7ff0000000800000001",TIMESTAMP,time.tv_sec,1);
    TESTUP_VAL("d7ff0000000800000001",TIMESTAMP,time.tv_nsec,2);
    TESTUP_VAL("c70cff1dcd6500ffffffffffffffff",TIMESTAMP,time.tv_sec,-1);
    TESTUP_VAL("c70cff1dcd6500ffffffffffffffff",TIMESTAMP,time.tv_nsec,500000000);

#define TESTUP_AREA(buffer,etype,blob,len)                 \
    blob_length = len;                                     \
    TESTUP_VAL(buffer,etype,blob.length,len)               \

    // TESTUP str
    TESTUP_AREA("a0",STR,str,0);
    TESTUP_AREA("bf",STR,str,31);
    TESTUP_AREA("d920",STR,str,32);
    TESTUP_AREA("d9ff",STR,str,255);
    TESTUP_AREA("da0100",STR,str,256);
    TESTUP_AREA("daffff",STR,str,65535);
    TESTUP_AREA("db00010000",STR,str,65536);
    
    // TESTUP bin
    TESTUP_AREA("c400",BIN,bin,0);
    TESTUP_AREA("c4ff",BIN,bin,255);
    TESTUP_AREA("c50100",BIN,bin,256);
    TESTUP_AREA("c5ffff",BIN,bin,65535);
    TESTUP_AREA("c600010000",BIN,bin,65536);
    
    // TESTUP ext
#define CWP_ITEM_15 15
#define CWP_ITEM_16 16
#define CWP_ITEM_17 17
#define CWP_ITEM_18 18
#define CWP_ITEM_19 19
#define CWP_ITEM_20 20
#define CWP_ITEM_21 21
    
    TESTUP_AREA("d40f",15,ext,1);
    TESTUP_AREA("d510",16,ext,2);
    TESTUP_AREA("c70311",17,ext,3);
    TESTUP_AREA("d612",18,ext,4);
    TESTUP_AREA("d713",19,ext,8);
    TESTUP_AREA("d814",20,ext,16);
    TESTUP_AREA("c7ff15",21,ext,255);
    TESTUP_AREA("c8010015",21,ext,256);
    TESTUP_AREA("c8ffff15",21,ext,65535);
    TESTUP_AREA("c90001000015",21,ext,65536);
    

    
    //*******************   TEST skip   ***************************
    
    cw_pack_context_init (&pack_ctx, outbuffer, 100, 0);
    cw_pack_array_size(&pack_ctx,2);
    cw_pack_str(&pack_ctx,"Test of skip",12); //array component
    cw_pack_unsigned(&pack_ctx,0x68357); //array component
    cw_pack_unsigned(&pack_ctx,0x952); //first item after array
    if(pack_ctx.return_code)
    {
        ERROR("Couldn't generate testdata for skip_items");
    }
    else
    {
        cw_unpack_context_init (&unpack_ctx, pack_ctx.start, (unsigned long)(pack_ctx.current-pack_ctx.start), 0);
        
        cw_skip_items (&unpack_ctx, 1); /* skip whole array */
        check_unpack (0x952, CWP_RC_OK);
        check_unpack (0, CWP_RC_END_OF_INPUT);
    }
    
    
    //*************************************************************

    printf("CWPack module test completed, ");
    switch (error_count)
    {
        case 0:
            printf("no errors detected\n");
            break;
            
        case 1:
            printf("1 error detected\n");
            break;
            
        default:
            printf("%d errors detected\n", error_count);
            break;
    }
    
    return error_count;
}
