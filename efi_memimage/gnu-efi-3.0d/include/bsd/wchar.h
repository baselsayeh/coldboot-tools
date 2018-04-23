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

    wchar.h
    
Abstract:

    ANSI wide character functions


Revision History

--*/

#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <sys/types.h>
#include <stddef.h>

typedef wchar_t wint_t;
typedef wchar_t wctype_t;
typedef wchar_t wctrans_t;
typedef int 	mbstate_t;

#ifndef _FILE_DEFINED

typedef	_BSD_OFF_T_	fpos_t;

#define	_FSTDIO			/* Define for new stdio with functions. */

/*
 * NB: to fit things in six character monocase externals, the stdio
 * code uses the prefix `__s' for stdio objects, typically followed
 * by a three-character attempt at a mnemonic.
 */

/* stdio buffers */
struct __sbuf {
	unsigned char *_base;
	int	_size;
};

/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 *
 * NB: see WARNING above before changing the layout of this structure!
 */
typedef	struct __sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
	short	_file;		/* fileno, if Unix descriptor, else -1 */
	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;	/* cookie passed to io functions */
	int	(*_close) __P((void *));
	int	(*_read)  __P((void *, char *, int));
	fpos_t	(*_seek)  __P((void *, fpos_t, int));
	int	(*_write) __P((void *, const char *, int));

	/* separate buffer for long sequences of ungetc() */
	struct	__sbuf _ub;	/* ungetc buffer */
	unsigned char *_up;	/* saved _p when _p is doing ungetc data */
	int	_ur;		/* saved _r when _r is counting ungetc data */

	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];	/* guarantee a getc() buffer */

	/* separate buffer for fgetln() when line crosses buffer boundary */
	struct	__sbuf _lb;	/* buffer for fgetln() */

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
	fpos_t	_offset;	/* current lseek offset (see WARNING) */
} FILE;

#define _FILE_DEFINED
#endif  /* _FILE_DEFINED */


#define WEOF ((wint_t)-1)

/* Wide character input/output functions */
/* Wide character input/output functions */
__BEGIN_DECLS
wint_t	fgetwc __P((FILE *));
wint_t	fputwc __P((wint_t, FILE *));
wint_t	getwc __P((FILE *));
wint_t	getwchar __P((void));
wint_t	putwc __P((wint_t, FILE *));
wint_t	putwchar __P((wint_t));
wint_t	ungetwc __P((wint_t, FILE *));
int		fputws __P((const wchar_t *, FILE *));
wchar_t	*fgetws __P((wchar_t *, int, FILE *));
wchar_t	*getws __P((wchar_t *));
int		putws __P((const wchar_t *));
int 	fwide __P((FILE *, int));
FILE	*wfopen __P((const wchar_t *, const wchar_t *));

int	 fwprintf __P((FILE *, const wchar_t *, ...));
int	 wprintf __P((const wchar_t *, ...));
int	 swprintf __P((wchar_t *, const wchar_t *, ...));
int	 vfwprintf __P((FILE *, const wchar_t *, _BSD_VA_LIST_));
int	 vwprintf __P((const wchar_t *, _BSD_VA_LIST_));
int	 vswprintf __P((wchar_t *, const wchar_t *, _BSD_VA_LIST_));

int	 fwscanf __P((FILE *, const wchar_t *, ...));
int	 wscanf __P((const wchar_t *, ...));
int	 swscanf __P((const wchar_t *, const wchar_t *, ...));
int	 vwscanf __P((const wchar_t *, _BSD_VA_LIST_));
int	 vswscanf __P((const wchar_t *, const wchar_t *, _BSD_VA_LIST_));
int	 __svfwscanf __P((FILE *, const wchar_t *, _BSD_VA_LIST_));

/*
 *  stat.h wide char equivalents
 */
int	wstat __P((const wchar_t *, struct stat *));
int	wlstat __P((const wchar_t *, struct stat *));
int	_wfaststat __P((const wchar_t *, struct stat *));
int	_wfastlstat __P((const wchar_t *, struct stat *));

/*
 *  unistd.h wide char equivalents
 */
extern int	wopterr;
extern int	woptind;
extern int	woptopt;
extern wchar_t	*woptarg;
int     wopen __P((const wchar_t *, int, ...));
int     wchdir __P((const wchar_t *));
wchar_t	*wgetcwd __P((wchar_t *, size_t));
int		wrmdir __P((const wchar_t *));
int		wmkdir __P((const wchar_t *, mode_t));
__END_DECLS


/*
 * This is a #define because the function is used internally and
 * (unlike vfwscanf) the name __svfwscanf is guaranteed not to collide
 * with a user function when _ANSI_SOURCE or _POSIX_SOURCE is defined.
 */
#define	 vfwscanf	__svfwscanf

