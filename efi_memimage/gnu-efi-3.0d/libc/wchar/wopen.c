/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * 
 * Portions copyright (c) 1999, 2000
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
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// wide character version

#include <efi_interface.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <sys/param.h>
#ifndef KERNEL
#define KERNEL
#include <errno.h>
#undef KERNEL
#else
#include <errno.h>
#endif
#include "sys/filedesc.h"
#include "sys/map.h"


#include <libc_debug.h>
#include <assert.h>

int
wopen( 
    const wchar_t * path, 
    int flags, 
    ... )
/*++
Name:
    wopen

Description:
    Provides BSD-like wopen interface for libc

Arguments:
    path:   file name
    flags:  flags per fcntl.h or implementation-specific
    mode:   if flags contains O_CREAT then mode is flie permissions

Returns:
    file descriptor (index into the file descriptor table)
--*/
{
    int             fd				= -1;
    mode_t          mode			= 0;
	char			*DevName		= NULL ;
	char			*FilePath		= NULL ;
	char			*tmp			= NULL ;
	char			*cwd			= NULL ;
    EFI_STATUS      Status 			= EFI_SUCCESS;
	EFI_DEV_OPEN	OpenFunc		= NULL ;
	UINT32			LengthDevName	= 0 ;	
	UINT32			LengthFilePath	= 0 ;	
	char			_path[FILENAME_MAX];
	char			__path[FILENAME_MAX];

	DPRINT(( L"wopen: path %X, flags %X\n", path, flags ));

	wcstombs(_path, path, FILENAME_MAX);
	
    //
    //  Check params
    //
	// DPRINT(( L"open: check params\n" ));
    if (_path == NULL) {
      errno = EINVAL;
	  goto done ;
    }

    //
    //  Process arguments
    //
    if (flags & O_CREAT) {
      va_list arg;
      va_start( arg, flags );
      mode = va_arg( arg, mode_t );
      va_end( arg );
    }

	//
	//  Split path into device and file name
	//

/*
	if( ( tmp = strchr( _path, ':' ) ) != NULL ) {
			LengthDevName = ( UINT32 ) ( tmp - _path + 1 ) ;
			tmp++ ;
	}
	else {
			tmp = ( char * ) _path ;
			LengthDevName = 0 ;
	}
*/

reparse:
	if( ( tmp = strchr( _path, ':' ) ) != NULL ) {
		LengthDevName = ( UINT32 ) ( tmp - _path + 1 ) ;
		tmp++ ;
	}
	else {
		//
		//  No device name.  Assume this is a file device and do
		//  current working directory processing
		//
		cwd = getcwd( NULL, 0 );
		if (cwd == NULL) {
			errno = ENOMEM ;
			goto done ;
		}

		//
		//  If this is not a root relative path, make sure that it is.
		//
		if ( _path[ 0 ] != '/' &&
		     _path[ 0 ] != '\\' ) {

			 //
			 //  Path is not root relative.  Append a slash if the CWD
			 //  does not end with one.
			 //
			 if ( cwd[ strlen( cwd ) - 1 ] != '/'  &&
			      cwd[ strlen( cwd ) - 1 ] != '\\' ) {
				strcat( cwd, "/") ;
			}
		}

		//
		//  Root relative so only need to prepend CWD to device name.
		//
		else {
			tmp = strchr( cwd, ':' );
			assert( tmp != NULL );

			*(++tmp) = 0;
		}

		//
		//  We "know" that getcwd() returns a buffer of MAXPATHLEN
		//  size so we should have room to append the path to it.
		//  After checking that we won't creat a string larger than
		//  MAXPATHLEN, append specified path to CWD.
		//
		if ( strlen( cwd ) + strlen( _path ) >= MAXPATHLEN ) {
			errno = E2BIG ;
			goto done ;
		}
		strcat( cwd, _path ) ;
//		_path = cwd ;
		strcpy(__path, cwd);
		strcpy(_path, __path);

		//
		//  We should have a fully qualified pathname - reparse it.
		//
		goto reparse ;
	}

	LengthFilePath = (UINT32)strlen( _path ) - LengthDevName ;

	if( LengthDevName ) {
		DevName = calloc( LengthDevName + 1, sizeof( char ) ) ;
		if( !DevName ) {
            errno = ENOMEM;
			goto done ;
		}
		strncpy( DevName, _path, LengthDevName ) ;
	}

	if( LengthFilePath ) {
		FilePath = calloc( LengthFilePath + 1, sizeof( char ) ) ;
		if( !FilePath ) {
            errno = ENOMEM;
			goto done ;
		}
		strncpy( FilePath, tmp, LengthFilePath ) ;
	}

/*
	//
	// If no DevName then assume default
	//
	if( !DevName ) {
		DevName = calloc( strlen( DFT_DEVICE ) + 1, sizeof( char ) ) ;
		if( !DevName ) {
            errno = ENOMEM;
			goto done ;
		}
		strcpy( DevName, DFT_DEVICE ) ;
	}
*/
	//
	// Find and call open function for the specified device name
	//
	DPRINT(( L"open: FilePath %a DevName %a\n", FilePath, DevName ));
	Status = _CallDevNameOpenFunc( FilePath, DevName, flags, mode, &fd ) ;
	if ( EFI_ERROR( Status ) ) {
		errno = _EfiErrToErrno(Status);
	} else if ( _AddFileReference( fd ) < 0 ) {
		DPRINT(( L"open: Error adding reference for fd %d\n", fd));
	}
	
	done:

	if( DevName ) {
		free( DevName ) ;
	}
	if( FilePath ) {
		free( FilePath ) ;
	}
	if ( cwd ) {
		free( cwd ) ;
	}
    DPRINT(( L"open: return\n" ));
    
    return fd;
}
