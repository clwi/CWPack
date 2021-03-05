/*      CWPack/goodies - basic_contexts.c   */
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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "basic_contexts.h"




/*****************************************  DYNAMIC MEMORY PACK CONTEXT  ********************************/


static int handle_memory_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    unsigned long contains = (unsigned long)(pc->current - pc->start);
    unsigned long tot_len = contains + more;
    unsigned long buffer_length = (unsigned long)(pc->end - pc->start);
    while (buffer_length < tot_len)
        buffer_length = 2 * buffer_length;
    void *new_buffer = realloc (pc->start, buffer_length);
    if (!new_buffer)
        return CWP_RC_BUFFER_OVERFLOW;

    pc->start = (uint8_t*)new_buffer;
    pc->current = pc->start + contains;
    pc->end = pc->start + buffer_length;
    return CWP_RC_OK;
}


void init_dynamic_memory_pack_context (dynamic_memory_pack_context* dmpc, unsigned long initial_buffer_length)
{
    unsigned long buffer_length = (initial_buffer_length > 0 ? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        dmpc->pc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }

    cw_pack_context_init((cw_pack_context*)dmpc, buffer, buffer_length, &handle_memory_pack_overflow);
}


void free_dynamic_memory_pack_context(dynamic_memory_pack_context* dmpc)
{
    if (dmpc->pc.return_code != CWP_RC_MALLOC_ERROR)
        free(dmpc->pc.start);
}



/*****************************************  STREAM PACK CONTEXT  *********************************/



static int flush_stream_pack_context(struct cw_pack_context* pc)
{
    stream_pack_context* spc = (stream_pack_context*)pc;
    unsigned long contains = (unsigned long)(pc->current - pc->start);
    if (contains)
    {
        unsigned long rc = fwrite(pc->start, contains, 1, spc->file);
        if (rc != 1)
        {
            pc->err_no = ferror(spc->file);
            return CWP_RC_ERROR_IN_HANDLER;
        }
    }
    return CWP_RC_OK;
}


static int handle_stream_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    int rc = flush_stream_pack_context(pc);
    if (rc != CWP_RC_OK)
        return rc;

    unsigned long buffer_length = (unsigned long)(pc->end - pc->start);
    if (buffer_length < more)
    {
        while (buffer_length < more)
            buffer_length = 2 * buffer_length;

        void *new_buffer = malloc (buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_OVERFLOW;

        free(pc->start);
        pc->start = (uint8_t*)new_buffer;
        pc->end = pc->start + buffer_length;
    }
    pc->current = pc->start;
    return CWP_RC_OK;
}


void init_stream_pack_context (stream_pack_context* spc, unsigned long initial_buffer_length, FILE* file)
{
    unsigned long buffer_length = (initial_buffer_length > 0 ? initial_buffer_length : 4096);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        spc->pc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    spc->file = file;

    cw_pack_context_init((cw_pack_context*)spc, buffer, buffer_length, &handle_stream_pack_overflow);
    cw_pack_set_flush_handler((cw_pack_context*)spc, &flush_stream_pack_context);
}


void terminate_stream_pack_context(stream_pack_context* spc)
{
    cw_pack_context* pc = (cw_pack_context*)spc;
    cw_pack_flush(pc);

    if (pc->return_code != CWP_RC_MALLOC_ERROR)
        free(pc->start);
}



/*****************************************  STREAM UNPACK CONTEXT  *******************************/


