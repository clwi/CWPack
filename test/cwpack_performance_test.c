/*      CWPack/test cwpack_performance_test.c   */
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



#include <time.h>
#include <math.h>

#include "cwpack.h"
#include "cmp.h"
#include "mpack.h"
#include "basic_contexts.h"




static double milliseconds(void) {
    return (double)clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID)/1000000.0;
}


#define BEFORE_PTEST(code) \
    cw_pack_context_init(&pc, buffer, BUF_Length, 0);\
    cmp_init(&cc, buffer, 0, 0, b_writer);\
    mpack_writer_init(&mw, buffer, BUF_Length); \
    code; \
    itemSize = (int)(pc.current - pc.start); \
    memcpy(item, buffer, itemSize); \
    cw_pack_context_init(&pc, buffer, BUF_Length, 0);\


#define PTEST(packer,code) { \
    int n, max = ITERATIONS/100; \
    double duration[10]; \
    for (n=0; n<(ITERATIONS*itemSize)+1; n++) buffer[n] = (char)(0xc1); \
    for (n=0; n<10; n++) \
    { \
        double start = milliseconds(); \
        int i; \
        for (i = 0; i<max; i++) {code; code; code; code; code; code; code; code; code; code;} \
        double stopp = milliseconds(); \
        duration[n] = (stopp - start); \
    } \
    double min = duration[1]; \
    double mean = 0; \
    for (n=0; n<10; n++) {mean += 0.1 * duration[n]; if(duration[n] < min) min = duration[n];} \
    double variation = 0; \
    for (n=0; n<10; n++) variation += 0.1 * (duration[n]-mean) * (duration[n]-mean); \
    printf("Packer: %-8sCode: %-35s Min:%5.2f  Mean:%5.2f  SD:%5.2f\n", packer, #code, min, mean, sqrt(variation)); \
    char* p = buffer; \
    for (n=0; n<ITERATIONS; n++) { \
        if (memcmp(buffer,item,(unsigned long)itemSize)) {\
            printf("****** Value error *****\n"); \
            break;} \
        p += itemSize;} \
}


#define AFTER_PTEST \
    if (pc.return_code) \
        printf("Error, RC= %d\n", pc.return_code); \
    printf("\n");


#define BUF_Length 40000000
#define ITERATIONS  1000000

char buffer[BUF_Length]; /* to use for messagepack message */

static size_t b_writer(cmp_ctx_t *ctx, const void *data, size_t count)
{
    if (((char*)ctx->buf + count) > (buffer + BUF_Length))
        return 0;

    memcpy (ctx->buf,data,count);
    ctx->buf = (uint8_t*)ctx->buf + count;
    return count;
}

static void pack_test(void)
{
    /***************  Test of pack  *****************/

    cw_pack_context pc;
    cmp_ctx_t cc;
    mpack_writer_t mw;

    int ii, itemSize;
    for (ii=0; ii<BUF_Length; ii++) buffer[ii] = 0; /*Load pages in memory*/
    uint8_t item[40];


    BEFORE_PTEST(cw_pack_nil(&pc));
    PTEST("CMP",cmp_write_nil(&cc));
    PTEST("MPack", mpack_write_nil(&mw));
    PTEST("CWPack", cw_pack_nil(&pc));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_signed(&pc, -1));
    PTEST("CMP",cmp_write_integer(&cc, -1));
    PTEST("MPack", mpack_write_i64(&mw, -1));
    PTEST("CWPack", cw_pack_signed(&pc, -1));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_signed(&pc, 200));
    PTEST("CMP",cmp_write_integer(&cc, 200));
    PTEST("MPack", mpack_write_i64(&mw, 200));
    PTEST("CWPack", cw_pack_signed(&pc, 200));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_signed(&pc, 10000));
    PTEST("CMP",cmp_write_integer(&cc, 10000));
    PTEST("MPack", mpack_write_i64(&mw, 10000));
    PTEST("CWPack", cw_pack_signed(&pc, 10000));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_signed(&pc, 100000));
    PTEST("CMP",cmp_write_integer(&cc, 100000));
    PTEST("MPack", mpack_write_i64(&mw, 100000));
    PTEST("CWPack", cw_pack_signed(&pc, 100000));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_float(&pc, (float)3.14));
    PTEST("CMP",cmp_write_float(&cc, (float)3.14));
    PTEST("MPack", mpack_write_float(&mw, (float)3.14));
    PTEST("CWPack", cw_pack_float(&pc, (float)3.14));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_double(&pc, 3.14));
    PTEST("CMP",cmp_write_decimal(&cc, 3.14));
    PTEST("MPack", mpack_write_double(&mw, 3.14));
    PTEST("CWPack", cw_pack_double(&pc, 3.14));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_str(&pc, "Claes",5));
    PTEST("CMP",cmp_write_str(&cc, "Claes",5));
    PTEST("MPack", mpack_write_str(&mw, "Claes",5));
    PTEST("CWPack", cw_pack_str(&pc, "Claes",5));
    AFTER_PTEST;

    BEFORE_PTEST(cw_pack_str(&pc, "Longer string than the other one.",33));
    PTEST("CMP",cmp_write_str(&cc, "Longer string than the other one.",33));
    PTEST("MPack", mpack_write_str(&mw, "Longer string than the other one.",33));
    PTEST("CWPack", cw_pack_str(&pc, "Longer string than the other one.",33));
    AFTER_PTEST;
}

