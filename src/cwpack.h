/*      CWPack - cwpack.h   */
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

#ifndef CWPack_H__
#define CWPack_H__


#include <stdint.h>
#include <stdbool.h>
#include <time.h>



/*******************************   Return Codes   *****************************/

#define CWP_RC_OK                         0
#define CWP_RC_END_OF_INPUT              -1
#define CWP_RC_BUFFER_OVERFLOW           -2
#define CWP_RC_BUFFER_UNDERFLOW          -3
#define CWP_RC_MALFORMED_INPUT           -4
#define CWP_RC_WRONG_BYTE_ORDER          -5
#define CWP_RC_ERROR_IN_HANDLER          -6
#define CWP_RC_ILLEGAL_CALL              -7
#define CWP_RC_MALLOC_ERROR              -8
#define CWP_RC_STOPPED                   -9
#define CWP_RC_TYPE_ERROR               -10
#define CWP_RC_VALUE_ERROR              -11
#define CWP_RC_WRONG_TIMESTAMP_LENGTH   -12



/*******************************   P A C K   **********************************/


struct cw_pack_context;

typedef int (*pack_overflow_handler)(struct cw_pack_context*, unsigned long);
typedef int (*pack_flush_handler)(struct cw_pack_context*);

typedef struct cw_pack_context {
    uint8_t*                current;
    uint8_t*                start;
    uint8_t*                end;
    bool                    be_compatible;
    int                     return_code;
    int                     err_no;          /* handlers can save error here */
    pack_overflow_handler   handle_pack_overflow;
    pack_flush_handler      handle_flush;
} cw_pack_context;


int cw_pack_context_init (cw_pack_context* pack_context, void* data, unsigned long length, pack_overflow_handler hpo);
void cw_pack_set_compatibility (cw_pack_context* pack_context, bool be_compatible);
void cw_pack_set_flush_handler (cw_pack_context* pack_context, pack_flush_handler handle_flush);
void cw_pack_flush (cw_pack_context* pack_context);

void cw_pack_nil (cw_pack_context* pack_context);
void cw_pack_true (cw_pack_context* pack_context);
void cw_pack_false (cw_pack_context* pack_context);
void cw_pack_boolean (cw_pack_context* pack_context, bool b);

void cw_pack_signed (cw_pack_context* pack_context, int64_t i);
void cw_pack_unsigned (cw_pack_context* pack_context, uint64_t i);

void cw_pack_float (cw_pack_context* pack_context, float f);
void cw_pack_double (cw_pack_context* pack_context, double d);
/* void cw_pack_real (cw_pack_context* pack_context, double d);   moved to cwpack_utils */

void cw_pack_array_size (cw_pack_context* pack_context, uint32_t n);
void cw_pack_map_size (cw_pack_context* pack_context, uint32_t n);
void cw_pack_str (cw_pack_context* pack_context, const char* v, uint32_t l);
void cw_pack_bin (cw_pack_context* pack_context, const void* v, uint32_t l);
void cw_pack_ext (cw_pack_context* pack_context, int8_t type, const void* v, uint32_t l);
void cw_pack_time (cw_pack_context* pack_context, int64_t sec, uint32_t nsec);

void cw_pack_insert (cw_pack_context* pack_context, const void* v, uint32_t l);


/*****************************   U N P A C K   ********************************/