static int handle_stream_unpack_underflow(struct cw_unpack_context* uc, unsigned long more)
{
    stream_unpack_context* suc = (stream_unpack_context*)uc;
    unsigned long remains = (unsigned long)(uc->end - uc->current);
    if (remains)
    {
        memmove (uc->start, uc->current, remains);
    }

    if (suc->buffer_length < more)
    {
        while (suc->buffer_length < more)
            suc->buffer_length = 2 * suc->buffer_length;

        void *new_buffer = realloc (uc->start, suc->buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_UNDERFLOW;

        uc->start = (uint8_t*)new_buffer;
    }
    uc->current = uc->start;
    uc->end = uc->start + remains;
    unsigned long l = fread(uc->end, 1, suc->buffer_length - remains, suc->file);
    if (!l)
    {
        if (feof(suc->file))
            return CWP_RC_END_OF_INPUT;
        suc->uc.err_no = ferror(suc->file);
        return CWP_RC_ERROR_IN_HANDLER;
    }

    uc->end += l;

    return CWP_RC_OK;
}


void init_stream_unpack_context (stream_unpack_context* suc, unsigned long initial_buffer_length, FILE* file)
{
    unsigned long buffer_length = (initial_buffer_length > 0? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        suc->uc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    suc->file = file;
    suc->buffer_length = buffer_length;

    cw_unpack_context_init((cw_unpack_context*)suc, buffer, 0, &handle_stream_unpack_underflow);
}


void terminate_stream_unpack_context(stream_unpack_context* suc)
{
    if (suc->uc.return_code != CWP_RC_MALLOC_ERROR)
        free(suc->uc.start);
}



/*****************************************  FILE PACK CONTEXT  **********************************/


static int flush_file_pack_context(struct cw_pack_context* pc)
{
    file_pack_context* fpc = (file_pack_context*)pc;
    uint8_t *bStart = fpc->barrier ? fpc->barrier : pc->current;
    unsigned long contains = (unsigned long)(bStart - pc->start);
    if (contains)
    {
        long rc = write (fpc->fileDescriptor, pc->start, contains);
        if (rc != (long)contains)
        {
            pc->err_no = errno;
            return CWP_RC_ERROR_IN_HANDLER;
        }
    }
    if (fpc->barrier)
    {
        long kept = pc->current - bStart;
        if (kept) {
            memcpy(pc->start, bStart, kept);
        }
        fpc->barrier = pc->start;
        pc->current = pc->start + kept;
    }
    else
        fpc->pc.current = fpc->pc.start;

    return CWP_RC_OK;
}

static int handle_file_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    file_pack_context* fpc = (file_pack_context*)pc;
    int rc = flush_file_pack_context(pc);
    if (rc != CWP_RC_OK)
        return rc;

    uint8_t *bStart = fpc->barrier ? fpc->barrier : pc->current;
    unsigned long kept = (unsigned long)(pc->current - bStart);
    unsigned long buffer_length = (unsigned long)(pc->end - pc->start);
    if (buffer_length < more + kept)
    {
        while (buffer_length < more + kept)
            buffer_length = 2 * buffer_length;

        void *new_buffer = malloc (buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_OVERFLOW;
        if (kept) {
            memcpy(new_buffer, bStart, kept);
        }
        pc->start = (uint8_t*)new_buffer;
        pc->end = pc->start + buffer_length;
    }
    else if (kept)
    {
        memcpy(pc->start, bStart, kept);
    }

    if (fpc->barrier)
    {
        fpc->barrier = pc->start;
    }

    pc->current = pc->start + kept;
    return CWP_RC_OK;
}


void init_file_pack_context (file_pack_context* fpc, unsigned long initial_buffer_length, int fileDescriptor)
{
    unsigned long buffer_length = (initial_buffer_length > 32 ? initial_buffer_length : 4096);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        fpc->pc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }

    fpc->fileDescriptor = fileDescriptor;
    fpc->barrier = NULL;

    cw_pack_context_init((cw_pack_context*)fpc, buffer, buffer_length, &handle_file_pack_overflow);
    cw_pack_set_flush_handler((cw_pack_context*)fpc, &flush_file_pack_context);
}


void file_pack_context_set_barrier (file_pack_context* fpc)
{
    fpc->barrier = fpc->pc.current;
}


void file_pack_context_release_barrier (file_pack_context* fpc)
{
    fpc->barrier = NULL;
}


void terminate_file_pack_context(file_pack_context* fpc)
{
    fpc->barrier = NULL;
    cw_pack_context* pc = (cw_pack_context*)fpc;
    cw_pack_flush(pc);

    if (pc->return_code != CWP_RC_MALLOC_ERROR)
        free(pc->start);
}



/*****************************************  FILE UNPACK CONTEXT  ********************************/


static int handle_file_unpack_underflow(struct cw_unpack_context* uc, unsigned long more)
{
    file_unpack_context* auc = (file_unpack_context*)uc;
    uint8_t *bStart = auc->barrier ? auc->barrier : uc->current;
    unsigned long kept = (unsigned long)(uc->current - bStart);
    unsigned long remains = (unsigned long)(uc->end - bStart);
    if (remains)
    {
        memcpy (uc->start, bStart, remains);
    }

    if (auc->buffer_length < more + kept)
    {
        while (auc->buffer_length < more + kept)
            auc->buffer_length = 2 * auc->buffer_length;

        void *new_buffer = realloc (uc->start, auc->buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_UNDERFLOW;

        uc->start = (uint8_t*)new_buffer;
    }
    uc->current = uc->start + kept;
    uc->end = uc->start + remains;
    if (auc->barrier)
        auc->barrier = uc->start;

    while ((unsigned long)(uc->end - uc->current) < more)
    {
        long l = read(auc->fileDescriptor, uc->end, auc->buffer_length - (unsigned long)(uc->end - uc->start));
        if (l == 0)
        {
            return CWP_RC_END_OF_INPUT;
        }
        if (l < 0)
        {
            auc->uc.err_no = errno;
            return CWP_RC_ERROR_IN_HANDLER;
        }
        uc->end += l;
    }

    return CWP_RC_OK;
}


void init_file_unpack_context (file_unpack_context* fuc, unsigned long initial_buffer_length, int fileDescriptor)
{
    unsigned long buffer_length = (initial_buffer_length > 0? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        fuc->uc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    fuc->fileDescriptor = fileDescriptor;
    fuc->barrier = NULL;
    fuc->buffer_length = buffer_length;

    cw_unpack_context_init((cw_unpack_context*)fuc, buffer, 0, &handle_file_unpack_underflow);
}


void file_unpack_context_set_barrier (file_unpack_context* fuc)
{
    fuc->barrier = fuc->uc.current;
}


void file_unpack_context_rescan_from_barrier (file_unpack_context* fuc)
{
    fuc->uc.current = fuc->barrier;
}

void file_unpack_context_release_barrier (file_unpack_context* fuc)
{
    fuc->barrier = NULL;
}


void terminate_file_unpack_context(file_unpack_context* fuc)
{
    if (fuc->uc.return_code != CWP_RC_MALLOC_ERROR)
        free(fuc->uc.start);
    fuc->uc.start = 0;
}


