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

    efi_init.c

Abstract:

    This file contains the initialization code for the socket
    interface to the tcpipv4 EFI protocol.


Revision History

--*/

#include <efisocket.h>	// must be first
#include <atk_libc.h>
#ifdef PPP_SUPPORT
#include <atk_ppp.h>
#endif

#include <sys/param.h>	// needed to include system.h
#include <sys/systm.h>	// to resolve log()

//
//  Initialization entry points
//
extern VOID			InitSocketInterface (VOID);
extern VOID			mbinit (VOID*);
extern VOID			CalloutInit (VOID);
extern VOID			domaininit (VOID*);
extern VOID			ifinit (VOID*);
extern VOID			loopattach (VOID*);
extern EFI_STATUS	StartTimer (VOID);
extern EFI_STATUS	StopTimer (VOID);
extern VOID			CalibrateHz(VOID);
extern EFI_STATUS	SniAttach (VOID);

extern EFI_SOCKET_INTERFACE		EfiSocketInterface;

//
//  Important EFI call table pointers
//
EFI_SYSTEM_TABLE		*TcpIpST;
EFI_BOOT_SERVICES		*TcpIpBS;
EFI_RUNTIME_SERVICES	*TcpIpRT;

EFI_GUID	SocketGuid = EFI_SOCKET_PROTOCOL;

static	EFI_SYSTEM_TABLE	BackupSystemTable;
static	EFI_GUID			LoadedImageGuid = LOADED_IMAGE_PROTOCOL;

#ifdef PPP_SUPPORT
static	void 	*pIface;
static EFI_GUID				pppGuid = PPP_PROTOCOL_GUID;
#endif

// Unload prototype
static EFI_STATUS
TcpIpUnload  (
    IN EFI_HANDLE       ImageHandle
    );

EFI_STATUS
EFIAPI
EfiNetEntry(
	EFI_HANDLE			ImageHandle,
	EFI_SYSTEM_TABLE	*SystemTable
	)
/*++

Routine Description:

    Main entry point to the socket protocol interface for the
	tcpipv4 network protocol stack.

Arguments:
    
    ImageHandle		- Handle to our image
	SystemTable		- Pointer to EFI system table

Returns:

    EFI_STATUS

--*/
{
	EFI_STATUS	Status;
	EFI_HANDLE	Handle;
	UINTN		BufferSize = sizeof(Handle);
	EFI_LOADED_IMAGE	*Image;

	//
	//  Workaround loaded protocol bug
	//

	BackupSystemTable = *SystemTable;

	//
	//  Remember our heritage
	//

	TcpIpST = &BackupSystemTable;
	TcpIpBS = TcpIpST->BootServices;
	TcpIpRT = TcpIpST->RuntimeServices;

	//
	//  Initialize Libc
	//

	InitializeLibC (ImageHandle, TcpIpST);

	//
	//  Make sure we are not already loaded
	//

	Status = TcpIpBS->LocateHandle (ByProtocol, &SocketGuid, NULL, &BufferSize, &Handle);
	if (Status != EFI_NOT_FOUND) {
		log(0, "This network protocol already loaded\n");
		return (EFI_ALREADY_STARTED);
	}

	// 
    //  Get our image information 
    //

    Status = TcpIpBS->HandleProtocol (ImageHandle, &LoadedImageGuid, (VOID*)&Image) ;

    // 
    //  Install unload handler
    //

    if ( ! EFI_ERROR( Status ) ) {
	    Image->Unload = TcpIpUnload ;
	}

	//
	//  Calibrate timer frequency
	//

	CalibrateHz();
	
	//
	//  Initialize socket table
	//

	InitSocketInterface();

	//
	//  Initilize mbufs
	//

	mbinit (NULL);

	//
	//  Initialize callout table
	//

	CalloutInit ();

	//
	//  Initialize network protocol domain
	//

	domaininit (0);

	//
	//  Initialize interface layer.
	//

	//  ifinit (NULL);

	//
	//  Attach loopback PSEUDO device
	//

	loopattach (NULL);

	//
	//  Attach to EFI_SIMPLE_NETWORK protocols
	//

	Status = SniAttach ();

#ifdef PPP_SUPPORT
	//
	//  Attach PPP interface
	//
{
	extern void* pppattach();

	if (pIface = pppattach( NULL )) {
		Status = TcpIpBS->InstallProtocolInterface (
								&ImageHandle,
								&pppGuid,
								EFI_NATIVE_INTERFACE,
								pIface);
		if (EFI_ERROR (Status)) {
			log(0, "Error %p installing PPP protocol\n");
		}
	}
}
#endif


        ifinit (NULL);

	if (!EFI_ERROR (Status)) {

		//
		//  Start timer
		//

		Status = StartTimer ();

		if (!EFI_ERROR (Status)) {

			//
			//  Install our interface
			//

			Status = TcpIpBS->InstallProtocolInterface (
									&ImageHandle,
									&SocketGuid,
									EFI_NATIVE_INTERFACE,
									&EfiSocketInterface);
		}
	}

	if (!EFI_ERROR (Status))
		log(0, "Network protocol loaded and initialized\n");

	return (Status);
}

static EFI_STATUS
TcpIpUnload  (
    IN EFI_HANDLE       ImageHandle
    )
{
	EFI_STATUS	Status = EFI_SUCCESS;

	Status = StopTimer();

	_LIBC_Cleanup();

#ifdef PPP_SUPPORT
{
	if( pIface ) {
    	Status = TcpIpBS->UninstallProtocolInterface (ImageHandle, 
											&pppGuid,
											pIface
											);
	}
}
#endif

    Status = TcpIpBS->UninstallProtocolInterface (ImageHandle, 
				                   		&SocketGuid,
				                   		&EfiSocketInterface
                                        );

	return Status;
}



//
//  Allow NT emulation debug
//

EFI_DRIVER_ENTRY_POINT(EfiNetEntry);
