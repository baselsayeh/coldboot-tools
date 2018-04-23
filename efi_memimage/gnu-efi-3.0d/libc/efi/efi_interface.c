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

    efi_interface.c
    
Abstract:

    Primary interfaces to EFI for libc


Revision History

--*/

#include <efi.h>
#include <efi_interface.h>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libc_debug.h>
#include "libc_private.h"
#include "sys/filedesc.h"

//
//  Structure to store interfaces to EFI core
//
typedef struct {
    EFI_HANDLE        ImageHandle;
    EFI_SYSTEM_TABLE *SystemTable;
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_MEMORY_TYPE   PoolAllocationType;
    UINT32            Pid;                /* fake BSD process id */
} LIBC_APP_INFO;

static LIBC_APP_INFO AppInfo;

VOID * _ImageBase;
UINT64 _ImageSize;

//
//  GUID structure initialization
//
EFI_GUID _LibcLoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
EFI_GUID _LibcFileSystemProtocol  = SIMPLE_FILE_SYSTEM_PROTOCOL;
EFI_GUID _LibcDevicePathProtocol  = DEVICE_PATH_PROTOCOL;
EFI_GUID _LibcFileInfoId          = EFI_FILE_INFO_ID;
EFI_GUID _LibcFileSystemInfoId    = EFI_FILE_SYSTEM_INFO_ID;
#ifdef MAP_BLOCKIO_DEVICES
EFI_GUID _LibcBlockIoProtocol     = BLOCK_IO_PROTOCOL;
#endif

//
//  Local prototypes
//
static CHAR16* _GetCwdFromLoadOptions( VOID *Dummy );

//
//  Current Working Directory functions
//
GetCwd_t	_GetCwd  = _GetCwdFromLoadOptions;
FreeCwd_t	_FreeCwd = free;

//
//  Name:
//      InitializeEfiInterface
//
//  Description:
//      Initialize data structures for EFI interfaces
//
//  Arguments:
//      Standard EFI ImageHandle and SystemTable for this app
//
//  Returns:
//      --none--
//
UINTN
_InitializeEfiInterface( 
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS         Status = EFI_SUCCESS;
    EFI_BOOT_SERVICES *BS     = NULL;
    struct timeval     Tv;

    AppInfo.ImageHandle = ImageHandle;
    AppInfo.SystemTable = SystemTable;
    BS                  = AppInfo.SystemTable->BootServices;

    //
    // Initialize pool allocation type
    //

    if (ImageHandle) {
        Status = BS->HandleProtocol (
                        ImageHandle,
                        &_LibcLoadedImageProtocol,
                        (VOID*)&AppInfo.LoadedImage
                        );

        if (!EFI_ERROR(Status)) {
            AppInfo.PoolAllocationType = AppInfo.LoadedImage->ImageDataType;
        } else {
            return -1;
        }
    }

    _ImageBase = AppInfo.LoadedImage->ImageBase;
    _ImageSize = AppInfo.LoadedImage->ImageSize;

    //
    //  Initialize fake BSD process id.  Although this is not strictly
    //  an EFI-ism, it is needed to make EFI look like BSD for the lib
    //
    gettimeofday( &Tv, 0 );
   	srand(Tv.tv_sec);
    AppInfo.Pid = rand() & 0xfff;

    return 0;
}


//
//  Name:
//      GetXXX functions
//
//  Description:
//      Return a pointer to EFI interface specified by XXX in name
//
//  Arguments:
//      --none--
//
//  Returns:
//      pointer to the specified interface
//

EFI_BOOT_SERVICES *
_GetBootServices( 
    VOID 
    )
{
    return AppInfo.SystemTable->BootServices;
}

EFI_RUNTIME_SERVICES *
_GetRuntimeServices( 
    VOID 
    )
{
    return AppInfo.SystemTable->RuntimeServices;
}

EFI_HANDLE
_GetImageHandle( 
    VOID 
    )
{
    return AppInfo.ImageHandle;
}

EFI_LOADED_IMAGE *
_GetLoadedImage( 
    VOID 
    )
{
    return AppInfo.LoadedImage;
}

EFI_MEMORY_TYPE *
_GetPoolAllocationType( 
    VOID 
    )
{
    return &AppInfo.PoolAllocationType;   
}

SIMPLE_INPUT_INTERFACE *
_GetConsoleIn( 
    VOID 
    )
{
    return AppInfo.SystemTable->ConIn;   
}

