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

    ResolveFilename.c
    
Abstract:

    Produce a fully qualified filename.


Revision History

--*/

#include <atk_libc.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>

#define JoinFilePaths( root, tail ) \
{ \
	wchar_t	LastChar = root[ wcslen(root) -1 ]; \
	if ( LastChar != L'\\' && LastChar != L'/' ) { \
		wcscat( root, L"\\" ); \
	} \
	wcscat( root,  tail ); \
}

/*
 *  Name:
 *      ResolveFilename
 *
 *  Description:
 *      Take a potentially partial filename produce a fully qualified filename
 *      using CWD and the PATH variable.
 *
 *  Arguments:
 *      Partial Unicode filename.
 *
 *  Returns:
 *      Unicode string if filename could be resolved.  The call is responsible
 *      for returning this memory with free().
 */
wchar_t*
ResolveFilename(
	IN	wchar_t	*FilePath
	)
{
	wchar_t		*FullPath;

	/*
	 *  If there is a device specification in the path, it is already
	 *  a fully qualified filename so just copy it.
	 */
	if ( wcschr( FilePath, L':' ) ) {
		FullPath = calloc( wcslen(FilePath) + 1, sizeof(wchar_t) );
		if ( FullPath )
			wcscpy( FullPath, FilePath );
	}

	/*
	 *  No device - see if there are any directories in the name.
	 *  NOTE:  We take advantage of the fact that wgetcwd returns memory
	 *         large enough to hold a string of MAXPATHLEN. 
	 */
	else if ( wcschr( FilePath, L'/' ) || wcschr( FilePath, L'\\') ) {

		FullPath = wgetcwd( NULL, 0 );
		if ( FullPath ) {

			/*
			 *  If it starts at the root, only add the device part of the CWD.
			 */
			if ( *FilePath == L'/' || *FilePath == L'\\' ) {
				wcscpy( wcschr(FullPath, L':') + 1, FilePath );
			}

			/*
			 *  It's a relative path so simpy append it to the CWD
			 */
			else {
				JoinFilePaths( FullPath, FilePath );
			}
		}
	}

	/*
	 *  This is a sole filename so search the path
	 */
	else {
		wchar_t	*Path;	// Unicode

		Path = malloc( MAXPATHLEN * sizeof(wchar_t) );
		if ( Path ) {
			FullPath = malloc( MAXPATHLEN * sizeof(wchar_t) );
			if ( FullPath ) {
				char	*path;	// Ascii
				wchar_t	*Next, *Current;

				/*
				 *  Get and convert the path we're searching.
				 */
				if ( !(path = getenv("PATH")) && !(path = getenv("path")) ) {

					/*
					 *  Use CWD if path is not set
					 */
					wgetcwd( Path, MAXPATHLEN * sizeof(wchar_t));

				} else {

					/*
					 *  Convert to Unicode
					 */
					mbstowcs( Path, path, MAXPATHLEN );
				}

				/*
				 *  Try each component of the path
				 */
				Next = Path;
				while ( (Current = wcssep( &Next, L";")) ) {

					/*
					 *  Unlike in FreeBSD, double, leading and trailing
					 *  colons (semicolons in our case) do not mean the
					 *  current directory.
					 */
					if (*Current) {
						struct stat dummy;

						/*
						 *  Build a full filename.
						 *  If the path is '.', substitute the CWD.
						 */
						if ( wcscmp( L".", Current ) ) {  // Not '.'
							wcscpy( FullPath, Current );
							JoinFilePaths( FullPath, FilePath );
						} else {		//Is '.'
							wgetcwd( FullPath, MAXPATHLEN * sizeof(wchar_t));

							JoinFilePaths( FullPath, FilePath );
							goto CHECKPOINT;
						}

                                                
						/*
						* If this path component doesn't include a device, prepend the
						*  device, and possibly path, using the current working directory.
						*/
  				              if ( wcschr( Current, L':' ) == 0 ) {
  				              
  		                            	wchar_t *CWD = wgetcwd( NULL, 0 );
  		                            	
  				                     if ( CWD ) {
  
  					                    /*
  					                     *  If it starts at the root, only add the device part of the CWD.
  					                     */
  					                    if ( *FullPath == L'/' || *FullPath == L'\\' ) {
  						                    wcscpy( wcschr(CWD, L':') + 1, FullPath );
  					                    }
  
  					                    /*
  					                     *  It's a relative path so simpy append it to the CWD
  					                     */
  					                    else {
  						                    JoinFilePaths( CWD, FullPath );
  					                    }
  		                                wcscpy( FullPath, CWD );
  		                                free( CWD );
  				                    }
  		                        }
              	CHECKPOINT:                 
				/*
				 *  See if it exists
				 */
				if ( wstat( FullPath, &dummy ) == 0 ) {

					break;
					}
				}
			}

				/*
				 *  If we failed to find a match, free and NULL FullPath 
				 */
			if ( Current == NULL ) {
				free( FullPath );
				FullPath = NULL;
				}
			}

			/*
			 *  We're done with Path
			 */
			free( Path );
		}
	}

	/*
	 *  If we have a path, convert all directory separators to EFI-normal form
	 */
	if ( FullPath ) {
		wchar_t	*tmp;

		for ( tmp = FullPath; *tmp; tmp++ ) {
			if ( *tmp == L'/' ) {
				*tmp = L'\\';
			}
		}
	}

	return ( FullPath );
}
