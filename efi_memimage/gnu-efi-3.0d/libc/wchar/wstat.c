/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * 
 * Portions copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// wide character version
// efi/lib/libc/wchar/wstat.c


#include <efi_interface.h>
#include <unistd.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "sys/filedesc.h"

#include <libc_debug.h>
#include <assert.h>


//
//  Name:
//      wstat
//
//  Description:
//      BSD stat interface for libc
//
//  Arguments:
//      path:  name of file to get info about
//      sb:    stat structure
//
//  Returns:
//      0 on success, -1 otherwise
//
int
wstat(
    const wchar_t *path, 
    struct stat *sb
    )
{
    int     fd = -1,
			ret = 0 ;

    DPRINT((L"wstat: filename %s\n", path));

	if( ( fd = wopen( path, O_RDONLY ) ) != -1 ) {
		ret = fstat( fd, sb );
		close( fd ) ;
	} else {
		ret = -1 ;	
	}

	return( ret ) ;
}


//
//  Name:
//      lstat
//
//  Description:
//      BSD lstat interface for libc
//
//  Arguments:
//      path:  name of file to get info about
//      sb:    stat structure
//
//  Returns:
//      0 on success, -1 otherwise
//
int
wlstat(
    const wchar_t *path,
    struct stat *sb
    )
{
    return wstat( path, sb );
}
