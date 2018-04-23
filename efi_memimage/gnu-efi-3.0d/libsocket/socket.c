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

    socket.c
    
Abstract:

    Map FreeBSD socket call to EFI Socket Protocol Interface


Revision History

--*/

#include "./libsocket.h"
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

extern	int _SocketLibIsInitialized;

int
socket(
	int domain,
	int type,
	int protocol
	)
/*++

Routine Description:

    Return a libc compatible file desciptor for a socket representing
	an endpoint for the communiciations semantics specified.

Arguments:
    
    domain		- Communications domain of the socket.
	type		- Type of communications semantic for the socket.
	protocol	- Specific protocol to be used with the socket.

Returns:

	Libc compatible file desriptor on success
    -1 on error

--*/
{
	EFI_SOCKET_INTERFACE	*pIface;
	LibSockContext_t		*pContext;
	EFI_STATUS				Status;
	int						fd;

	//
	//  If we haven't been initialized yet, do it.  This initialization
	//  method will only work if the caller uses the libc crt0 entry point.
	//  Otherwise, they would have had to make an explicit socket init call.
	//

	if (!_SocketLibIsInitialized) {
		Status = EfiSocketInit (NULL, NULL);

		if (EFI_ERROR(Status)) {
			errno = _EfiErrToErrno(Status);
			return (-1);
		}
	}

	//
	//  Open the "socket" device
	//

	fd = open (SOCKET_DEVICE, O_RDWR);
	if ( fd < 0 )
		return (fd);

	//
	//  Get pointer to our context
	//

	if (_LIBC_GetOpenFileDevSpecific (fd, (void**)&pContext) < 0) {
		return (-1);	// errno set by call
	}

	//
	//  Extract interface pointer
	//

	pIface = pContext->pIface;

	//
	//  Create the socket
	//

	Status = pIface->Socket (pIface, &pContext->Socket, domain, type, protocol);
	if (EFI_ERROR(Status)) {

		//
		//  On error, close socket and map return code to errno
		//

		pIface->GetLastError (pIface, pContext->Socket, (UINT32*)&errno);
		close (fd);
		return (-1);
	}

	//
	//  All is well
	//

	return (fd);
}
