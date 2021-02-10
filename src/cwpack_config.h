/*      CWPack - cwpack_config.h   */
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


#ifndef cwpack_config_h
#define cwpack_config_h



/*************************   A L I G N M E N T   ******************************/

/*
 * Some processors demand that integer access is to an even memory address.
 * In that case define FORCE_ALIGNMENT
 */

/* #define FORCE_ALIGNMENT */

/*
 * Some processors demand that 64 bit integer access is aligned.
 * In that case define FORCE_ALIGNMENT_64BIT
 */

/* #define FORCE_ALIGNMENT_64BIT */



/*************************   C   S Y S T E M   L I B R A R Y   ****************/

/*
 * The packer uses "memcpy" to move blobs. If you dont want to load C system library
 * for just that, define FORCE_NO_LIBRARY and CWPack will use an internal "memcpy"
 */

/* #define FORCE_NO_LIBRARY */



/*************************   B Y T E   O R D E R   ****************************/

/*
 * The pack/unpack routines are written in three versions: for big endian, for
 * little endian and insensitive to byte order. As you can get some speed gain
 * if the byte order is known, we try that when we can certainly detect it.
 * Define COMPILE_FOR_BIG_ENDIAN or COMPILE_FOR_LITTLE_ENDIAN if you know.
 */

#ifndef FORCE_ALIGNMENT
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define COMPILE_FOR_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define COMPILE_FOR_LITTLE_ENDIAN
#endif

#elif defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)

#if __BYTE_ORDER == __BIG_ENDIAN
#define COMPILE_FOR_BIG_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPILE_FOR_LITTLE_ENDIAN
#endif

#elif defined(__BIG_ENDIAN__)
#define COMPILE_FOR_BIG_ENDIAN

#elif defined(__LITTLE_ENDIAN__)
#define COMPILE_FOR_LITTLE_ENDIAN

#elif defined(__i386__) || defined(__x86_64__)
#define COMPILE_FOR_LITTLE_ENDIAN

#endif
#endif

//#undef COMPILE_FOR_LITTLE_ENDIAN



#endif /* cwpack_config_h */
