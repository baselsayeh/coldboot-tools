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

    unlink.c
    
Abstract:

    Functions implementing the standard "unlink" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <sys/param.h>
#include <unistd.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"

#include <libc_debug.h>
#include <assert.h>
#include <fcntl.h>

//
//  Name:
//      unlink
//
//  Description:
//      Provide BSD unlink interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      pointer to buffer
//      buffer size
//
//  Returns:
//      0 if success, otherwise -1
//

int
unlink( 
    const char *filename
    )
{
    EFI_STATUS      Status       = EFI_SUCCESS;
    int             ret          = 0;
	int				fd;

    if ( !filename ) {
        errno = EINVAL;
        return -1;
    }

	DPRINT((L"unlink: filename %a\n", filename));

    /*
     *  Must have open file to delete via EFI
     *
     */
    fd = open( filename, O_RDWR );
   	if ( fd == -1 ) {

		ret = -1;

	} else {

        /*
         *  Mark file for unlink - 
         */
        if ( _MarkFileForUnlink( fd ) ) {
	        DPRINT((L"unlink: Marking for unlinke returned error\n"));
	        ret = -1;
	            /* fall through to close */
        }

        /*
         *  Close it -
         *  If this was the only open FD for this file, this close will
         *  do the unlink.
         */
        if ( close( fd ) ) {
	        DPRINT((L"unlink: close returned error\n"));
	        ret = -1;
        }
	}

    return ret;
}
