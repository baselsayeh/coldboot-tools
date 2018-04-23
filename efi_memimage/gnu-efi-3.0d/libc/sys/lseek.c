/*
 * Copyright (c) 1999, 2000
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
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Module Name:

    lseek.c
    
Abstract:

    Functions implementing the standard "lseek" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <fcntl.h>
#include <unistd.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>
#include <assert.h>

//
//  Name:
//      lseek
//
//  Description:
//      BSD lseek interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      position or offset to seek to
//      "whence" controls how to interpret the position arg- abs or relative
//
//  Returns:
//      returns resulting offset, or -1 on error
//

off_t
lseek( 
    int fd,
    off_t pos,
    int whence
    )
{
    EFILIBC_LSEEK_T Lseek        = NULL;
    EFI_STATUS      Status       = EFI_SUCCESS;
    VOID            *DevSpecific = NULL;

    DPRINT((L"lseek: fd %d\n", fd));

    if ( _GetOpenFileLseek( fd, &Lseek ) != 0 ) {
        pos = -1;
        goto Done;
    }
    assert( Lseek != NULL );

    if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
        pos = -1;
        goto Done;
    }

    Status = Lseek( (UINT64*)&pos, whence, DevSpecific );
    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"EFI error encountered in lseek: %d\n", Status ));
        errno = _EfiErrToErrno( Status );
        pos = -1;
        goto Done;
    }

    DPRINT((L"lseek: complete\n"));

Done:
    return pos;
}
