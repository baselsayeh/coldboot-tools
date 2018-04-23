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
      cwd

  Description:
      The set of routines that manipulate the current working directory


--*/

#include <efi_interface.h>
#ifndef KERNEL
#define KERNEL
#include <sys/errno.h>
#undef KERNEL
#else
#include <sys/errno.h>
#endif
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <assert.h>

static wchar_t	_cwd[ MAXPATHLEN ];

/*******************************
 *
 *  UNICODE routines
 */
int
wchdir( const wchar_t	*path )
{
	wchar_t	*tmp;

#ifndef EFI_NT_EMULATOR
  struct stat st;
	/*
	 *  Validate that the path is a valid directory
	 */
	if ( _wfaststat( path, &st ) < 0 ) {
		errno = ENOENT;
		return (-1);
	} else if ((st.st_mode & S_IFMT) != S_IFDIR ) {
		errno = ENOTDIR;
		return (-1);
	}
#endif
	if ( wcslen( path ) < MAXPATHLEN ) {
		//
		//  If the path includes the device, we assume it is a
		//  fully qualified path that we can use in total.
		//
		tmp = wcschr( path, L':' );
		if ( tmp ) {
			//
			//  It does so just do a complete copy
			//
			wcscpy( _cwd, path );
		} else {
			//
			//  Does not include device.  Check to see if this is a root
			//  relative path
			//
			if ( path[ 0 ] == L'/' || path[ 0 ] == L'\\' ) {
				//
				//  Root relative so copy right after device name already
				//  in _cwd
				//
				tmp = wcschr( _cwd, L':' );
				assert(tmp);
				tmp++;

				//
				//  Make sure it will fit
				//
				if ( (((size_t)(tmp - _cwd) / sizeof(wchar_t)) + wcslen( path )) >= MAXPATHLEN ) {
					goto TooLong;
				}
				wcscpy( tmp, path );
			} else {
				//
				//  Make sure it will fit
				//
				if ( (wcslen( _cwd ) + wcslen( path )) >= MAXPATHLEN ) {
					goto TooLong;
				}

				//
				//  Add/subtract slashes between path components if needed.
				//
				tmp = &_cwd[ wcslen(_cwd) - 1 ];

				if ( (*tmp    == L'/' || *tmp    == L'\\') &&
				     (path[0] == L'/' || path[0] == L'\\') ) {
					//
					//  Don't want two consecutive slashes - remove one
					//
					path++;
				} else if ( *tmp    != L'/' && *tmp    != L'\\' &&
						    path[0] != L'/' && path[0] != L'\\' ) {
					//
					//  Make sure there is a slash between the two joined parts
					//
					wcscat( _cwd, L"/" );
				}

				//
				//  Append the relative path to the current working directory
				//
				wcscat( _cwd, path );
			}
		}

		//
		//  Lastly, make path use consistent file separators.  This is just to
		//  make it look prettier when someone gets the CWD and prints it out.
		//
		for ( tmp = _cwd; *tmp; tmp++ ) {
			if ( *tmp == L'\\' )
				*tmp = L'/';
		}

		return (0);
	}

TooLong:
	errno = ENAMETOOLONG;
	return (-1);
}

wchar_t*
wgetcwd( wchar_t *buf, size_t size )
{
	if ( buf ) {
		if ( size == 0 ) {
			errno = EINVAL;
			return (NULL);
		}
		if ( size < ((wcslen( _cwd ) + 1) * sizeof(wchar_t)) ) {
			errno = ERANGE;
			return (NULL);
		}
	} else {
		buf = malloc( sizeof( _cwd ) );
		if ( buf == NULL ) {
			errno = ENOMEM;
			return (NULL);
		}
	}

	return ( wcscpy( buf, _cwd ) );
}

/*******************************
 *
 *  ASCII routines
 *
 */
int
chdir( const char *path )
{
	wchar_t	*tmp;
	int		ret = -1;

	tmp = calloc( strlen( path ) + 1, sizeof(wchar_t) );
	if ( tmp == NULL ) {
		errno = ENOMEM;
	} else if ( mbstowcs( tmp, path, strlen( path ) ) == -1 ) {
		errno = EILSEQ;
	} else {
		ret = wchdir( (const wchar_t*) tmp );
		free( tmp );
	}
	
	return ( ret );
}

char*
getcwd( char *buf, size_t size )
{
	if ( buf ) {
		if ( size == 0 ) {
			errno = EINVAL;
			return (NULL);
		}
		if ( size < (wcslen( _cwd ) + 1) ) {
			errno = ERANGE;
			return (NULL);
		}
	} else {
		buf = malloc( MAXPATHLEN );
		if ( buf == NULL ) {
			errno = ENOMEM;
			return (NULL);
		}
		size = MAXPATHLEN-1;
	}

	if ( wcstombs( buf, _cwd, size ) == -1 ) {
		errno = EILSEQ;
		free( buf );
		return (NULL);
	}

	return (buf);
}
