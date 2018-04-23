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

    EfiSocketInit.c
    
Abstract:

    Socket library initialization routines.


Revision History

--*/

#include "./libsocket.h"
#include <stdlib.h>

int	_SocketLibIsInitialized = FALSE;

static EFI_GUID	SocketGuid = EFI_SOCKET_PROTOCOL;
static EFI_GUID	AtkTcpIpV4 = ATK_VENDOR_GUID;

EFI_SOCKET_INTERFACE	*__pSocket;

static EFI_STATUS
FindNetworkProtocol(
	EFI_SOCKET_INTERFACE	*pIface
	)
/*++

Routine Description:

    Validate that the socket interface supports the required
	network protocols.

Arguments:
    
    pIface		- Pointer to EFI Socket Interface to check.

Returns:

	EFI_SUCCSS		// This interface is acceptable
    EFI_NOT_FOUND	// Interface does not support the needed protocols
	Other EFI status codes as needed

--*/
{
	EFI_SOCKET_PROTO	*pProtos;
	EFI_STATUS			Status;
	EFI_GUID			VendorGuid;
	UINTN				ProtoSize, i;
	UINT32				ProtosFound;

	//
	//  Check the vendor quid of the interface.
	//  If it's not ours, return
	//

	Status = pIface->GetVendorGuid (pIface, &VendorGuid);
	if (EFI_ERROR (Status))
		return (Status);

	if (memcmp (&VendorGuid, &AtkTcpIpV4, sizeof(EFI_GUID)))
		return (EFI_NOT_FOUND);

	//
	//  Request network protocol list.  Since we have found the vendor
	//  of the implementation we wanted, this probably isn't necessary but
	//  it allows the same vendor to supply more than one protocol stack.
	//

	ProtoSize = 10;
RetryProto:
	pProtos = (EFI_SOCKET_PROTO*)malloc (ProtoSize * sizeof(EFI_SOCKET_PROTO));
	if (pProtos == NULL) {
		return (EFI_OUT_OF_RESOURCES);
	}

	Status = pIface->GetProtocols (pIface, &ProtoSize, pProtos);

	//
	//  See if the buffer was too small
	//

	if (Status == EFI_BUFFER_TOO_SMALL) {
		free (pProtos);
		goto RetryProto;
	} else if (EFI_ERROR (Status)) {

		//
		//  There was an error - bail
		//

		free (pProtos);
		return (Status);
	}

	//
	//  Search for required protocols
	//

	ProtosFound = 0;
	for (i = 0; i < ProtoSize; i++) {

		//
		//  We're only interested in the internet domain
		//

		if (pProtos[ i ].Domain == EFI_INTERNET_DOMAIN) {
			switch (pProtos[ i ].Protocol) {
				case IP_PROTO_ID:
					ProtosFound |= IP_PROTO;
					break;
				case ICMP_PROTO_ID:
					ProtosFound |= ICMP_PROTO;
					break;
				case UDP_PROTO_ID:
					ProtosFound |= UDP_PROTO;
					break;
				case TCP_PROTO_ID:
					ProtosFound |= TCP_PROTO;
					break;
				case RAW_PROTO_ID:
					ProtosFound |= RAW_PROTO;
					break;
			}
		}
	}

	//
	//  Free resources and return status
	//

	free (pProtos);
	if (ProtosFound == NEEDED_PROTOS)
		return (EFI_SUCCESS);
	else
		return (EFI_NOT_FOUND);
}

static EFI_STATUS
FindSocketProtocol(
	EFI_BOOT_SERVICES		*BS,
	EFI_SOCKET_INTERFACE	**pSocket
	)