SIMPLE_TEXT_OUTPUT_INTERFACE *
_GetConsoleOut( 
    VOID 
    )
{
    return AppInfo.SystemTable->ConOut;   
}

SIMPLE_TEXT_OUTPUT_INTERFACE *
_GetConsoleErr( 
    VOID 
    )
{
    return AppInfo.SystemTable->StdErr;   
}

UINT32
_GetPid(
    VOID
    )
{
    return AppInfo.Pid;
}

CHAR16*
_GetParentCwd( VOID )
{
	CHAR16	*CurrentDir;
	CHAR16	*pCwd = NULL;

	CurrentDir = _GetCwd( NULL );
	if (CurrentDir) {
		pCwd = calloc(wcslen(CurrentDir) + 1, sizeof(CHAR16));
		if (pCwd) {
			/*
			 *  Make our copy then free the returned string
			 */
			wcscpy(pCwd, CurrentDir);
		}
   		_FreeCwd(CurrentDir);
	}

	return (pCwd);
}


#define	WHITESPACE( p ) (InQuote == FALSE && (*p == L' ' || *p == L'\t'))

VOID
_LIBC_GetArgsFromString(
    OUT int     *Argc,
    OUT CHAR16  ***Argv,
    IN  CHAR16  *pCommandLine
    )
{
	size_t	Length;
	CHAR16	*pArgs, *pEnd, **pArgv;
	BOOLEAN	InQuote = FALSE;

    //
    // Find first non-whitespace character then get command line length.
    //
    while ( WHITESPACE( pCommandLine ) ) {
        pCommandLine++;
    }

    if ( Length = wcslen( pCommandLine ) ) {
        Length += 1;    // add one for termination

        //
        // Allocate buffer for a copy of command line args
        //
        pArgs = calloc( Length, sizeof( CHAR16 ) );
        if( pArgs ) {
            //
            // Allocate array of pointers to arguments.  This is generally
            // going to be way more than we need but it's easier than
            // walking the string twice.
            //
            *Argv = pArgv = calloc( Length, sizeof( CHAR16* ) );
            if( pArgv ) {
                //
                // Copy the command line from the LoadOptions buffer.
                // (We can't alter the LoadOptions data.)
                //
                wcscpy( pArgs, pCommandLine );

                //
                // Now walk the string counting args and NULL
                // terminating them.
                //
                // NOTE: pEnd is just a saftey.  It should should
                //       never come into play.
                //
                pEnd = pArgs + Length;
                while ( *pArgs ) {
                	//
                	// See if we're starting a quoted argument
                	//
                	if ( *pArgs == L'"' ) {
                		*pArgs++ = 0;
                		InQuote = TRUE;
                	}

                    //
                    // Update vector and increment argument count.
                    //
                    *pArgv++ = pArgs;
                    (*Argc)++;

                    //
                    // Get to end of argument
                    //
                    while ( ! WHITESPACE( pArgs ) && *pArgs ) {
                        if ( *pArgs == L'"' ) {
                        	InQuote = FALSE;
                        	break;
                        }
                        pArgs++;
                    }

                    //
                    // If we're not at the end of the command line,
                    // terminate the argument and find the start of
                    // the next one.
                    //
                    if ( *pArgs ) {
                        //
                        // Terminate the argument and find the next one.
                        //
                        *pArgs++ = 0;
                        while ( WHITESPACE ( pArgs ) )
                            pArgs++;
                    }
                }

                // Null terminate the vector
                *pArgv = NULL;

            } else {
                *Argv = NULL;
                free ( pArgs );
            }
        }
    }
}

VOID
_LIBC_GetArgsFromLoadOptions(
	int		*Argc,
	CHAR16	***Argv
	)
{
	CHAR16	*pCommandLine = _GetLoadedImage()->LoadOptions;

	//
	// Set error defaults
	//
	*Argc = 0;
	*Argv = NULL;

	//
	// Process command line if present.
	//
	if ( _GetLoadedImage()->LoadOptionsSize && pCommandLine ) {

        _LIBC_GetArgsFromString( Argc, Argv, pCommandLine );

	}

	//
	//  BugBug - We depend on exit() to free the vector and arguments.
	//
}

VOID
_LIBC_FreeArgs(
    CHAR16   **Argv
    )
{
	if (Argv) {
		if (Argv[ 0 ])
			free( Argv[ 0 ] );
		free( Argv );
	}
}

