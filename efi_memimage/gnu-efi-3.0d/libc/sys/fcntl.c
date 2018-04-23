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

    fcntl.c
    
Abstract:

    Functions implementing the standard "fcntl" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <fcntl.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>

//
//  Name:
//      fcntl
//
//  Description:
//      Provide BSD fcntl interface for libc
//
//  Arguments:
//      fd:  File Descriptor (index into file descriptor table)
//      cmd: integer index denoting what to do
//
//  Returns:
//      0 if success, otherwise -1
//

int
fcntl( 
    int fd,
    int cmd,
    ...
    )
{
   	va_list	ap;
    int		flags = 0;

    DPRINT((L"fcntl: fd %d, cmd %d\n", fd, cmd ));

    switch ( cmd ) {
    case F_GETFL:
        if ( _GetOpenFileFlags( fd, (UINT32*)&flags ) != 0 ) {
            return -1;
        }
        return flags;

    case F_SETFL:
#if __STDC__
		va_start( ap, cmd );
#else
		va_start( ap );
#endif
		flags = va_arg( ap, int);
		va_end( ap );

        if ( _SetOpenFileFlags( fd, flags ) != 0 ) {
            return -1;
        }
        return flags;

    default:
        errno = EINVAL;
        return -1;
    }

    DPRINT((L"fcntl: return\n" ));

    return 0;
}
