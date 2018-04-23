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

    rename.c
    
Abstract:

    Functions implementing the standard "rename" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
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
//      rename
//
//  Description:
//      BSD close interface for libc
//
//  Arguments:
//      From: path of existing name  To: path of new name
//
//  Returns:
//      0 on success, -1 otherwise
//

int
rename (const char *from, const char *to)
{
    EFI_STATUS      Status       = EFI_SUCCESS;
    int             ret          = 0;
	char			*s;
	int				retErrno	 = 0;
	int				fd			 = -1;
    wchar_t         *wcspath  	 = NULL;
    VOID            *DevSpecific = NULL;
	struct 			stat st;

	/*
	 *	Sanity check args 
	 */

	if (( from == NULL ) || (to == NULL) || (strlen (from) == 0) || (strlen (to) == 0) )
	{
   		DPRINT((L"rename: failed - null or zero length path \n"));
		retErrno = EINVAL;
		ret = -1;
		goto Done;
	}

	if ( (strlen (from) > MAXPATHLEN) || (strlen (to) > MAXPATHLEN) )
	{
   		DPRINT((L"rename: failed - path too long \n"));
		retErrno = ENAMETOOLONG;
		ret = -1;
		goto Done;
	}

	/*
	 *	Can't rename special dirs
	 */

	if ( (strcmp (to, ".") == 0) || (strcmp (to, "..") == 0) || (strcmp (from, ".") == 0)	||
		(strcmp (from, "..") == 0) )	
	{
   		DPRINT((L"rename: failed - '.' and '..' illegal \n"));
		retErrno = EINVAL;
		ret = -1;
		goto Done;
	}


	/*
	 *	EFI does not allow 'to' to have a device in it's path
	 */

    for (s = (char *)to; *s; s += 1) 
    {
        if (*s == ':') 
        {
	   		DPRINT((L"rename: device not allowed in new name \n"));
			retErrno = EXDEV;
			ret = -1;
            goto Done;
        }
    }

	/*
	 *	Fail if 'from' does not exist or 'to' already exists
	 */

	fd = open (from, O_RDWR);
	if (fd == -1)
	{
   		DPRINT((L"rename: error opening source \n"));
		retErrno = ENOENT;
		ret = -1;
		goto Done;
	}

	if (stat (to, &st) == 0)
	{
   		DPRINT((L"rename: new name already exists \n"));
		retErrno = EACCES;
		ret = -1;
		goto Done;
	}

	if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
		retErrno = ENOENT;
		ret = -1;
		goto Done;
	}

	wcspath = _ConvertToEfiPathname( to );
	if ( !wcspath ) {
		retErrno = ENOMEM;
		goto Done;
	}


    Status = EFI_RenameFile( DevSpecific, wcspath );
	if ( EFI_ERROR( Status ) ) 
	{
   		DPRINT((L"rename: EFI_RenameFile returned 0x%x\n",Status));
		retErrno = _EfiErrToErrno( Status );
		ret = -1;
		/* fall through and clean up */
	}

Done:

	if (fd != -1) close ( fd );
	if (wcspath) free( wcspath );

	errno=retErrno;

    return ret;
}

