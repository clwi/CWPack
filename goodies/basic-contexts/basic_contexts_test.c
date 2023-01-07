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
#include <math.h>
#include <sys/file.h>
#include <errno.h>

#include "cwpack.h"
#include "cwpack_internals.h"
#include "basic_contexts.h"

#include "cwpack_utils.h"



cw_pack_context pack_ctx;
cw_pack_context *pctx = &pack_ctx;
cw_unpack_context unpack_ctx;
cw_unpack_context *unpctx = &unpack_ctx;
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


static void FILEERROR(const char* file, const char* action)
{
    error_count++;
    printf("ERROR: File: \"%s\", %s error: %d\n", file, action, errno);
}


static char char2hex (char c)
{
    if (c <= '9')
        return c - '0';
    if (c <= 'F')
        return c - 'A' + 10;

    return c - 'a' + 10;
}

static void checkArea(const void* area, const char* expected)
{
    unsigned long expected_length = strlen(expected) / 2;
    unsigned long i;
    uint8_t *p = (uint8_t*)area;
    const char *ucp = expected;
    for (i = 0; i < expected_length; i++)
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
            ERROR("Different area value");
        }
    }
}


static void check_file_result(const char* fileName, const char* expexted_value)
{
    char buffer[1000] = {0};
    int fd = open(fileName, O_RDONLY);
    if (fd < 0) FILEERROR(fileName, "open(read)");
    unsigned long expected_length = strlen(expexted_value)/2;
    long rc = read(fd, buffer, expected_length);
    if (rc != (long)expected_length) FILEERROR(fileName, "read");
    checkArea(buffer, expexted_value);
}



int main(int argc, const char * argv[])
{
    printf("CWPack basic context test started.\n");
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

    cw_pack_context_init (&pack_ctx, outbuffer, 70000, 0);
    if (pctx->return_code == CWP_RC_WRONG_BYTE_ORDER)
    {
        ERROR("***** Compiled for wrong byte order, test terminated *****\n\n");
        exit(1);
    }


    //*******************   TEST basic context  ****************************

    /*****************************************  DYNAMIC MEMORY PACK CONTEXT  ************************/
    {
        printf("Dynamic Memory Pack Context test started.\n");

        dynamic_memory_pack_context dmpc;;
        init_dynamic_memory_pack_context(&dmpc,4);

        pctx = &dmpc.pc;
        cw_pack_cstr(pctx, "AAAABBBB");
        checkArea(pctx->start, "A84141414142424242");
    }
    /*****************************************  STREAM PACK CONTEXT  *********************************/
    {
        printf("Stream Pack Context test started.\n");

        stream_pack_context spc;
        FILE *f = fopen("sptest.dat", "w+b");
        init_stream_pack_context(&spc,4,f);

        pctx = &spc.pc;
        cw_pack_nil(pctx);
        cw_pack_str_size(pctx, 8);
        cw_pack_insert(pctx,"AAAABBBB",8);
        cw_pack_cstr(pctx, "CC");
        terminate_stream_pack_context(&spc);
        fclose(f);

        check_file_result("sptest.dat","C0A84141414142424242A24343");
    }
    /*****************************************  STREAM UNPACK CONTEXT  *********************************/
    {
        printf("Stream Unpack Context test started.\n");

        stream_unpack_context supc;
        FILE *f = fopen("sptest.dat", "r");
        init_stream_unpack_context(&supc,4,f);

        unpctx = &supc.uc;
        cw_unpack_next_nil(unpctx);
        if (pctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_nil return_code:", pctx->return_code);

        cw_unpack_next_descriptor(unpctx);
        if (unpctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_descriptor return_code:", unpctx->return_code);
        if (unpctx->item.type != CWP_ITEM_STR)  ERROR1("cw_unpack_next_descriptor item.type:", unpctx->item.type);
        if (unpctx->item.as.
            str.length != 8)  ERROR1("cw_unpack_next_descriptor str.length:", unpctx->item.as.str.length);

        cw_unpack_data(unpctx, outbuffer, 8);
        if (pctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_descriptor return_code:", pctx->return_code);
        checkArea(outbuffer,"4141414142424242");

        if (cw_unpack_next_str_length(unpctx) != 2) ERROR("Wrong str length");
        checkArea(unpctx->item.as.str.start,"4343");
        if (supc.buffer_length > 4) ERROR("Buffer has expanded");

        terminate_stream_unpack_context(&supc);
        remove("sptest.dat");
    }
    /*****************************************  FILE PACK CONTEXT  *********************************/
    {
        printf("File Pack Context test started.\n");

        file_pack_context fpc;
        int fd = open("fptest.dat", O_RDWR + O_CREAT + O_TRUNC, 0666);
        init_file_pack_context(&fpc,4,fd);

        pctx = &fpc.pc;
        cw_pack_nil(pctx);
        cw_pack_str_size(pctx, 8);
        cw_pack_insert(pctx,"AAAABBBB",8);
        cw_pack_cstr(pctx, "C");
        if (fpc.pc.end - fpc.pc.start > 4) ERROR("Buffer has expanded");
        terminate_file_pack_context(&fpc);
        close(fd);

        check_file_result("fptest.dat","C0A84141414142424242A143");
    }
    /*****************************************  FILE UNPACK CONTEXT  *********************************/
    {
        printf("File Unpack Context test started.\n");

        file_unpack_context fupc;
        int fd = open("fptest.dat", O_RDONLY);
        init_file_unpack_context(&fupc,4,fd);

        unpctx = &fupc.uc;
        cw_unpack_next_nil(unpctx);
        if (pctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_nil return_code:", pctx->return_code);

        cw_unpack_next_descriptor(unpctx);
        if (unpctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_descriptor return_code:", unpctx->return_code);
        if (unpctx->item.type != CWP_ITEM_STR)  ERROR1("cw_unpack_next_descriptor item.type:", unpctx->item.type);
        if (unpctx->item.as.
            str.length != 8)  ERROR1("cw_unpack_next_descriptor str.length:", unpctx->item.as.str.length);

        cw_unpack_data(unpctx, outbuffer, 8);
        if (pctx->return_code != CWP_RC_OK) ERROR1("cw_unpack_next_descriptor return_code:", pctx->return_code);
        checkArea(outbuffer,"4141414142424242");

        if (cw_unpack_next_str_length(unpctx) != 1) ERROR("Wrong str length");
        checkArea(unpctx->item.as.str.start,"43");
        if (fupc.buffer_length > 4) ERROR("Buffer has expanded");

        terminate_file_unpack_context(&fupc);
        remove("fptest.dat");
    }
 //************************************************************* End

    printf("\nCWPack basic context test completed, ");
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


