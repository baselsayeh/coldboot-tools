/*
 * Copyright (c) 1999, 2000
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
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Module Name:

    recv.c
    
Abstract:

    Map FreeBSD recv call to EFI Socket Protocol Interface


Revision History

--*/

#include "./libsocket.h"
#include <sys/types.h>
#include <sys/socket.h>

ssize_t
recv(
	int		s,
	void	*buf,
	size_t	len,
	int		flags
	)
/*++

Routine Description:

    Receive data from an endpoint of the socket

Arguments:
    
    s		- File desciptor for the socket.
	buf		- Pointer to receive buffer 
	len		- Length of receive buffer
	flags	- Receive flags

Returns:

	Number of bytes received or
    -1 on error

--*/
{
	EFI_SOCKET_INTERFACE	*pIface;
	LibSockContext_t		*pContext;
	EFI_STATUS				Status;
	UINT32					BytesReceived;

	//
	//  Do sanity check on arguments
	//

	if (buf == NULL || (ssize_t)len < 0) {
		errno = EINVAL;
		return (-1);
	}

	//
	//  Get pointer to our context
	//

	if (_LIBC_GetOpenFileDevSpecific (s, (void**)&pContext) < 0) {
		return (-1);	// errno set by call
	}

	//
	//  Extract interface pointer
	//

	pIface = pContext->pIface;

	//
	//  Make the interface call
	//

	Status = pIface->Receive (pIface,
							  pContext->Socket,
							  buf,
							  (int)len,
							  flags,
							  NULL,
							  0,
							  &BytesReceived
							  );
	if (EFI_ERROR(Status)) {

		//
		//  On error set errno
		//

		pIface->GetLastError (pIface, pContext->Socket, (UINT32*)&errno);
		return (-1);
	}

	//
	//  All is well
	//

	return ((ssize_t)BytesReceived);
}