#define BEFORE_UTEST(code) { \
    cw_pack_context_init(&pc, buffer, BUF_Length, 0);\
    int i; \
    for (i = 0; i < ITERATIONS; i++) code;   \
    unsigned int l = (unsigned int)(pc.current - pc.start); \
    cw_unpack_context_init (&uc, buffer, l, 0); \
    mpack_reader_init_data (&mr, buffer, l); \
    cmp_init(&cc, buffer, b_reader, 0, 0);\
    printf("Buffer filled with: %-35s\n", #code); \
}


#define UTEST(unpacker,code) { \
    int n, max = ITERATIONS/100; \
    double duration[10]; \
    for (n=0; n<10; n++) \
    { \
        double start = milliseconds(); \
        int i; \
        for (i = 0; i<max; i++) {code; code; code; code; code; code; code; code; code; code;} \
        double stopp = milliseconds(); \
        duration[n] = (stopp - start); \
    } \
    double min = duration[1]; \
    double mean = 0; \
    for (n=0; n<10; n++) {mean += 0.1 * duration[n]; if(duration[n] < min) min = duration[n];} \
    double variation = 0; \
    for (n=0; n<10; n++) variation += 0.1 * (duration[n]-mean) * (duration[n]-mean); \
    printf("Packer: %-8sCode: %-35s Min:%5.2f  Mean:%5.2f  SD:%5.2f\n", unpacker, #code, min, mean, sqrt(variation)); \
}


#define AFTER_UTEST \
    printf("\n");


static bool b_reader(struct cmp_ctx_s *ctx, void *data, size_t limit)
{
    if (((char*)ctx->buf + limit) > (buffer + BUF_Length))
        return false;

    memcpy (data,ctx->buf,limit);
    ctx->buf = (uint8_t*)ctx->buf + limit;
    return true;
}

static void unpack_test(void)
{
    /***************  Test of unpack  *****************/

    cw_pack_context pc;
    cw_unpack_context uc;
    mpack_reader_t mr;
    cmp_ctx_t cc;
    cmp_object_t cobj;

    BEFORE_UTEST(cw_pack_nil(&pc));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_read_tag(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;

    BEFORE_UTEST(cw_pack_signed(&pc, -1));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_read_tag(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;

    BEFORE_UTEST(cw_pack_signed(&pc, 100000));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_read_tag(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;

    BEFORE_UTEST(cw_pack_float(&pc, (float)3.14));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_read_tag(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;

    BEFORE_UTEST(cw_pack_double(&pc, 3.14));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_read_tag(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;

    BEFORE_UTEST(cw_pack_str(&pc, "Claes",5));
    UTEST("CMP", cmp_read_object(&cc, &cobj));
    UTEST("MPack", mpack_skip_bytes(&mr,mpack_expect_str(&mr));mpack_done_str(&mr));
    UTEST("CWPack", cw_unpack_next(&uc));
    AFTER_UTEST;


}


int main(int argc, const char * argv[])
{
    printf("\n*****************************   PERFORMANCE TEST   *****************************\n\n");
    pack_test();
    unpack_test();
    exit (0);
}
