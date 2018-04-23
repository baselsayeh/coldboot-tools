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

    read.c
    
Abstract:

    Functions implementing the standard "read" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <assert.h>
#include <libc_debug.h>

//
//  Name:
//      read
//
//  Description:
//      Provide BSD read interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      pointer to buffer
//      buffer size
//
//  Returns:
//      number of bytes read
//

ssize_t
read( 
    int fd, 
    void *buffer,
    size_t bufsize
    )
{
    EFILIBC_READ_T     Read;
    EFI_STATUS         Status;
    UINTN              bytesread;
    VOID               *DevSpecific;
    int                flags;

    // assume the worst
    bytesread = -1;

    if ( !buffer ) {
        errno = EINVAL;
        goto Done;
    }

    DPRINT((L"read: fd %d, bufsize %d\n", fd, bufsize));

	if ( _GetOpenFileFlags( fd, &flags ) != 0 ) {
        goto Done;
	} else if ( (flags & O_ACCMODE) == O_WRONLY ) {
		errno = EBADF;	// seems odd but this is what FreeBSD returns
		goto Done;
	}

    if ( _GetOpenFileRead( fd, &Read ) != 0 ) {
        goto Done;
    }
    assert( Read != NULL );

    if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
        goto Done;
    }

    bytesread = bufsize;

    Status = Read( (VOID *)buffer, &bytesread, DevSpecific );
    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"EFI error encountered in read: %d\n", Status ));
        errno = _EfiErrToErrno( Status );
        bytesread = -1;
    }

    DPRINT((L"read: read %d bytes\n", bytesread));

Done:
    return (ssize_t)bytesread;
}