typedef enum
{
    CWP_ITEM_MIN_RESERVED_EXT       = -128,
    CWP_ITEM_TIMESTAMP              = -1,
    CWP_ITEM_MAX_RESERVED_EXT       = -1,
    CWP_ITEM_MIN_USER_EXT           = 0,
    CWP_ITEM_USER_EXT_0             = 0,
    CWP_ITEM_USER_EXT_1             = 1,
    CWP_ITEM_USER_EXT_2             = 2,
    CWP_ITEM_USER_EXT_3             = 3,
    CWP_ITEM_USER_EXT_4             = 4,
    CWP_ITEM_USER_EXT_5             = 5,
    CWP_ITEM_USER_EXT_6             = 6,
    CWP_ITEM_USER_EXT_7             = 7,
    CWP_ITEM_USER_EXT_8             = 8,
    CWP_ITEM_USER_EXT_9             = 9,
    CWP_ITEM_USER_EXT_10            = 10,
    CWP_ITEM_USER_EXT_11            = 11,
    CWP_ITEM_USER_EXT_12            = 12,
    CWP_ITEM_USER_EXT_13            = 13,
    CWP_ITEM_USER_EXT_14            = 14,
    CWP_ITEM_USER_EXT_15            = 15,
    CWP_ITEM_USER_EXT_16            = 16,
    CWP_ITEM_USER_EXT_17            = 17,
    CWP_ITEM_USER_EXT_18            = 18,
    CWP_ITEM_USER_EXT_19            = 19,
    CWP_ITEM_USER_EXT_20            = 20,
    CWP_ITEM_USER_EXT_21            = 21,
    CWP_ITEM_USER_EXT_22            = 22,
    CWP_ITEM_USER_EXT_23            = 23,
    CWP_ITEM_USER_EXT_24            = 24,
    CWP_ITEM_USER_EXT_25            = 25,
    CWP_ITEM_USER_EXT_26            = 26,
    CWP_ITEM_USER_EXT_27            = 27,
    CWP_ITEM_USER_EXT_28            = 28,
    CWP_ITEM_USER_EXT_29            = 29,
    CWP_ITEM_USER_EXT_30            = 30,
    CWP_ITEM_USER_EXT_31            = 31,
    CWP_ITEM_USER_EXT_32            = 32,
    CWP_ITEM_USER_EXT_33            = 33,
    CWP_ITEM_USER_EXT_34            = 34,
    CWP_ITEM_USER_EXT_35            = 35,
    CWP_ITEM_USER_EXT_36            = 36,
    CWP_ITEM_USER_EXT_37            = 37,
    CWP_ITEM_USER_EXT_38            = 38,
    CWP_ITEM_USER_EXT_39            = 39,
    CWP_ITEM_USER_EXT_40            = 40,
    CWP_ITEM_USER_EXT_41            = 41,
    CWP_ITEM_USER_EXT_42            = 42,
    CWP_ITEM_USER_EXT_43            = 43,
    CWP_ITEM_USER_EXT_44            = 44,
    CWP_ITEM_USER_EXT_45            = 45,
    CWP_ITEM_USER_EXT_46            = 46,
    CWP_ITEM_USER_EXT_47            = 47,
    CWP_ITEM_USER_EXT_48            = 48,
    CWP_ITEM_USER_EXT_49            = 49,
    CWP_ITEM_USER_EXT_50            = 50,
    CWP_ITEM_USER_EXT_51            = 51,
    CWP_ITEM_USER_EXT_52            = 52,
    CWP_ITEM_USER_EXT_53            = 53,
    CWP_ITEM_USER_EXT_54            = 54,
    CWP_ITEM_USER_EXT_55            = 55,
    CWP_ITEM_USER_EXT_56            = 56,
    CWP_ITEM_USER_EXT_57            = 57,
    CWP_ITEM_USER_EXT_58            = 58,
    CWP_ITEM_USER_EXT_59            = 59,
    CWP_ITEM_USER_EXT_60            = 60,
    CWP_ITEM_USER_EXT_61            = 61,
    CWP_ITEM_USER_EXT_62            = 62,
    CWP_ITEM_USER_EXT_63            = 63,
    CWP_ITEM_USER_EXT_64            = 64,
    CWP_ITEM_USER_EXT_65            = 65,
    CWP_ITEM_USER_EXT_66            = 66,
    CWP_ITEM_USER_EXT_67            = 67,
    CWP_ITEM_USER_EXT_68            = 68,
    CWP_ITEM_USER_EXT_69            = 69,
    CWP_ITEM_USER_EXT_70            = 70,
    CWP_ITEM_USER_EXT_71            = 71,
    CWP_ITEM_USER_EXT_72            = 72,
    CWP_ITEM_USER_EXT_73            = 73,
    CWP_ITEM_USER_EXT_74            = 74,
    CWP_ITEM_USER_EXT_75            = 75,
    CWP_ITEM_USER_EXT_76            = 76,
    CWP_ITEM_USER_EXT_77            = 77,
    CWP_ITEM_USER_EXT_78            = 78,
    CWP_ITEM_USER_EXT_79            = 79,
    CWP_ITEM_USER_EXT_80            = 80,
    CWP_ITEM_USER_EXT_81            = 81,
    CWP_ITEM_USER_EXT_82            = 82,
    CWP_ITEM_USER_EXT_83            = 83,
    CWP_ITEM_USER_EXT_84            = 84,
    CWP_ITEM_USER_EXT_85            = 85,
    CWP_ITEM_USER_EXT_86            = 86,
    CWP_ITEM_USER_EXT_87            = 87,
    CWP_ITEM_USER_EXT_88            = 88,
    CWP_ITEM_USER_EXT_89            = 89,
    CWP_ITEM_USER_EXT_90            = 90,
    CWP_ITEM_USER_EXT_91            = 91,
    CWP_ITEM_USER_EXT_92            = 92,
    CWP_ITEM_USER_EXT_93            = 93,
    CWP_ITEM_USER_EXT_94            = 94,
    CWP_ITEM_USER_EXT_95            = 95,
    CWP_ITEM_USER_EXT_96            = 96,
    CWP_ITEM_USER_EXT_97            = 97,
    CWP_ITEM_USER_EXT_98            = 98,
    CWP_ITEM_USER_EXT_99            = 99,
    CWP_ITEM_USER_EXT_100           = 100,
    CWP_ITEM_USER_EXT_101           = 101,
    CWP_ITEM_USER_EXT_102           = 102,
    CWP_ITEM_USER_EXT_103           = 103,
    CWP_ITEM_USER_EXT_104           = 104,
    CWP_ITEM_USER_EXT_105           = 105,
    CWP_ITEM_USER_EXT_106           = 106,
    CWP_ITEM_USER_EXT_107           = 107,
    CWP_ITEM_USER_EXT_108           = 108,
    CWP_ITEM_USER_EXT_109           = 109,
    CWP_ITEM_USER_EXT_110           = 110,
    CWP_ITEM_USER_EXT_111           = 111,
    CWP_ITEM_USER_EXT_112           = 112,
    CWP_ITEM_USER_EXT_113           = 113,
    CWP_ITEM_USER_EXT_114           = 114,
    CWP_ITEM_USER_EXT_115           = 115,
    CWP_ITEM_USER_EXT_116           = 116,
    CWP_ITEM_USER_EXT_117           = 117,
    CWP_ITEM_USER_EXT_118           = 118,
    CWP_ITEM_USER_EXT_119           = 119,
    CWP_ITEM_USER_EXT_120           = 120,
    CWP_ITEM_USER_EXT_121           = 121,
    CWP_ITEM_USER_EXT_122           = 122,
    CWP_ITEM_USER_EXT_123           = 123,
    CWP_ITEM_USER_EXT_124           = 124,
    CWP_ITEM_USER_EXT_125           = 125,
    CWP_ITEM_USER_EXT_126           = 126,
    CWP_ITEM_USER_EXT_127           = 127,
    CWP_ITEM_MAX_USER_EXT           = 127,
    
    CWP_ITEM_NIL                    = 300,
    CWP_ITEM_BOOLEAN                = 301,
    CWP_ITEM_POSITIVE_INTEGER       = 302,
    CWP_ITEM_NEGATIVE_INTEGER       = 303,
    CWP_ITEM_FLOAT                  = 304,
    CWP_ITEM_DOUBLE                 = 305,
    CWP_ITEM_STR                    = 306,
    CWP_ITEM_BIN                    = 307,
    CWP_ITEM_ARRAY                  = 308,
    CWP_ITEM_MAP                    = 309,
    CWP_ITEM_EXT                    = 310,
    CWP_NOT_AN_ITEM                 = 999
} cwpack_item_types;


