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

    _start_a.c
    
Abstract:

   	C run-time library startup code for applications using ASCII args to main()


Revision History

--*/

#include <efilibc.h>
#include <efi_interface.h>

#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "libc_debug.h"

//
//  External declarations
//
extern char	*__progname ;
extern int main( int, void ** );

//
//  Name:
//		_LIBC_Start_A
//
//  Description:
//	   	C run-time library startup code
//
//  Arguments:
//		ImgHandle:	Image handle	
//		SysTable:	System table	
//
//  Returns:
//		Nothing
//


EFI_STATUS
_LIBC_Start_A(
    IN EFI_HANDLE           ImgHandle,
    IN EFI_SYSTEM_TABLE     *SysTable
    )
{
	int			ReturnValue, 
				Argc,
				ArgIndex;
	size_t		Length = 0 ;
    CHAR16		**Argv ;
	char		*ArgBuf,
				**ArgPtr ;

	// Initialize C library
	InitializeLibC( ImgHandle, SysTable );

	_LIBC_GetArgsFromLoadOptions( &Argc, &Argv );

	if ( Argc && Argv ) {
		// Count number of Unicode characters including NULLs
		for( ArgIndex = 0 ; ArgIndex < Argc ; ArgIndex++ ) {
			Length += wcslen( Argv[ArgIndex] ) + 1 ;
		}

		// Allocate buffer for arguments
		ArgBuf = calloc( Length, sizeof( char ) ) ;
		if( !ArgBuf ) {
			DPRINT((L"crt0:  Memory Allocation Error (%d)\n", errno)) ;
			ReturnValue = errno ;
			goto Done ;
		}

		// Allocate array of pointers to arguments
		ArgPtr = calloc( Argc + 1, sizeof( char * ) ) ;
		if( !ArgPtr ) {
			DPRINT(("crt0:  Memory Allocation Error (%d)\n", errno)) ;
			ReturnValue = errno ;
			goto Done ;
		}

		// Convert each argument to ASCII
		for( ArgIndex = 0 ; ArgIndex < Argc ; ArgIndex++ ) {
			ArgPtr[ ArgIndex ] = ArgBuf ;
			wcstombs( ArgBuf, Argv[ ArgIndex ], wcslen( Argv[ ArgIndex ] ) ) ;
			ArgBuf += ( wcslen( Argv[ ArgIndex ] ) + 1 ) * sizeof( char ) ;
		}
        ArgPtr[ Argc ] = NULL;

		__progname = ArgPtr[0] ;

	} else {
		Argc = 0;
		ArgPtr = NULL;
	}

	// Run the application
	ReturnValue = main( Argc, (void**)ArgPtr ) ;

Done:
	//
	//  BugBug - If main() calls exit(), we depend on it to free the vector
	//  and arguments during malloc cleanup.
	//

	//
	//  Free our copy of load option arguments
	//
	_LIBC_FreeArgs( Argv );

	//
	//  Free what we allocated
	//
	if ( ArgPtr ) {
		if ( ArgPtr[0] )
			free( ArgPtr[0]);
		free(ArgPtr);
	}

	// Cleanup
	exit( ReturnValue ) ;
	return EFI_SUCCESS;
}

// Debug option:
//EFI_DRIVER_ENTRY_POINT(_LIBC_Start_A)
