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

  Name:
      ioctl

  Description:
      BSD ioctl interface for libc

  Arguments:
      File Descriptor (index into file descriptor table)
      request
      request-specific data

  Returns:
      0 on success, -1 otherwise

--*/

#include <efi_interface.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>
#include <assert.h>

int
ioctl( int fd, unsigned long request, ... )
{
    EFILIBC_IOCTL_T Ioctl        = NULL;
    EFI_STATUS      Status       = EFI_SUCCESS;
    VOID            *DevSpecific = NULL;
    int             ret          = 0; /* assume the best */
    va_list			ap;

    DPRINT((L"ioctl: fd %d, request %d\n", fd, request));

    if ( _GetOpenFileIoctl( fd, &Ioctl ) != 0 ) {
        ret = -1;
        goto Done;
    }
    assert( Ioctl != NULL );

    if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
        ret = -1;
        goto Done;
    }

    /*
     *  get point to va_list then make device specific call
     */
#if __STDC__
    va_start( ap, request );
#else
    va_start( ap );
#endif
    Status = Ioctl( DevSpecific, request, ap );
    va_end( ap );

    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"EFI error encountered in ioctl: %r\n", Status ));
        errno = _EfiErrToErrno( Status );
        ret = -1;
    }

    DPRINT((L"ioctl: return\n"));

Done:
    return ret;
}