__BEGIN_DECLS
wint_t  __sputwc __P((wint_t, FILE *));
wint_t	__sgetwc __P((FILE *));
wint_t	__swrefill __P((FILE *));



// for internal use
int  _Wcrtomb_lk __P((char *,wchar_t ,mbstate_t *));
int  _Mbrtowc_lk __P((wchar_t  *,const char *,size_t ,mbstate_t *));

wchar_t *wcscat __P(( wchar_t *, const wchar_t * ));
wchar_t *wcschr __P(( const wchar_t *, int ));
int      wcscmp __P(( const wchar_t*, const wchar_t* ));
wchar_t *wcscpy __P(( wchar_t*, const wchar_t* ));
size_t	wcscspn __P(( const wchar_t *, const wchar_t * ));
size_t   wcslen __P(( const wchar_t* ));
wchar_t *wcsncat __P(( wchar_t *, const wchar_t *, size_t ));
int		wcsncmp __P(( const wchar_t *, const wchar_t *, size_t ));
wchar_t *wcsncpy __P(( wchar_t *, const wchar_t *, size_t ));
wchar_t *wcspbrk __P(( const wchar_t *, const wchar_t * ));
wchar_t *wcsrchr __P(( const wchar_t *, int ));
size_t	wcsspn __P(( const wchar_t *, const wchar_t * ));
wchar_t *wcsstr __P(( const wchar_t *, const wchar_t * ));
wchar_t *wcstok __P(( wchar_t *, const wchar_t * ));
wchar_t *wcssep __P(( wchar_t **, const wchar_t * ));
int		wgetopt __P(( int, wchar_t **, const wchar_t * ));
wchar_t *wmemchr __P(( const wchar_t*, wchar_t, size_t ));
int		wmemcmp __P((const wchar_t *, const wchar_t *, size_t ));
wchar_t *wmemcpy __P(( wchar_t *, const wchar_t*, size_t ));
wchar_t *wmemmove __P(( wchar_t *, const wchar_t*, size_t ));
wchar_t *wmemset __P(( const wchar_t*, wchar_t, size_t ));

double wcstod __P(( const wchar_t *, wchar_t ** ));
long wcstol __P(( const wchar_t *, wchar_t ** , int));
unsigned long wcstoul __P(( const wchar_t *, wchar_t **, int));
quad_t wcstoq __P(( const wchar_t *, wchar_t ** , int));
u_quad_t wcstouq __P(( const wchar_t *, wchar_t ** , int));
int wcscoll __P((const wchar_t *, const wchar_t *));
size_t wcsftime __P(( wchar_t *, size_t , const wchar_t *, const struct tm *));
size_t wcsxfrm __P(( wchar_t *, const wchar_t *, size_t ));
wchar_t *wasctime __P((const struct tm*));
wchar_t *wctime __P((const time_t *));
wint_t btowc __P((int));
int wctob __P((wint_t ));
int mbsinit __P((const mbstate_t *));
size_t mbrlen __P((const char *, size_t , mbstate_t *));
size_t mbrtowc __P((wchar_t *, const char *, size_t , mbstate_t *));
size_t wcrtomb __P((char * , wchar_t, mbstate_t *));


__END_DECLS

#ifndef lint
#define	getwc_unlocked(fp)	__sgetwc(fp)
#define putwc_unlocked(x, fp)	__sputwc(x, fp)
#ifdef	_THREAD_SAFE
static __inline wint_t			\
__getwc_locked(FILE *_fp)		\
{					\
	extern int __isthreaded;	\
	wint_t _ret;			\
	if (__isthreaded)		\
		_FLOCKFILE(_fp);	\
	_ret = getwc_unlocked(_fp);	\
	if (__isthreaded)		\
		funlockfile(_fp);	\
	return (_ret);			\
}
static __inline wint_t			\
__putwc_locked(wint_t _x, FILE *_fp)	\
{					\
	extern int __isthreaded;	\
	wint_t _ret;			\
	if (__isthreaded)		\
		_FLOCKFILE(_fp);	\
	_ret = putwc_unlocked(_x, _fp);	\
	if (__isthreaded)		\
		funlockfile(_fp);	\
	return (_ret);			\
}
#define	getwc(fp)	__getwc_locked(fp)
#define	putwc(x, fp)	__putwc_locked(x, fp)
#else
#define	getwc(fp)	getwc_unlocked(fp)
#define putwc(x, fp)	putwc_unlocked(x, fp)
#endif
#endif /* lint */

#define	getwchar()		getwc(stdin)
#define	getwchar_unlocked()	getwc_unlocked(stdin)
#define	putwchar(x)		putwc(x, stdout)
#define	putwchar_unlocked(x)	putwc_unlocked(x, stdout)



#endif /* !_WCHAR_H_ */
