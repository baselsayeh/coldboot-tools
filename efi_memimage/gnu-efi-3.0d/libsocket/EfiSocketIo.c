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

    EfiSocketIo.c
    
Abstract:

    Hook socket library for the EFI Socket Protocol Interface into
	the libc file I/O subsystem.


Revision History

--*/

#include "./libsocket.h"
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <sys/poll.h>


static EFI_STATUS
EfiReadSock(
    VOID   *Buffer,
    UINTN  *BufSize,
    VOID   *DevSpecific
    )
/*++

Routine Description:

    Map read(2) call to socket operation

Arguments:
    
    Buffer		- Pointer to input buffer
	BufSize		- On input, size of input buffer.  On output, number of bytes read.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext = DevSpecific;
	EFI_STATUS			Status;
	UINT32				BytesReceived;

	//
	//  Read data from socket
	//
	Status = pContext->pIface->Receive (pContext->pIface,	// This
										pContext->Socket,	// Socket
										Buffer,				// Buffer
										(UINT32)*BufSize,	// Buffer Size
										0,					// Flags
										NULL,				// SockAddr
										0,					// SockAddr Len
										&BytesReceived);	// Bytes received
	//
	//  Set errno on error.  Otherwise, set the number of bytes read.
	//
	if (EFI_ERROR(Status)) {
		pContext->pIface->GetLastError (pContext->pIface, pContext->Socket, (UINT32*)&errno);
		Status = EFI_ERRNO_ALREADY_SET;
	} else {
		*BufSize = BytesReceived;
	}

	return (Status);
}

static EFI_STATUS
EfiWriteSock(
    VOID   *Buffer,
    UINTN  *BufSize,
    VOID   *DevSpecific
    )
/*++

Routine Description:

    Map write(2) call to socket operation

Arguments:
    
    Buffer		- Pointer to input buffer
	BufSize		- On input, size of output buffer.  On output, number of bytes written.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext = DevSpecific;
	EFI_STATUS			Status;
	UINT32				BytesSent;

	//
	//  Write the data to the socket
	//
	Status = pContext->pIface->Send (pContext->pIface,	// This
									 pContext->Socket,	// Socket
									 Buffer,			// Buffer
									 (UINT32)*BufSize,	// Buffer Size
									 0,					// Flags
									 NULL,				// SockAddr
									 0,					// SockAddr Len
									 &BytesSent);		// Bytes Sent
	//
	//  Set errno on error.  Otherwise, set the number of bytes read.
	//
	if (EFI_ERROR(Status)) {
		pContext->pIface->GetLastError (pContext->pIface, pContext->Socket, (UINT32*)&errno);
		Status = EFI_ERRNO_ALREADY_SET;
	} else {
		*BufSize = BytesSent;
	}

	return (Status);
}

static EFI_STATUS
EfiCloseSock(
    VOID  *DevSpecific
    )
/*++

Routine Description:

    Map close(2) call to socket operation

Arguments:
    
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext = DevSpecific;
	EFI_STATUS			Status;

	//
	//  Close our socket descriptor
	//
	Status = pContext->pIface->CloseSocket (pContext->pIface, pContext->Socket);

	//
	//  If all went well, free our context.  Otherwise set errno.
	//
	if (EFI_ERROR(Status)) {
		pContext->pIface->GetLastError (pContext->pIface, pContext->Socket, (UINT32*)&errno);
		Status = EFI_ERRNO_ALREADY_SET;
	} else {
		free (pContext);
	}

	return (EFI_SUCCESS);
}

static EFI_STATUS
EfiSeekSock(
    IN UINT64  *Position,
    IN UINT32  Whence,
    IN VOID    *DevSpecific
    )
/*++

Routine Description:

    Map lseek(2) call to socket operation

Arguments:
    
    Position	- Offset into file
	Whence		- Relativity of Position.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI_UNSUPPORTED - This call is not supported on a socket.

--*/
{
	return (EFI_UNSUPPORTED);
}

static EFI_STATUS
EfiFstatSock(
    IN struct stat *StatBuf,
    IN VOID        *DevSpecific
    )
/*++

Routine Description:

    Map stat(2) call to socket operation

Arguments:
    
    StatBuf		- Pointer to stat buffer
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI_UNSUPPORTED - This call is not supported on a socket.

--*/
{

	return (EFI_UNSUPPORTED);
}

static EFI_STATUS
EfiIoctlSock(
    IN VOID    *DevSpecific,
    IN UINT32  Request,
	IN va_list pArgs
    )
