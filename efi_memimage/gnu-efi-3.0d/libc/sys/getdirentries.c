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
#include <efi.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
#include <stddef.h>

#define	READ_SIZE	(sizeof(EFI_FILE_INFO)+1024)
int
getdirentries( int fd, char *buf, int nbytes, long *basep)
{
	ssize_t			RawDataReturned;
	EFI_FILE_INFO	*pEfiInfo;
	struct dirent	*pDirEnt;
	char			*mbstring;
	int				TotalBytes;

	//
	//  Set the current file offset.  For EFI, this can only be zero
	//  on directory files.
	//

	*basep = 0;

	//
	//  Get a buffer to read the directory into.
	//

	pEfiInfo = (EFI_FILE_INFO*)malloc (READ_SIZE);
	mbstring = (char*)malloc (sizeof(pDirEnt->d_name));

	//
	//  Loop through directory entries
	//

	TotalBytes = 0;
	nbytes -= sizeof( struct dirent ) ;	/* save space for dummy record */
	while (nbytes >= sizeof(struct dirent)) {	/* pretty conservative test */
		pDirEnt = (struct dirent*)buf;

		//
		//  Read an entry
		//

		if ((RawDataReturned = read (fd, pEfiInfo, READ_SIZE)) == -1) {
			pDirEnt->d_reclen = 0 ;
			goto err;
		} else if ( RawDataReturned == 0) {
			break;
		}

		//
		//  Convert the name
		//
		wcstombs (mbstring, pEfiInfo->FileName, sizeof(pDirEnt->d_name));

		//
		//  Convert the entry
		//
		pDirEnt->d_fileno = 1000;	/* anything but zero */
		pDirEnt->d_type   = pEfiInfo->Attribute & EFI_FILE_DIRECTORY ? DT_DIR : DT_REG;
		pDirEnt->d_namlen = (u_char)strlen (mbstring);
		pDirEnt->d_reclen = (u_short)roundup(offsetof(struct dirent, d_name) + pDirEnt->d_namlen + 1, sizeof(UINT64));
		strcpy(pDirEnt->d_name, mbstring);

		//
		//  Setup for next pass
		//
		buf += pDirEnt->d_reclen;
		nbytes -= pDirEnt->d_reclen;
		TotalBytes += pDirEnt->d_reclen;
	}

	//
	// Record length of last dummy record is zero
	//
	pDirEnt = (struct dirent*)buf;
	pDirEnt->d_reclen = 0 ;

	free (pEfiInfo);
	free (mbstring);
	return ( TotalBytes );

err:
	free (pEfiInfo);
	free (mbstring);
	return (-1);
}
