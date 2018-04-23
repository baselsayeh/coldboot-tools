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

    stat.c
    
Abstract:

    Functions implementing the standard "stat" system call interfaces


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>
#include <assert.h>

//
//  Name:
//      fstat
//
//  Description:
//      BSD fstat interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      stat structure
//
//  Returns:
//      0 on success, -1 otherwise
//

int
fstat( 
    int fd, 
    struct stat *sb
    )
{
    EFILIBC_FSTAT_T Fstat        = NULL;
    EFI_STATUS      Status       = EFI_SUCCESS;
    VOID            *DevSpecific = NULL;
    int             ret          = 0;

    DPRINT((L"fstat: fd %d\n", fd));

    if ( _GetOpenFileFstat( fd, &Fstat ) != 0 ) {
        ret = -1;
        goto Done;
    }
    assert( Fstat );

    DPRINT((L"fstat: fcn ptr 0x%X\n", Fstat));

    if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
        ret = -1;
        goto Done;
    }

    Status = Fstat( sb, DevSpecific );
    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"EFI error encountered in fstat: %x\n", Status ));
        errno = _EfiErrToErrno( Status );
        ret = -1;
        goto Done;
    }

    DPRINT((L"fstat: return %d\n", 0));

Done:
    return ret;
}

//
//  Name:
//      stat
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
stat(
    const char *path, 
    struct stat *sb
    )
{
    int     fd = -1,
			ret = 0 ;

    DPRINT((L"stat: filename %a\n", path));

	if( ( fd = open( path, O_RDONLY ) ) != -1 ) {
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
lstat(
    const char *path,
    struct stat *sb
    )
{
    return stat( path, sb );
}
