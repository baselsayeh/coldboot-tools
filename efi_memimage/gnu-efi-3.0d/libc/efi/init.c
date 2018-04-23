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

    Initialization code for EFI libc
    Takes the place of crt0 (_start) in a Unix system


Revision History

--*/

#include <efi.h>
#include <efilibc.h>
#include <efi_interface.h>
#include <atk_guid.h>
#include <locale.h>

#include <libc_debug.h>
#include <assert.h>


//
//  anbother variable from crt0
//
char *__progname;

//
//  MSFT compiler uses this variable to inform itself whether floating
//  point support is loaded.  We know we have what we need.
//
int _fltused;

//
//  Unique vendor GUID for this implemenation
//
EFI_GUID	_LIBC_VendorGuid = ATK_VENDOR_GUID; 

//
//  Name:
//      InitializeLibC
//
//  Description:
//      Initialize libc data structures and EFI interfaces
//
//  Arguments:
//      Standard EFI ImageHandle and SystemTable for this app
//
//  Returns:
//      0 on success, otherwise -1
//
int
InitializeLibC( 
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    int				retval ;
	static BOOLEAN	initLibC = FALSE ;
	char *locale;	
	
#ifndef EFI_APP_MULTIMODAL
	LibcVersionCheck(ImageHandle, SystemTable);
#endif	

	if( initLibC == FALSE )
	{
		//
		// First, turn off the watchdog timer
		//
		SystemTable->BootServices->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );

#ifdef LIBC_DEBUG 
		//
		//  Do EFI library initialization so we can use Print()
		//
		InitializeLib( ImageHandle, SystemTable );
#endif
		//
		//  Set global pointer for application reference
		//
		_LIBC_EFIImageHandle = ImageHandle;
		_LIBC_EFISystemTable = SystemTable;

    	retval = (int)_InitializeEfiInterface( ImageHandle, SystemTable );
		if( retval )
		{
			return( -1 ) ;
		}

    	_InitializeMalloc();  // no return value

    	//
    	//  Initialize before Sytem IO so it can use getenv
    	//
    	_InitializeEnv( SystemTable );

    	retval = (int)_InitializeSystemIO();
		if( retval )
		{
			return( -1 ) ;
		}

    	locale = setlocale(LC_ALL,"");
    	if (locale == NULL) 
    	{
    		return( -1 );
    	}

    	__progname = "EFI Application";

		initLibC = TRUE ;
	}

	return( 0 ) ;
}

void
LibcVersionCheck(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
     )
{
      if (SystemTable->Hdr.Revision < EFI_SYSTEM_TABLE_REVISION){
          SystemTable->ConOut->OutputString(
		  	SystemTable->ConOut,
		  	L"Please start this image in EFI higher version \r\n"
		  	);
          SystemTable->BootServices->Exit(ImageHandle,EFI_UNSUPPORTED,0,NULL);
      	}
	return;     
}

