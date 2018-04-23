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

    dup2.c
    
Abstract:

    Functions implementing the standard "dup2" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
#include <stdlib.h>
#include "./filedesc.h"
#define KERNEL
#include <errno.h>
#undef KERNEL

#include <libc_debug.h>
#include <assert.h>

//
//  Name:
//      dup2
//
//  Description:
//      BSD dup2 interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      0 on success, -1 otherwise
//

int
dup2( int OldFd, int NewFd )
{
    EFI_STATUS      Status;
    UINT32			Junk;

    /*
     *  If old = new, nothing to do
     */
    if ( OldFd == NewFd )
    	return 0;

    /*
     *  Quick and dirty way to see if OldFd is valid.
     */
    if ( _GetOpenFileFlags( OldFd, &Junk ) != EFI_SUCCESS ) {
   		return -1;
    }

    /*
     *  Quick and dirty way to see if NewFd is valid and associated with an
     *  open file.  If it is, close it.
     */
    if ( _GetOpenFileFlags( NewFd, &Junk ) == EFI_SUCCESS ) {
    	if ( close( NewFd ) ) {
    		return -1;
    	}
    }

    Status = _DupFileDescriptor( OldFd, &NewFd );
	if ( EFI_ERROR( Status ) ) {
		DPRINT((L"EFI error encountered in dup2: %x\n", Status ));
		errno = _EfiErrToErrno( Status );
		return -1;
	}

    return 0;
}
