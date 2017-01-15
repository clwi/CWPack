/*      CWPack Module test - main.c   */
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
#include <sys/fcntl.h>
//#include <sys/errno.h>

#include "cwpack.h"


cw_pack_context testbuf;
cw_unpack_context testinbuf;
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


static void check_pack_result(const char* expected_header, unsigned long data_length)
{
    // expected contains the result in HEX
    unsigned long header_length = strlen(expected_header) / 2;
    if (testbuf.current - outbuffer == (long)(header_length + data_length))
    {
        
        if (header_length*2 == strlen(expected_header))
        {
            unsigned long i;
            uint_fast8_t *p = (uint_fast8_t*)outbuffer;
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


static void check_pack(int call, int result)
{
    int rc = call;
    if (rc != result)
        ERROR2("rc=", rc, result);
}

static void check_unpack(int val, int result)
{
    int rc = cw_unpack_next(&testinbuf);
    if (rc != result)
        ERROR2("rc=", rc, result);
    if (rc == 0 && testinbuf.item.type != CWP_ITEM_POSITIVE_INTEGER && testinbuf.item.as.i64 != val)
        ERROR("Wrong item");
}




int main(int argc, const char * argv[])
{
    printf("CWPack module test started.\n");
    error_count = 0;

        
    //*******************   TEST cwpack pack  ****************************
    
    
#define TESTP(call,data,result)                     \
    testbuf.current = outbuffer;                    \
    if(cw_pack_##call (&testbuf, data))             \
        ERROR("In pack");                           \
    check_pack_result(result,0)
    
    
    
    if (cw_pack_context_init (&testbuf, outbuffer, 70000, 0))
    {
        ERROR("Compiled for wrong byte order");
        exit(1);
    }
    int i;
    unsigned int ui;
    for (ui=0; ui<70000; ui++)
    {
        TEST_area[ui] = ui & 0x7fUL;
    }
    
    
    // TESTP NIL
    cw_pack_nil(&testbuf);
    check_pack_result("c0",0);
    
    // TESTP boolean
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
    TESTP(double,0.0,"cb0000000000000000");
    TESTP(double,f1,"cb40091eb860000000");
    TESTP(double,3.14,"cb40091eb851eb851f");
    
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
    testbuf.current = outbuffer;                        \
    if(cw_pack_##call (&testbuf, TEST_area, len))       \
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
    testbuf.current = outbuffer;                        \
    if(cw_pack_##call (&testbuf, type, TEST_area, len)) \
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
    
    
    
    //*******************   TEST cwpack unpack   **********************
    
    char inputbuf[30];
    
    
#define TESTUP(buffer,etype)                                                \
{                                                                           \
    unsigned long len = strlen(buffer)/2;                                            \
    for (ui = 0; ui < len; ui++)                                               \
    inputbuf[ui] = (uint8_t)(char2hex(buffer[2*ui])<<4) + char2hex(buffer[2*ui +1]);    \
    cw_unpack_context_init (&testinbuf, inputbuf, len+blob_length, 0);   \
    if ((i = cw_unpack_next(&testinbuf)))                                   \
        ERROR1("In unpack_next, rc = ",i);                                  \
    if (testinbuf.item.type != CWP_ITEM_##etype)                            \
    ERROR("In unpack, type error");                                         \
}
    
#define TESTUP_VAL(buffer,etype,var,val)                    \
    TESTUP(buffer,etype);                                   \
    if (testinbuf.item.as.var != val)                       \
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
    
    cw_pack_context_init (&testbuf, outbuffer, 100, 0);
    check_pack( cw_pack_array_size(&testbuf,2), CWP_RC_OK);
    check_pack( cw_pack_str(&testbuf,"Test of skip",12), CWP_RC_OK); //array component
    check_pack( cw_pack_unsigned(&testbuf,0x68357), CWP_RC_OK); //array component
    check_pack( cw_pack_unsigned(&testbuf,0x952), CWP_RC_OK); //first item after array
    
    cw_unpack_context_init (&testinbuf, testbuf.start, (unsigned long)(testbuf.current-testbuf.start), 0);

    cw_skip_items (&testinbuf, 1); /* skip whole array */
    check_unpack (0x952, CWP_RC_OK);
    check_unpack (0, CWP_RC_END_OF_INPUT);
    
    
    
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