/*++

Routine Description:

    Find socket interface that supports the TCP/IP protocols needed

Arguments:
    
    BS		- Pointer Boot Services table.
	pSociet	- Out pointer to acceptable EFI Socket Interface

Returns:

	EFI_SUCCSS		// An acceptable interface was found 
	Other EFI status codes as needed

--*/
{
    EFI_HANDLE      *pHandle;
    UINTN           BufferSize;
    EFI_STATUS      Status;
	UINTN			HandleCount, i;

    //
    //  Locate all socket protocol interfaces
    //

    BufferSize = sizeof(EFI_HANDLE) * 5;
RetryLocate:
	pHandle = (EFI_HANDLE*)malloc (BufferSize);
	if (pHandle == NULL) {
		return (EFI_OUT_OF_RESOURCES);
	}

    Status = BS->LocateHandle (
						ByProtocol,
						&SocketGuid,
						NULL,
						&BufferSize,
						pHandle
						);
    //
    //  See if the buffer was big enough
    //

	if (Status == EFI_BUFFER_TOO_SMALL) {
		free (pHandle);
		goto RetryLocate;
    } else if (EFI_ERROR (Status)) {

		//
		//  There was an error - bail
		//

		free (pHandle);
		return (Status);
	}

	HandleCount = BufferSize / sizeof(EFI_HANDLE);
    for (i = 0; i < HandleCount; i++) {
		EFI_SOCKET_INTERFACE	*pIface;

		//
		//  Get the interface for this instance of the protocol
		//

		Status = BS->HandleProtocol (pHandle[ i ], &SocketGuid, (VOID*)&pIface);
		if (EFI_ERROR (Status)) {
			free (pHandle);
			return (Status);
		}

		Status = FindNetworkProtocol (pIface);

		//
		//  If there was a problem, try the next handle
		//

		if (EFI_ERROR (Status))
			continue;

		//
		//  We found a socket interface that supports the TCP/IP protocol
		//

		*pSocket = pIface;
		break;
	}

	free (pHandle);
	return (Status);
}

static EFI_STATUS
MapSocketDevice(
		EFI_SOCKET_INTERFACE	*pSocket
		)
/*++

Routine Description:

    Map the EFI Socket Protocol Interface in the libc I/O subsystem

Arguments:
    
	pSociet	- Pointer to EFI Socket Protocol Interface to map

Returns:

	EFI status code

--*/
{
	EFI_STATUS	Status;

	Status = _LIBC_MapProtocol (&SocketGuid, EfiOpenSocket);
	if (!EFI_ERROR(Status)) {
		Status = _LIBC_MapDevice (&SocketGuid, NULL, SOCKET_DEVICE);
	}

	return (Status);
}

int
EfiSocketInit(
	void	*voidImage,			// the void pointers just keeps sys/socket.h
	void	*voidSystemTable	// from containing any efi-isms.
	)
/*++

Routine Description:

    Initialize the EFI Socket Protocol Interface for use by the
	socket library.  This call can be made explicitly or implicitly.
	Implicit use assumes the caller has already called InitializeLibC.

Arguments:
    
	pImage			- Optional pointer to EFI image using this library
	pSystemTable	- Optional pointer EFI system table

Returns:

	0 on success, -1 on error

--*/
{
	EFI_BOOT_SERVICES		*BS;
	EFI_STATUS				Status;
	EFI_SOCKET_INTERFACE	*pSocket;
	EFI_HANDLE				*pImage = voidImage;
	EFI_SYSTEM_TABLE		*pSystemTable = voidSystemTable;

	//
	//  Make sure libc has been initialized.
	//

	if (_LIBC_EFISystemTable == NULL) {

		//
		//  We need to do initialization.  Make sure they passed
		//  valid pointers.
		//

		if (pImage == NULL || pSystemTable == NULL) {
			return (-1);
		} else  if (InitializeLibC (pImage, pSystemTable)) {
			return (-1);
		}
	}

	BS = _LIBC_EFISystemTable->BootServices; 

	//
	//  Find the appropriate socket protocol
	//

	Status = FindSocketProtocol (BS, &pSocket);

	if (!EFI_ERROR (Status)) {

		//
		//  Hook ourselfs into the file I/O subsystem
		//

		Status = MapSocketDevice (pSocket);

		//
		//  Set global interface pointer
		//
		__pSocket = pSocket;
	}

	//
	//  Check for error
	//
	_SocketLibIsInitialized = TRUE;
	return (Status ? -1 : 0);
}
