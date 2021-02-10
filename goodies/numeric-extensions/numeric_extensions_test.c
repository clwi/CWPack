/*      CWPack/goodies - numeric_extensions_test.c   */
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
#include <stdlib.h>
#include <string.h>

#include "cwpack.h"
#include "cwpack_internals.h"
#include "numeric_extensions.h"




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



int main(int argc, const char * argv[])
{
    printf("CWPack numeric extensions test started.\n");
    error_count = 0;

    bool endian_switch_found = false;
#ifdef COMPILE_FOR_BIG_ENDIAN
    printf("Compiled for big endian.\n\n");
    endian_switch_found = true;
#endif

#ifdef COMPILE_FOR_LITTLE_ENDIAN
    printf("Compiled for little endian.\n\n");
    endian_switch_found = true;
#endif

    if (!endian_switch_found)
    {
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
    }


    //*******************   TEST numeric extensions  ****************************

    cw_pack_context_init (&pack_ctx, outbuffer, 70000, 0);
    if (pack_ctx.return_code == CWP_RC_WRONG_BYTE_ORDER)
    {
        ERROR("***** Compiled for wrong byte order, test terminated *****\n\n");
        exit(1);
    }




#define TESTP_EXT(call,type,value,header)           \
    pack_ctx.current = outbuffer;                   \
    cw_pack_ext_##call (&pack_ctx, type, value);    \
    if(pack_ctx.return_code)                        \
        ERROR("In pack");                           \
    check_pack_result(header, 0)

    // TESTP ext
    TESTP_EXT(integer,15,1,"d40f01");
    TESTP_EXT(integer,15,128,"d50f0080");
    TESTP_EXT(integer,15,-32769,"d60fffff7fff");

    float f1 = (float)3.14;
    TESTP_EXT(float,15,f1,"d60f4048f5c3");
    TESTP_EXT(double,15,f1,"d70f40091eb860000000");

    int64_t integer_var;
    float float_var;
    double double_var;
    char inputbuf[30];

#define TESTUP_EXT(buffer,etype,call,value)                                                 \
{                                                                                           \
    unsigned long ui;                                                                       \
    unsigned long len = strlen(buffer)/2;                                                   \
    for (ui = 0; ui < len; ui++)                                                            \
        inputbuf[ui] = (uint8_t)(char2hex(buffer[2*ui])<<4) + char2hex(buffer[2*ui +1]);    \
    cw_unpack_context_init (&unpack_ctx, inputbuf, len, 0);                                 \
    cw_unpack_next(&unpack_ctx);                                                            \
    if (unpack_ctx.return_code)                                                             \
        ERROR1("In unpack_next, rc = ",unpack_ctx.return_code);                             \
    if (unpack_ctx.item.type != etype)                                                      \
        ERROR("In unpack, type error");                                                     \
    call##_var = get_ext_##call (&unpack_ctx);                                              \
    if (unpack_ctx.return_code)                                                             \
        ERROR1("In get_ext, rc = ",unpack_ctx.return_code);                                 \
    if (call##_var != value)                                                                \
        ERROR("In unpack, value error");                                                    \
}

    TESTUP_EXT("d40f01",15,integer,1);
    TESTUP_EXT("d50f0080",15,integer,128);
    TESTUP_EXT("d60fffff7fff",15,integer,-32769);
    TESTUP_EXT("d60f4048f5c3",15,float,f1);
    TESTUP_EXT("d70f40091eb860000000",15,double,(double)f1);
    //*************************************************************

    printf("CWPack numeric extensions test completed, ");
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


