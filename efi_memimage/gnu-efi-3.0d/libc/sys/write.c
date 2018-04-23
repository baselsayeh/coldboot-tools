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

    write.c
    
Abstract:

    Functions implementing the standard "write" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
#include <fcntl.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>
#include <assert.h>

//
//  Name:
//      write
//
//  Description:
//      BSD write interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      buffer
//      buffer size
//
//  Returns:
//      number of bytes written
//

ssize_t
write( 
    int fd, 
    const void *buffer, 
    size_t bufsize
    )
{
    EFILIBC_WRITE_T Write;
    EFI_STATUS      Status;
    UINTN           byteswritten;
    VOID            *DevSpecific;
    UINT32          flags;

    DPRINT((L"write: fd %d, bufsize %d\n", fd, bufsize));
    DHEXDMP(buffer, bufsize, L"write buffer");

	// assume the worst
	byteswritten = -1;

	if ( _GetOpenFileFlags( fd, &flags ) != 0 ) {
        goto Done;
	} else if ( (flags & O_ACCMODE) == O_RDONLY ) {
		errno = EBADF;	// seems odd but this is what FreeBSD returns
		goto Done;
	}

    if ( _GetOpenFileWrite( fd, &Write ) != 0 ) {
        goto Done;
    }

    assert( Write );

    if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
        goto Done;
    }
    byteswritten = bufsize;

    Status = Write( (VOID *)buffer, &byteswritten, DevSpecific );
    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"EFI error encountered in write: %x\n", Status ));
        errno = _EfiErrToErrno( Status );
        byteswritten = -1;
    }

Done:
    return (ssize_t)byteswritten;
}