/*++

Routine Description:

    Map ioctl(2) call to socket operation

Arguments:
    
	DevSpecific	- Pointer to device specific context.
    Request		- Ioctl request
	...			- Variable argument list.

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext = DevSpecific;
	VOID				*Argp;
	EFI_STATUS			Status;

	//
	//  Get a ioctl argument pointer
	//

	Argp = va_arg (pArgs, void*);

	//
	//  Make the call
	//

	Status = pContext->pIface->SocketIoctl (pContext->pIface,	// This
											pContext->Socket,	// Socket
											Request,			// Cmd
											Argp);				// Argp
	//
	//  Set errno on error.  Otherwise, set the number of bytes read.
	//

	if (EFI_ERROR(Status)) {
		pContext->pIface->GetLastError (pContext->pIface, pContext->Socket, (UINT32*)&errno);
		Status = EFI_ERRNO_ALREADY_SET;
	}

	return (Status);
}

static EFI_STATUS
EfiPollSock(
    IN UINT32  Mask,
    IN VOID    *DevSpecific
    )
/*++

Routine Description:

    Map poll call to socket operation

Arguments:
    
    Mask		- mask of events of interest
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext = DevSpecific;
	SOCKET_EVENTS		NewMask, EventResults;
	EFI_STATUS			Status;

	//
	//  Convert from BSD to EFI_SOCKET mask bits
	//

	NewMask = 0;

	if (Mask & POLLIN)					NewMask |= EFI_SOCKPOLL_RECV_ANY;
	if (Mask & POLLRDNORM)				NewMask |= EFI_SOCKPOLL_RECV_NORMAL;
	if (Mask & (POLLPRI | POLLRDBAND))	NewMask |= EFI_SOCKPOLL_RECV_SPECIAL;
	if (Mask & POLLOUT)					NewMask |= EFI_SOCKPOLL_SEND_ANY;
	if (Mask & POLLWRNORM)				NewMask |= EFI_SOCKPOLL_SEND_NORMAL;
	if (Mask & POLLWRBAND)				NewMask |= EFI_SOCKPOLL_SEND_SPECIAL;

	//
	//  If we don't support the request, say so.
	//

	if (NewMask == 0)
		return (EFI_UNSUPPORTED);

	//
	//  Make the poll call
	//

	Status = pContext->pIface->PollSocket (pContext->pIface,	// This
										   pContext->Socket,	// Socket
										   NewMask,				// EventMask
										   &EventResults		// EventResults
										   );
	//
	//  Check results
	//

	if (!EFI_ERROR(Status)) {
		if (EventResults == 0)
			Status = EFI_NOT_READY;
	}

	return (Status);
}

EFI_STATUS
EfiOpenSocket(
	char			*FilePath,
	char			*DevName,
	int				Flags,
	mode_t			Mode,
	EFI_DEVICE_PATH	*DevPath,
	EFI_GUID		*Guid,
	INT32			*fd
	)
/*++

Routine Description:

    Map open(2) call to socket operation

Arguments:
    
    Buffer		- Pointer to input buffer
	BufSize		- On input, size of output buffer.  On output, number of bytes written.
	DevSpecific	- Pointer to device specific context.

	FilePath	- Path of file to open
	DevName		- Device name to use for mapping
	Flags		- Flags
	Mode		- Mode
	DevPath		- Device path
	Guid		- Guid
	fd			- Pointer to file descriptor returned on success

Returns:

    EFI status code

--*/
{
	LibSockContext_t	*pContext;
	wchar_t				*wcspath;
    EFI_STATUS			Status;

	//
	//  Assume the worst
	//

	*fd = -1;

	//
	//  Make sure there is no misunderstanding
	//

	if (strcmp (DevName, SOCKET_DEVICE)) {
		return (EFI_INVALID_PARAMETER);
	}

	//
	//  Convert path to UNICODE
	//

	if ( FilePath) {
		wcspath = calloc (strlen(FilePath) + 1, sizeof(wchar_t));
		if (wcspath == NULL) {
			return (EFI_OUT_OF_RESOURCES);
		}
		mbstowcs (wcspath, FilePath, strlen(FilePath) + 1);
	} else {
		wcspath = L"";
	}


	//
	//  Allocate and initialize device specific context
	//

	pContext = (LibSockContext_t*)calloc (1, sizeof(LibSockContext_t));
	if (pContext == NULL) {
		free (wcspath);
		return (EFI_OUT_OF_RESOURCES);
	}

	pContext->pIface = GetSocketInterfacePtr();

	//
	//  Allocate an initialized file descriptor
	//

	Status = _LIBC_AllocateNewFileDescriptor( 
				wcspath,                 // FileName
				Flags,                   // Flags
				Mode,                    // Mode
				FALSE,                   // IsATTy
				pContext,		         // DevSpecific
				EfiReadSock,             // read
				EfiWriteSock,            // write
				EfiCloseSock,            // close
				EfiSeekSock,             // lseek
				EfiFstatSock,            // fstat
				EfiIoctlSock,            // ioctl stub
				EfiPollSock,             // poll stub
				fd                       // New FileDescriptor
				);

	if (EFI_ERROR(Status)) {
		free (pContext);
		free (wcspath);
		return (Status);
	}

	return( Status ) ;
}