static CHAR16*
_GetCwdFromLoadOptions(
	IN	VOID *Dummy
	)
{
	CHAR16	*Options = _GetLoadedImage()->LoadOptions;
	INT32	OptionsLength = (INT32)_GetLoadedImage()->LoadOptionsSize;

	if ( Options ) {
		//
		//  Make sure we the length is a multipal of what we're pointing at.
		//
		OptionsLength &= ~(sizeof(CHAR16) - 1);

		//
		//  Get past command line arguments
		//
		while ( *Options && (OptionsLength > 0) ) {
			Options++;
			OptionsLength -= sizeof(CHAR16);
		}

		//
		//  Get past possible null
		//
		OptionsLength -= sizeof(CHAR16);
		Options++;

		//
		//  If we haven't hit the end of options, assume the CWD follows
		//  command line arguments.
		//
		if ( OptionsLength > 0 ) {
			CHAR16	*Cwd = calloc( wcslen(Options) + 1, sizeof(CHAR16) );
			if ( Cwd ) {
				wcscpy( Cwd, Options );
			}
			return ( Cwd );
		}
	}

	return ( NULL );
}

//
//  Name:
//      _ConvertToEfiPathname
//
//  Description:
//      Converts ASCII/Unix style pathname to EFI compatible form.
//
//  Arguments:
//      AsciiPath - ASCII/Unix style pathname
//
//  Returns:
//      Pointer to persistent storage of converted path.  Should be freed
//      using free().
//

wchar_t*
_ConvertToEfiPathname(
    const char    *AsciiPath
    )
{
    wchar_t	*wcsfilename;
    size_t	i;
    size_t	len;

    /*
     *  Capture length since we use it so much
     */
    len = strlen( AsciiPath );

    /*
     *  Allocate memory for UNICODE version and convert
     */
    wcsfilename = calloc( len + 1, sizeof(wchar_t) );

    if ( wcsfilename ) {
        mbstowcs( wcsfilename, AsciiPath, len + 1 );

        /*
         *  Convert '/' to '\'
         */
        for ( i = 0; i < len; ++i ) {
            if ( wcsfilename[ i ] == L'/' )
                wcsfilename[ i ] = L'\\';
        }
	}

	return ( wcsfilename );
}

//
//  Name:
//      _CommonEfiCleanup
//
//  Description:
//      Common EFI infrustructure cleanup
//
//  Arguments:
static VOID
_CommonEfiCleanup( VOID )
{
    /*
     *  Do file decriptor cleanup
     */
	_FileDescriptorTableShutdown();

    /*
     *  Do malloc cleanup
     */
    _DeInitializeMalloc();
}

//
//  Name:
//      _EfiExit
//
//  Description:
//      System level cleanup and exit to EFI.
//      
//
//  Arguments:
//      Status      Exit status
//      DataSize    Size of ExitData in bytes
//      ExitData    Optional exit data
//
//  Returns:
//      -- none --
//
VOID
_EfiExit(
    IN  EFI_STATUS  Status,
    IN  UINTN       DataSize,
    IN  CHAR16      *ExitData
    )
{
	/*
	 *  Do common EFI cleanup
	 */
	_CommonEfiCleanup();

    /*
     *  Exit to EFI
     */
    _GetBootServices()->Exit( _GetImageHandle(), Status, DataSize, ExitData );
}

//
//  Name:
//      _LIBC_EfiExit
//
//  Description:
//      Stdio cleanup and exit to EFI.  Like exit() but allows the caller
//      to pass full arguments to EFI parent.
//      
//
//  Arguments:
//      Status      Exit status
//      DataSize    Size of ExitData in bytes
//      ExitData    Optional exit data
//
//  Returns:
//      -- none --
//
VOID
_LIBC_EfiExit(
    IN  EFI_STATUS  Status,
    IN  UINTN       DataSize,
    IN  CHAR16      *ExitData
    )
{
    /*
     *  Do stdio cleanup
     */
    _CommonStdioShutdown();

	/*
	 *  Do system cleanup and exit
	 */
	_EfiExit( Status, DataSize, ExitData );
}

//
//  Name:
//      _LIBC_Cleanup
//
//  Description:
//      Do all libc oriented cleanup without calling EFI exit routine.
//      This is to support EFI protocol/driver cleanup on unload.
//      
//  Arguments:
//      Status      Exit status
//      DataSize    Size of ExitData in bytes
//      ExitData    Optional exit data
//
//  Returns:
//      -- none --
//
VOID
_LIBC_Cleanup( VOID )
{
    /*
     *  Do stdio cleanup
     */
    _CommonStdioShutdown();

	/*
	 *  Do EFI cleanup
	 */
	_CommonEfiCleanup();
}
