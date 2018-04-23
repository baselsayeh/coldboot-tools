/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Module Name:

    wctype.h
    
Abstract:

    Wide character ctype macros as specified in some version of ANSI C
    9X/200 draft.

    XXX
    IMPORTANT: THIS SUPPORTS US ENGLISH *ONLY*.  THIS DOES NOT USE ANY
               OF THE LOCALE SUPPORT WHICH IT SHOULD ULTIMATELY DO


Revision History

--*/

#ifndef _WCTYPE_H_
#define _WCTYPE_H_

#include <wchar.h>


/*

#define iswalnum(wc) ((int)(iswalpha(wc) || iswdigit(wc)))
#define iswalpha(wc) ((int)(iswupper(wc) || iswlower(wc)))
#define iswcntrl(wc) ((int)((wc >= 0x0000 && wc <= 0x001f) || (wc == 0x007f)))
#define iswdigit(wc) ((int)(wc >= L'0' && wc <= L'9'))
#define iswgraph(wc) ((int)(iswprint(wc) && !iswspace(wc)))
#define iswlower(wc) ((int)(wc >= L'a' && wc <= L'z'))
#define iswprint(wc) ((int)(wc >= 0x0020 && wc <= 0x007e))
#define iswpunct(wc) ((int)(wc >= 0x0021 && wc <= 0x002f) || \
                           (wc >= 0x003a && wc <= 0x0040) || \
                           (wc >= 0x005b && wc <= 0x0060) || \
                           (wc >= 0x007b && wc <= 0x007e))
#define iswspace(wc) ((int)((wc == L' ') || (wc >= 0x0009 && wc <= 0x0020)))
#define iswupper(wc) ((int)(wc >= L'A' && wc <= L'Z'))
#define iswxdigit(wc) ((int)( iswdigit(wc) || \
                              (wc >= L'A' && wc <= L'F') || \
                              (wc >= L'a' && wc <= L'f')) )

#define iswctype( wc, func ) ( func(wc) )
#define wctype( property ) is##property

#define towlower(wc) ( iswupper(wc) ? (wc - L'A' + L'a') : wc )
#define towupper(wc) ( iswlower(wc) ? (wc - L'a' + L'A') : wc )

*/


__BEGIN_DECLS
int	iswalnum __P((wint_t));
int	iswalpha __P((wint_t));
int	iswcntrl __P((wint_t));
int	iswdigit __P((wint_t));
int	iswgraph __P((wint_t));
int	iswlower __P((wint_t));
int	iswprint __P((wint_t));
int	iswpunct __P((wint_t));
int	iswspace __P((wint_t));
int	iswupper __P((wint_t));
int	iswxdigit __P((wint_t));
wint_t	towlower __P((wint_t));
wint_t	towupper __P((wint_t));
int	iswctype __P((wint_t, wctype_t));
wctype_t	wctype __P((const char *));
wint_t towctrans __P((wint_t, wctrans_t));
wctrans_t wctrans __P((const char *));
__END_DECLS


static const struct _wctranstab {
	const char *s;
	wint_t (*p)();
	wctype_t val;
	} wctranstab[] = {
	{"tolower", towlower, 1},
	{"toupper", towupper, 2},
	{(const char *)0, (wint_t)0, 0}};

static const struct _wctypetab {
	const char *s;
	int (*p)();
	wctype_t val;
	} wctypetab[] = {
	{"alnum", iswalnum, 1},
	{"alpha", iswalpha, 2},
	{"cntrl", iswcntrl, 3},
	{"digit", iswdigit, 4},
	{"graph", iswgraph, 5},
	{"lower", iswlower, 6},
	{"print", iswprint, 7},
	{"punct", iswpunct, 8},
	{"space", iswspace, 9},
	{"upper", iswupper, 10},
	{"xdigit", iswxdigit, 11},
	{(const char *)0, ( void * )0, 0}};

#endif  /* !_WCTYPE_H_ */