typedef struct {
    const void*     start;
    uint32_t        length;
} cwpack_blob;


typedef struct {
    uint32_t    size;
} cwpack_container;


typedef struct {
    int64_t     tv_sec;
    uint32_t    tv_nsec;
} cwpack_timespec;


typedef struct {
    cwpack_item_types   type;
    union
    {
        bool            boolean;
        uint64_t        u64;
        int64_t         i64;
        float           real;
        double          long_real;
        cwpack_container array;
        cwpack_container map;
        cwpack_blob     str;
        cwpack_blob     bin;
        cwpack_blob     ext;
        cwpack_timespec time;
    } as;
} cwpack_item;

struct cw_unpack_context;

typedef int (*unpack_underflow_handler)(struct cw_unpack_context*, unsigned long);

typedef struct cw_unpack_context {
    cwpack_item                 item;
    uint8_t*                    start;
    uint8_t*                    current;
    uint8_t*                    end;             /* logical end of buffer */
    int                         return_code;
    int                         err_no;          /* handlers can save error here */
    unpack_underflow_handler    handle_unpack_underflow;
} cw_unpack_context;



int cw_unpack_context_init (cw_unpack_context* unpack_context, const void* data, unsigned long length, unpack_underflow_handler huu);

void cw_unpack_next (cw_unpack_context* unpack_context);
void cw_skip_items (cw_unpack_context* unpack_context, long item_count);
cwpack_item_types cw_look_ahead (cw_unpack_context* unpack_context);


#endif  /* CWPack_H__ */
