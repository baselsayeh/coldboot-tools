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

    rmdir.c
    
Abstract:

    Remove an EFI directory.


Revision History

--*/


#include <efi_interface.h>
#include <sys/param.h>
#include <unistd.h>
#include <wchar.h>
#include <dirent.h>
#define KERNEL
#include <errno.h>
#undef KERNEL

//
//  Name:
//      rmdir
//
//  Description:
//      Provide BSD rmdir interface for libc
//
//  Arguments:
//      File path of directory to remove
//
//  Returns:
//      0 if success, otherwise -1
//

int
rmdir( 
    const char *filename
    )
{
    DIR		*pDir;
    int		i;

    if ( !filename ) {
        errno = EINVAL;
        return -1;
    }

	/*
	 *  Make sure the directory exists.
	 */
	if ((pDir = opendir( filename)) == NULL) {
        /*
         *  errno set by opendir()
         */
        return -1;
	}

	/*
	 *  Make sure it is empy
	 */
	for ( i = 0; readdir( pDir ); i++) {
		/*
		 *  only allow '.' and '..'
		 */
		if ( i == 2 ) {
			closedir( pDir );
			errno = ENOTEMPTY;
			return -1;
		}
	}

	closedir( pDir );

	/*
	 *  Remove it.
	 *  errno set by unlink()
	 */
	return ( unlink( filename ) );
}
