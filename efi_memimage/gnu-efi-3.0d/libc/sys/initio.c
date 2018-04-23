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

    init.c
    
Abstract:

    Functions initializing "system-level" I/O


--*/


#include <efi_interface.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <wchar.h>
#include "./filedesc.h"
#include "./map.h"
#include "./init.h"

#include <libc_debug.h>
#include <assert.h>

//
//	Globals for application
//
EFI_HANDLE			_LIBC_EFIImageHandle = NULL ;
EFI_SYSTEM_TABLE	*_LIBC_EFISystemTable = NULL ;

EFI_GUID		ConsoleInGuid = SIMPLE_TEXT_INPUT_PROTOCOL ;
EFI_GUID		ConsoleOutGuid = SIMPLE_TEXT_OUTPUT_PROTOCOL ;
EFI_GUID		ConsoleErrGuid = SIMPLE_TEXT_OUTPUT_PROTOCOL ;

static EFI_DEVICE_PATH	*DevicePath = NULL ;

int
_InitializeSystemIO( 
    VOID
    )
{
    EFI_STATUS              Status;
    EFI_LOADED_IMAGE       *LoadedImage;
    CHAR16				   *Cwd;
    char                   *Dev;
    int						i;

    //
    //  initialize file descriptors
    //
    _InitializeFileDescriptorTable();

	//
	//  initialize protocol table
	//
    _InitializeMapProtocolTable();

	//
	//  initialize device table
	//
    _InitializeMapDeviceTable();

	//
	//  map stdin, stdout, stderr
	//
	_LIBC_MapProtocol( &ConsoleInGuid, _OpenConsole ) ;
	_LIBC_MapProtocol( &ConsoleOutGuid, _OpenConsole ) ;

	_LIBC_MapDevice( &ConsoleInGuid, NULL, "consolein:" ) ;
	_LIBC_MapDevice( &ConsoleOutGuid, NULL, "consoleout:" ) ;
	_LIBC_MapDevice( &ConsoleErrGuid, NULL, "consoleerr:" ) ;

#ifdef MAP_BLOCKIO_DEVICES
	_LIBC_MapProtocol( &_LibcBlockIoProtocol, _OpenBlock );
#endif

	//
	//  map default filesystem
	//
	_LIBC_MapProtocol( &_LibcFileSystemProtocol, _OpenFile ) ;

    LoadedImage = _GetLoadedImage();

	if ( LoadedImage->DeviceHandle ) {
		Status = _GetBootServices()->HandleProtocol ( LoadedImage->DeviceHandle,
													  &_LibcDevicePathProtocol,
													  (VOID*)&DevicePath );
	    if ( EFI_ERROR ( Status ) || DevicePath == NULL ) {
		    return -1;
		}

		_AddFileMapping( DFT_DEVICE, DevicePath );
	}

    //
    //  Merge in shell mappings (and the default mapping added above)
    //
    for ( i = 0; ;i++) {
    	char		*MapName;
    	void		*DevPath;
    	EFI_GUID	*Guid;

		if ( MapName = _GetFileMapping( i, &DevPath ) ) {
#ifdef MAP_BLOCKIO_DEVICES
			if (strncmp(MapName, "blk", 3) == 0) {
				Guid = &_LibcBlockIoProtocol;
			} else {
				Guid = &_LibcFileSystemProtocol;
			}
#else
			Guid = &_LibcFileSystemProtocol;
#endif
			Status = _LIBC_MapDevice( Guid, DevPath, MapName );
			if ( EFI_ERROR( Status ) ) {
				DPRINT((L"Error mapping %a device\n", MapName));
			} else {
				DPRINT((L"Device %a successfully mapped\n", MapName));
			}
		} else {
			break;
		}
    }

    //
    //  Get and set cwd of parent if known
    //
    Cwd = _GetParentCwd();
    if ( Cwd ) {
    	wchdir( Cwd );
    	free( Cwd );
	} else {
		chdir( DFT_DEVICE "/" );
	}

    //
    //  open stdin, stdout, stderr
    //
    if ( open( "consolein:", O_RDONLY ) != 0 ) {
        assert( 1 == 0 );
        return -1;
    }

    if ( open( "consoleout:", O_WRONLY ) != 1 ) {
        assert( 2 == 0 );
        return -1;
    }

    //
    //  Check the environment to see where they want stderr to go.  By default,
    //  we will map it to the standard console out device.
    //
    Dev = getenv( "USE_STDERR_DEV" );
    if ( Dev && !strcasecmp( Dev, "Y" ) ) {
    	if ( open( "consoleerr:", O_WRONLY ) != 2 ) {
       	 assert( 3 == 0 );
       	 return -1;
    	}
    } else {
    	if ( open( "consoleout:", O_WRONLY ) != 2 ) {
       	 assert( 3 == 0 );
       	 return -1;
    	}
    }

    return 0;
}
