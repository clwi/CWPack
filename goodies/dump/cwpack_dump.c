
/*      CWPack/goodies/dump - cwpack_dump.c   */

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
#include <unistd.h>
#include <string.h>
#include "basic_contexts.h"
#include "numeric_extensions.h"

char tabString[21] = "                     ";
bool recognizeObjects = false;

#define NEW_LINE(tablevel) {printf ("\n%6x  ",(unsigned)(context->current - context->start)); for (ti=0; ti<tablevel; ti++) printf ("%s",tabString);}
#define CHECK_NEW_LINE if(*tabString) NEW_LINE(tabLevel) else if (i) printf(" ")

/*******************************   DUMP NEXT ITEM   **********************************/

static void dump_as_hex(const void* area, long length)
{
    unsigned int i;
    unsigned char c;
    printf("<");
    for (i=0; i < length; i++)
    {
        c = ((unsigned char*)area)[i];
        printf("%02x",c);
    }
    printf(">");
}
static void dump_item( cw_unpack_context* context, int tabLevel);

static void dump_next_item( cw_unpack_context* context, int tabLevel)
{
    cw_unpack_next (context);
    if (context->return_code) return;
    dump_item (context, tabLevel);
}

static void dump_item( cw_unpack_context* context, int tabLevel)
{
    long long dim =99;
    int i,ti;
    double d;
    struct tm tm;
    char    s[128];

    switch (context->item.type)
    {
        case CWP_ITEM_NIL:
            printf("nil");
            break;

        case CWP_ITEM_BOOLEAN:
            if (context->item.as.boolean)
                printf("YES");
            else
                printf("NO");
            break;

        case CWP_ITEM_POSITIVE_INTEGER:
            printf("%llu", context->item.as.u64);
            break;

        case CWP_ITEM_NEGATIVE_INTEGER:
            printf("%lld", context->item.as.i64);
            break;

        case CWP_ITEM_FLOAT:
            context->item.as.long_real = (double)context->item.as.real;

        case CWP_ITEM_DOUBLE:
            printf ("%g", context->item.as.long_real);
            break;

        case CWP_ITEM_STR: {
            printf("\"");

            for (i=0; i < (int)context->item.as.str.length; i++)
            {
                unsigned char c = ((unsigned char*)(context->item.as.str.start))[i];
                switch (c)
                {
                    case '"':  printf("\\\""); break;
                    case '\\': printf("\\\\"); break;
                    case '\b': printf("\\b"); break;
                    case '\f': printf("\\f"); break;
                    case '\n': printf("\\n"); break;
                    case '\r': printf("\\r"); break;
                    case '\t': printf("\\t"); break;

                    default:
                        if (c < ' ')
                            printf("\\u%04x",c);
                        else
                            printf("%c",c);
                        break;
                }
            }

            printf("\"");
            break;}

        case CWP_ITEM_BIN:
            dump_as_hex (context->item.as.bin.start, context->item.as.bin.length);
            break;

        case CWP_ITEM_ARRAY:
        {
            dim = context->item.as.array.size;
            if (!dim)
            {
                printf("[]");
                break;
            }

            cw_unpack_next (context);
            if (context->return_code) break;

            if (recognizeObjects && (context->item.type == 127))
            {
                long label = get_ext_integer(context);
                bool userObject = label >= 0;
                if (dim == 1) /* reference */
                {
                    printf("->%ld",label);
                    break;
                }
                if (label)
                    printf("%ld->",label);
                if (!userObject)
                {
                    if (dim != 2)
                    {
                        context->return_code = CWP_RC_MALFORMED_INPUT;
                        break;
                    }
                    dump_next_item(context,tabLevel);
                    break;
                }
                cw_unpack_next (context);
                if (context->return_code) break;
                if (context->item.type != CWP_ITEM_STR)
                {
                    context->return_code = CWP_RC_MALFORMED_INPUT;
                    break;
                }
                printf("%.*s(",context->item.as.str.length, context->item.as.str.start);
                tabLevel++;
                for (i = 0; i < dim-2; i++)
                {
                    CHECK_NEW_LINE;
                    dump_next_item(context,tabLevel);
                }
                tabLevel--;
                if(*tabString) NEW_LINE(tabLevel);
                printf(")");
            }
            else
            {
                printf("[");
                tabLevel++;
                i = 0;
                CHECK_NEW_LINE;
                dump_item(context,tabLevel);
                for (i = 1; i < dim; i++)
                {
                    CHECK_NEW_LINE;
                    dump_next_item(context,tabLevel);
                }
                tabLevel--;
                if(*tabString) NEW_LINE(tabLevel);
                printf("]");
            }
            break;
        }

        case CWP_ITEM_MAP:
            printf("{");
            dim = context->item.as.map.size;
            tabLevel++;
            for (i = 0; i < dim; i++)
            {
                CHECK_NEW_LINE;
                dump_next_item(context,tabLevel);
                printf(":");
                dump_next_item(context,tabLevel);
            }
            tabLevel--;
            if(*tabString) NEW_LINE(tabLevel);
            printf("}");
            break;

        case CWP_ITEM_TIMESTAMP:
            printf("'");
            time_t tv_sec = context->item.as.time.tv_sec;
            gmtime_r(&tv_sec,&tm);
            strftime(s,128,"%F %T", &tm);
            printf("%s",s);
            if (context->item.as.time.tv_nsec)
            {
                d = (double)(context->item.as.time.tv_nsec) / 1000000000;
                sprintf(s,"%f",d);
                printf("%s",s+1);
            }
            printf("'");
            break;

        default:
            if (CWP_ITEM_MIN_RESERVED_EXT <= context->item.type && context->item.type <= CWP_ITEM_MAX_USER_EXT)
            {
                printf("(%d,",context->item.type);
                dump_as_hex (context->item.as.ext.start, context->item.as.ext.length);
                printf(")");
            }
            else
                printf("????? type = %d", context->item.type );
            break;
    }
}


/*******************************   M A I N   ******************************/


int main(int argc, const char * argv[])
{
    int i;
    int t = 0;
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-t") && (i++ < argc))
        {
                t = *argv[i] - '0';
                if (strlen(argv[i]) != 1 || t < 1 || t > 9) {
                    printf("Tab size must be between 1 and 9\n");
                    exit(0);
                }
        }
        else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--version"))
        {
            printf("cwpack_dump version = 1.0\n");
            exit(0);
        }
        else if (!strcmp(argv[i],"-r"))
        {
            recognizeObjects = true;
        }
        else
        {
            printf("cwpack_dump [-t 9] [-r] [-v] [-h]\n");
            printf("-h   Help\n");
            printf("-r   Recognize records\n");
            printf("-t 9 Tab size\n");
            printf("-v   Version\n");
            printf("\nIf Tab size isn't given, structures are written on a single line\n");
            printf("\nInput is taken from stdin and output is written to stdout\n");
            exit(0);
        }
    }
    tabString[t] = 0;

    file_unpack_context fuc;
    cw_unpack_context *context = (cw_unpack_context*)&fuc;

    init_file_unpack_context (&fuc, 4096, STDIN_FILENO);
    file_unpack_context_set_barrier (&fuc); /* keep whole file in memory buffer to simplify offset calculation */

    while (!context->return_code)
    {
        int ti;
        NEW_LINE(0);
        dump_next_item(context,0);
    }
    printf("\n");
    if (context->return_code != CWP_RC_END_OF_INPUT)
        printf("\nERROR RC = %d\n",context->return_code);

    terminate_file_unpack_context (&fuc);
}

