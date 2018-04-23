/*-
 * Copyright (c) 2007-2008
 *      Bill Paul <wpaul@windriver.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define NULL (void *)0
#define NBBY 8
#define CHAR_BIT	NBBY

#define _QUAD_HIGHWORD	1
#define _QUAD_LOWWORD	0

#if defined(__GNUCLIKE_ATTRIBUTE_MODE_DI)
typedef int __attribute__((__mode__(__DI__)))           int64_t;
typedef unsigned int __attribute__((__mode__(__DI__)))  uint64_t;
#else
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef uint16_t u_short;
typedef uint8_t u_char;
typedef uint32_t u_long;
typedef int32_t u_int;
typedef int32_t intptr_t;
typedef unsigned long uintptr_t;
typedef uint64_t uintmax_t;
typedef int64_t intmax_t;
typedef int64_t quad_t;
typedef uint64_t u_quad_t;
typedef uint32_t ptrdiff_t;
typedef uint32_t size_t;
typedef uint64_t off_t;

typedef uint64_t p4_entry_t;
typedef uint64_t p3_entry_t;
typedef uint64_t p2_entry_t;

#define __byte_swap_word_const(x) \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))

#define __byte_swap_int_const(x) \
        ((((x) & 0xff000000) >> 24) | \
         (((x) & 0x00ff0000) >>  8) | \
         (((x) & 0x0000ff00) <<  8) | \
         (((x) & 0x000000ff) << 24))

/*
 * General byte order swapping functions.
 */

extern uint16_t bswap16(uint16_t);
extern uint32_t bswap32(uint32_t);
extern uint64_t bswap64(uint64_t);

#ifdef notdef
#define bswap16(x)      __byte_swap_word_const(x)
#define bswap32(x)      __byte_swap_int_const(x)
#define bswap64(x)      /* not yet */
#endif

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htons(x)	bswap16((x))
#define ntohs(x)	bswap16((x))
#define htonl(x)	bswap32((x))
#define ntohl(x)	bswap32((x))
#define htonll(x)	bswap64(x)
#define ntohll(x)	bswap64(x)
#define htobe16(x)      bswap16((x))
#define htobe32(x)      bswap32((x))
#define htobe64(x)      bswap64((x))
#define htole16(x)      ((UINT16)(x))
#define htole32(x)      ((UINT32)(x))
#define htole64(x)      ((UINT64)(x))

#define be16toh(x)      bswap16((x))
#define be32toh(x)      bswap32((x))
#define be64toh(x)      bswap64((x))
#define le16toh(x)      ((UINT16)(x))
#define le32toh(x)      ((UINT32)(x))
#define le64toh(x)      ((UINT64)(x))
#else /* _BYTE_ORDER != _LITTLE_ENDIAN */
#define htons(x)	((x))
#define ntohs(x)	((x))
#define htonl(x)	((x))
#define ntohl(x)	((x))
#define htonll(x)	(x)
#define ntohll(x)	(x)
#define htobe16(x)      ((UINT16)(x))
#define htobe32(x)      ((UINT32)(x))
#define htobe64(x)      ((UINT64)(x))
#define htole16(x)      bswap16((x))
#define htole32(x)      bswap32((x))
#define htole64(x)      bswap64((x))

#define be16toh(x)      ((UINT16)(x))
#define be32toh(x)      ((UINT32)(x))
#define be64toh(x)      ((UINT64)(x))
#define le16toh(x)      bswap16((x))
#define le32toh(x)      bswap32((x))
#define le64toh(x)      bswap64((x))
#endif /* _BYTE_ORDER == _LITTLE_ENDIAN */
