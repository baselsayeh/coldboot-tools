/*
 * Copyright (c) 1982, 1986, 1989, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Portions copyright (c) 1999, 2000
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
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    efi_interface.c

Abstract:

    This is a very hacked up version of FreeBSD uipc_syscalls.c
    It implements EFI Socket Protocol Interface.



Revision History

--*/

#include <efisocket.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/uio.h>
#include <sys/filio.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/route.h>
#include <atk_guid.h>
#include <sys/poll.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

extern int _MallocListLength;

#ifdef MALLOC_MEMORY_TRACE 
int _MallocTraceBack( char **Buf, size_t Size, int *Depth );
int _MallocTraceBackDiff( char **InBuf, int InCount, char **OutBuf, int OutCount );


#define	MALLOC_SNAP() \
do { \
	Count = _MallocTraceBack( InBuf, sizeof(InBuf), &Depth ); \
} while(0)

#define	MALLOC_DIFF() \
do { \
	int	new, i, x; \
	new = _MallocTraceBackDiff( InBuf, Count, OutBuf, sizeof(OutBuf) / Depth ); \
	for ( i = 0; i < new; i++ ) { \
		printf("%p: % 5d - ", OutBuf[ i*Depth ], OutBuf[(i*Depth)+1]); \
		for (x = 2; x < Depth; x++) { \
			printf("%p ", OutBuf[ (i*Depth)+x ]); \
		} \
		printf("\n"); \
	} \
} while(0)

int		Depth;
int		Count;
char	*InBuf[ 0x2000 ];
char	*OutBuf[ 0x2000 ];

#else

#define MALLOC_SNAP() printf("List = %d\n", _MallocListLength)
#define MALLOC_DIFF() printf("List = %d\n", _MallocListLength)

#endif

//
//  Macros
//

#define	HOLD_SOCKET_SLOT	((struct socket*)(-1))

#define GETSOCKET( index ) \
	((index) >= MAX_SOCKETS ? NULL : SocketTable[ (index) ])

#define VALIDATE_SOCKET(so) \
	do { \
		if (so == NULL || so == HOLD_SOCKET_SLOT) { \
			LastError = ENOTSOCK; \
			return (EFI_INVALID_PARAMETER); \
		} \
	} while (0)

#define ConvertEfiToBsdSockAddr( pEfiAddr, pBsdAddr, EfiAddrLen ) \
	do { \
		bcopy (&(pEfiAddr)->AddressFamily, &(pBsdAddr)->sa_family, (EfiAddrLen)); \
		(pBsdAddr)->sa_len = (UINT8)(EfiAddrLen) + 1; \
	} while (0)

#define ConvertBsdToEfiSockAddr( pBsdAddr, pEfiAddr, EfiAddrLen ) \
	do { \
		if ((EfiAddrLen) > ((UINT32)(pBsdAddr)->sa_len - 1)) \
			(EfiAddrLen) = (pBsdAddr)->sa_len - 1; \
		bcopy (&(pBsdAddr)->sa_family, &(pEfiAddr)->AddressFamily, (EfiAddrLen)); \
	} while (0)

#define ConverBsdToEfiOption(val)						\
	val == SO_DEBUG      ? EFI_SOCKOPT_DEBUG          : \
	val == SO_ACCEPTCONN ? EFI_SOCKOPT_ACCEPT_CONN    : \
	val == SO_REUSEADDR  ? EFI_SOCKOPT_REUSE_ADDR     : \
	val == SO_KEEPALIVE  ? EFI_SOCKOPT_KEEP_ALIVE     : \
	val == SO_DONTROUTE  ? EFI_SOCKOPT_DONT_ROUTE     : \
	val == SO_BROADCAST  ? EFI_SOCKOPT_BROADCAST      : \
	val == SO_LINGER     ? EFI_SOCKOPT_LINGER         : \
	val == SO_OOBINLINE  ? EFI_SOCKOPT_URGENT_INLINE  : \
	val == SO_SNDBUF     ? EFI_SOCKOPT_SEND_BUF_SIZE  : \
	val == SO_RCVBUF     ? EFI_SOCKOPT_RECV_BUF_SIZE  : \
	val == SO_SNDLOWAT   ? EFI_SOCKOPT_SEND_LOW_WATER : \
	val == SO_RCVLOWAT   ? EFI_SOCKOPT_RECV_LOW_WATER : \
	val == SO_SNDTIMEO   ? EFI_SOCKOPT_SEND_TIMEOUT   : \
	val == SO_RCVTIMEO   ? EFI_SOCKOPT_RECV_TIMEOUT   : \
	val == SO_ERROR      ? EFI_SOCKOPT_ERROR_STATUS   : \
	val == SO_TYPE       ? EFI_SOCKOPT_PROTOCOL_TYPE  : \
	val
		
#define ConverEfiToBsdOption(val)						\
	val == EFI_SOCKOPT_DEBUG          ? SO_DEBUG      : \
	val == EFI_SOCKOPT_ACCEPT_CONN    ? SO_ACCEPTCONN : \
	val == EFI_SOCKOPT_REUSE_ADDR     ? SO_REUSEADDR  : \
	val == EFI_SOCKOPT_KEEP_ALIVE     ? SO_KEEPALIVE  : \
	val == EFI_SOCKOPT_DONT_ROUTE     ? SO_DONTROUTE  : \
	val == EFI_SOCKOPT_BROADCAST      ? SO_BROADCAST  : \
	val == EFI_SOCKOPT_LINGER         ? SO_LINGER     : \
	val == EFI_SOCKOPT_URGENT_INLINE  ? SO_OOBINLINE  : \
	val == EFI_SOCKOPT_SEND_BUF_SIZE  ? SO_SNDBUF     : \
	val == EFI_SOCKOPT_RECV_BUF_SIZE  ? SO_RCVBUF     : \
	val == EFI_SOCKOPT_SEND_LOW_WATER ? SO_SNDLOWAT   : \
	val == EFI_SOCKOPT_RECV_LOW_WATER ? SO_RCVLOWAT   : \
	val == EFI_SOCKOPT_SEND_TIMEOUT   ? SO_SNDTIMEO   : \
	val == EFI_SOCKOPT_RECV_TIMEOUT   ? SO_RCVTIMEO   : \
	val == EFI_SOCKOPT_ERROR_STATUS   ? SO_ERROR      : \
	val == EFI_SOCKOPT_PROTOCOL_TYPE  ? SO_TYPE       : \
	val

#define ConverEfiToBsdShutdown(val)             \
	val == EFI_SOCK_SHUTDOWN_RD   ? SHUT_RD   : \
	val == EFI_SOCK_SHUTDOWN_WR   ? SHUT_WR   : \
	val == EFI_SOCK_SHUTDOWN_RDWR ? SHUT_RDWR : \
	val

#define	MAX_SOCKETS	64
static struct socket*	SocketTable[ MAX_SOCKETS ];

static EFI_GUID	AtkGuid = ATK_VENDOR_GUID;

static UINT32	LastError;	// lame errno

static EFI_STATUS
ConvertToEfiStatus(
	int error
	)
/*++

Routine Description:

    Convert FreeBSD errno to EFI_STATUS code

Arguments:
    
    error	FreeBSD error code

Returns:

    EFI_STATUS

--*/
{
	switch (error) {
		case 0:
			return (EFI_SUCCESS);

		case EACCES:
			return (EFI_WRITE_PROTECTED);

		case EADDRINUSE:
			return (EFI_SOCKERR_ADDRINUSE);

		case EADDRNOTAVAIL:
			return (EFI_SOCKERR_ADDRNOTAVAIL);

		case EAFNOSUPPORT:
			return (EFI_SOCKERR_AFNOSUPPORT);

		case EBUSY:
		case EINPROGRESS:
			return (EFI_SOCKERR_INPROGRESS);

		case ECONNABORTED:
			return (EFI_SOCKERR_CONNABORTED);

		case ECONNREFUSED:
			return (EFI_SOCKERR_CONNREFUSED);

		case ECONNRESET:
			return (EFI_SOCKERR_CONNRESET);

		case EHOSTUNREACH:
			return (EFI_SOCKERR_HOSTUNREACH);

		case EMSGSIZE:
			return (EFI_SOCKERR_MSGSIZE);

		case ENOTSOCK:
		case EINVAL:
		case EOPNOTSUPP:
			return (EFI_INVALID_PARAMETER);

		case EISCONN:
			return (EFI_SOCKERR_ISCONN);

		case ENETDOWN:
			return (EFI_SOCKERR_NETDOWN);

		case ENETUNREACH:
			return (EFI_SOCKERR_NETUNREACH);

		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
			return (EFI_OUT_OF_RESOURCES);

		case ENOENT:
			return (EFI_NOT_FOUND);

		case ENOTCONN:
			return (EFI_SOCKERR_NOTCONN);

		case EPERM:
			return (EFI_ACCESS_DENIED);

		case ETIMEDOUT:
			return (EFI_TIMEOUT);

		case EWOULDBLOCK:
			return (EFI_SOCKERR_WOULDBLOCK);

		case ESRCH:
		case ETOOMANYREFS:
		case EPROTONOSUPPORT:
		case EPROTOTYPE:
		case ERESTART:
		case EPIPE:
		case ENXIO:
		case ENOPROTOOPT:
		case ENAMETOOLONG:
		case EALREADY:
		case EDESTADDRREQ:
		case EDOM:
		case EEXIST:
		case EFAULT:
		case EHOSTDOWN:
		case EINTR:
			return (EFI_SOCKERR_FAILURE);
	}

	return (EFI_SOCKERR_FAILURE);
}

static EFI_STATUS EFIAPI
GetVendorGuid(
    IN  struct _EFI_SOCKET  *This,
    OUT EFI_GUID            *VendorGuid
    )
/*++

Routine Description:

    Return vendor GUID of network stack

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	bcopy (&AtkGuid, VendorGuid, sizeof(AtkGuid));
	return (EFI_SUCCESS);
}

static EFI_STATUS EFIAPI
GetProtocols(
    IN  struct _EFI_SOCKET  *This,
    IN OUT UINTN            *ArraySize,
    OUT EFI_SOCKET_PROTO    *ProtoArray
    )
/*++

Routine Description:

    Return list of supported protocols

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct domain	*dp;
	struct protosw	*pr;
	EFI_STATUS		Status;
	UINT32			Total;
	UINTN			Room;

	Status = EFI_SUCCESS;
	Room = (*ArraySize && ProtoArray);
	Total = 0;

	//
	//  Find all communications domains
	//

	for (dp = domains; dp; dp = dp->dom_next) {

		//
		//  Find all protocols within a domain
		//

		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++) {
			if (Room) {
				ProtoArray->Domain   = dp->dom_family;
				ProtoArray->Type     = pr->pr_type;
				ProtoArray->Protocol = pr->pr_protocol;
			}

			//
			//  Set up for next pass and check for overflow
			//
			if (Room && (++Total > *ArraySize)) {
				Room = FALSE;
				Status = EFI_BUFFER_TOO_SMALL;
			} else {
				ProtoArray++;
			}
		}
	}

	*ArraySize = Total;
	return (Status);
}

static EFI_STATUS EFIAPI
Socket(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              *Socket,
    IN  UINT32              Domain,
    IN  UINT32              Type,
    IN  UINT32              Protocol
	)
/*++

Routine Description:

    Creates an endpoint for communication and returns a descriptor.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	SOCKET		i;
	UINT32		error;
	EFI_STATUS	Status;
	int			s;

	//
	//  Find an empty socket slot.  We'll hold the slot until it is assigned
	//  a socket buffer in case the socreate() call blocks.  This is the
	//  reason we don't simple protect it with an spl().
	//

	s = splhigh();
	for (i = 0; i < MAX_SOCKETS; ++i)
		if (SocketTable[ i ] == NULL)
			break;

	if (i == MAX_SOCKETS) {
		splx (s);
		LastError = ENFILE;
		return (EFI_OUT_OF_RESOURCES);
	} else  {
		SocketTable[ i ] = HOLD_SOCKET_SLOT;
		splx (s);
	}

	error = socreate (Domain, &SocketTable[ i ], Type, Protocol, NULL);
	Status = ConvertToEfiStatus (error);

	if (error) {
		SocketTable[ i ] = NULL;
		LastError = error;
	} else {
		*Socket = i;
	}

	return (Status);
}

static EFI_STATUS EFIAPI
Bind(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  EFI_SOCKADDR	    *Addr,
    IN  UINT32              AddrLen
	)
/*++

Routine Description:

    Bind a name to a socket

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct socket	*so = GETSOCKET (Socket);
	struct sockaddr	BsdSockAddr;

	VALIDATE_SOCKET (so);

	ConvertEfiToBsdSockAddr( Addr, &BsdSockAddr, AddrLen );
	so->so_lastError = sobind (so, &BsdSockAddr, NULL);
	return (ConvertToEfiStatus (so->so_lastError));
}

static EFI_STATUS EFIAPI
Listen(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT32              Backlog
	)
/*++

Routine Description:

    Listen for connections on a socket

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct socket *so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	so->so_lastError = solisten (so, Backlog, NULL);
	return (ConvertToEfiStatus (so->so_lastError));
}

static EFI_STATUS EFIAPI
Accept(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    OUT SOCKET              *NewSocket,
    OUT EFI_SOCKADDR	    *Addr,
    IN OUT UINT32           *AddrLen
	)
/*++

Routine Description:

    Accept a connection on a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	int				s, error;
	SOCKET			index;
	struct sockaddr	*sa;
	struct socket	*so, *head = GETSOCKET (Socket);
	unsigned int oldTPL;

	VALIDATE_SOCKET (head);

	//
	//  Do a first pass check to make sure we have room in the socket table.
	//  We'll need to do this again before we assigne the true socket index.
	//

	for (index = 0; index < MAX_SOCKETS; ++index)
		if (SocketTable[ index ] == NULL)
			break;

	if (index == MAX_SOCKETS) {
		error = ENFILE;
		goto Error;
	}

	//
	//  Do some state checking
	//

	if ((head->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto Error;
	}

	if ((head->so_state & SS_NBIO) && head->so_comp.tqh_first == NULL) {
		error = EWOULDBLOCK;
		goto Error;
	}

	//
	//  Wakeup once in a while to check for errors and to prevent race where
	//  a connect gets queued after the tqh_first check and before tsleep()
	//  does the EFI WaitForEvent.
	//

        oldTPL = splnet();
        
	while (head->so_comp.tqh_first == NULL && head->so_error == 0) {
		if (head->so_state & SS_CANTRCVMORE)  {
			head->so_error = ECONNABORTED;
			break;
		}

		error = tsleep ((caddr_t)&head->so_timeo, PSOCK | PCATCH, "accept", 20);
		if (error) {
			if (error == EWOULDBLOCK) {
				continue;
			}
			splx(oldTPL);
			goto Error;
		}
	}

        splx(oldTPL);
        
	if (head->so_error) {
		error = head->so_error;
		head->so_error = 0;
		goto Error;
	}

	//
	// At this point we know that there is at least one connection
	// ready to be accepted. Remove it from the queue prior.
	//

	s = splhigh ();
	so = head->so_comp.tqh_first;
	TAILQ_REMOVE (&head->so_comp, so, so_list);
	head->so_qlen--;
	splx (s);

	so->so_state &= ~SS_COMP;
	so->so_head = NULL;
	if (head->so_sigio != NULL) {
		//fsetown(fgetown(head->so_sigio), &so->so_sigio);
		so->so_sigio = head->so_sigio;
	}

	sa = NULL;
	if (error = soaccept (so, &sa)) {
		soclose (so);
		if (sa)
			free (sa, 0);
		goto Error;
	}

	if (sa == NULL) {
		*AddrLen = 0;
	} else  if (Addr) {

		//
		//  Convert address
		//

		ConvertBsdToEfiSockAddr( sa, Addr, *AddrLen );
	}

	//
	//  We're done with the FreeBSD style sockaddr
	//

	if (sa)
		free (sa, 0);

	//
	//  Time to find a socket index
	//
	
	s = splhigh ();
	for (index = 0; index < MAX_SOCKETS; ++index)
		if (SocketTable[ index ] == NULL)
			break;

	if (index == MAX_SOCKETS) {

		//
		//  No slots left
		//

		soclose (so);
		error = ENFILE;
	} else {

		//
		//  Record the socket buffer
		//

		SocketTable[ index ] = so;
		*NewSocket = index;
	}
	splx (s);

Error:
	head->so_lastError = error;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
Connect(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  EFI_SOCKADDR	    *Addr,
    IN  UINT32              AddrLen
	)
/*++

Routine Description:

    Initiate a connection on a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	int				error;
	struct sockaddr	BsdSockAddr;
	struct socket	*so = GETSOCKET (Socket);
	unsigned int oldTPL;

	VALIDATE_SOCKET (so);

	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
		error = EALREADY;
		goto bad;
	}
 
	ConvertEfiToBsdSockAddr (Addr, &BsdSockAddr, AddrLen);
	error = soconnect (so, &BsdSockAddr, NULL);

	if (error)
		goto bad;

	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
		error = EINPROGRESS;
		goto bad;
	}
 
	//
	//  Wakeup once in a while to check for errors and to prevent race where
	//  a connect completes after the SS_ISCONNECTING check and before tsleep()
	//  does the EFI WaitForEvent.
	//

        oldTPL = splnet();

	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
		error = tsleep ((caddr_t)&so->so_timeo, PSOCK | PCATCH, "connec", 20);
		if (error) {
			if (error == EWOULDBLOCK) {
				error = 0;
				continue;
			}
			break;
		}
	}

        splx(oldTPL);
        
	if (error == 0) {
		error = so->so_error;
		so->so_error = 0;
	}

bad:
	so->so_state &= ~SS_ISCONNECTING;
	if (error == (u_int)ERESTART)
		error = EINTR;

	so->so_lastError = error;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
Send(
    IN  struct _EFI_SOCKET  *This,
	IN	SOCKET				Socket,
	IN	UINT8				*Buffer,
	IN	UINT32				BufferSize,
	IN	UINT32				Flags,
	IN	EFI_SOCKADDR		*Addr,
	IN	UINT32				AddrLen,
	OUT	UINT32				*BytesSent
	)
/*++

Routine Description:

    Send data through the network stack.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	int				error;
	struct uio		auio;
	struct iovec	aiov;
	struct sockaddr	BsdSockAddr, *pBsdSockAddr;
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	//
	//  Setup an IO vector
	//

	aiov.iov_base = (char*)Buffer;
	aiov.iov_len  = BufferSize;

	auio.uio_iov    = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw     = UIO_WRITE;
	auio.uio_procp  = NULL;
	auio.uio_offset = 0;
	auio.uio_resid  = (int)aiov.iov_len;

	if (Addr) {
		ConvertEfiToBsdSockAddr (Addr, &BsdSockAddr, AddrLen);
		pBsdSockAddr = &BsdSockAddr;
	} else {
		pBsdSockAddr = NULL;
	}

	error = so->so_proto->pr_usrreqs->pru_sosend (so, pBsdSockAddr, &auio, 0, NULL, Flags, NULL);
	if (error) {
		if ((auio.uio_resid != (int)BufferSize) &&
		    (error == ERESTART || error == EINTR || error == EWOULDBLOCK))
			error = 0;
	}

	so->so_lastError = error;
	*BytesSent = error ? -1 : BufferSize - auio.uio_resid;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
Receive(
    IN  struct _EFI_SOCKET  *This,
	IN	SOCKET				Socket,
	OUT	UINT8				*Buffer,
	IN	UINT32				BufferSize,
	IN	UINT32				Flags,
	OUT	EFI_SOCKADDR		*Addr,
	IN OUT UINT32			*AddrLen,
	OUT UINT32				*BytesReceived
	)
/*++

Routine Description:

    Receive data on a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	int				error;
	struct uio		auio;
	struct iovec	aiov;
	struct sockaddr	*fromsa = 0;
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	//
	//  Setup an IO vector
	//

	aiov.iov_base = (char*)Buffer;
	aiov.iov_len  = BufferSize;

	auio.uio_iov    = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw     = UIO_READ;
	auio.uio_procp  = NULL;
	auio.uio_offset = 0;
	auio.uio_resid  = (int)aiov.iov_len;

	error = so->so_proto->pr_usrreqs->pru_soreceive (so,
													 &fromsa,
													 &auio,
													 (struct mbuf **)0,
													 (struct mbuf **)0,
													 (int*)&Flags );
	if (error) {
		if ((auio.uio_resid != (int)BufferSize) &&
		     (error == ERESTART || error == EINTR || error == EWOULDBLOCK)) {
			error = 0;
		} else {
			goto out;
		}
	}

	if (Addr && AddrLen) {
		if ( *AddrLen <= 0 || fromsa == 0)
			*AddrLen = 0;
		else {
			ConvertBsdToEfiSockAddr (fromsa, Addr, *AddrLen);
		}
	}

out:
	if (fromsa)
		free(fromsa, 0);

	so->so_lastError = error;
	*BytesReceived = error ? -1 : BufferSize - auio.uio_resid;

	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
PollSocket(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  SOCKET_EVENTS       EventMask,
    OUT SOCKET_EVENTS       *EventResult
   )
/*++

Routine Description:

    Poll a socket for competed socket events.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	SOCKET_EVENTS	EfiResult;
	int				BsdMask, BsdResult;

	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	//
	//  Convert from EFI to BSD
	//

	BsdMask = 0;

	if (EventMask & EFI_SOCKPOLL_RECV_ANY)		BsdMask |= POLLIN;
	if (EventMask & EFI_SOCKPOLL_RECV_NORMAL)	BsdMask |= POLLRDNORM;
	if (EventMask & EFI_SOCKPOLL_RECV_SPECIAL)	BsdMask |= (POLLPRI | POLLRDBAND);
	if (EventMask & EFI_SOCKPOLL_SEND_ANY)		BsdMask |= POLLOUT;
	if (EventMask & EFI_SOCKPOLL_SEND_NORMAL)	BsdMask |= POLLWRNORM;
	if (EventMask & EFI_SOCKPOLL_SEND_SPECIAL)	BsdMask |= POLLWRBAND;

	//
	//  See if anything to do
	//

	if ( BsdMask == 0 )
		return (EFI_UNSUPPORTED);

	//
	//  Make call
	//

	BsdResult = so->so_proto->pr_usrreqs->pru_sopoll (so, BsdMask, NULL, NULL);

	//
	//  Convert from BSD to EFI
	//

	EfiResult = 0;

	if (BsdResult & POLLIN)					EfiResult |= EFI_SOCKPOLL_RECV_ANY;
	if (BsdResult & POLLRDNORM)				EfiResult |= EFI_SOCKPOLL_RECV_NORMAL;
	if (BsdResult & (POLLPRI | POLLRDBAND))	EfiResult |= EFI_SOCKPOLL_RECV_SPECIAL;
	if (BsdResult & POLLOUT)				EfiResult |= EFI_SOCKPOLL_SEND_ANY;
	if (BsdResult & POLLWRNORM)				EfiResult |= EFI_SOCKPOLL_SEND_NORMAL;
	if (BsdResult & POLLWRBAND)				EfiResult |= EFI_SOCKPOLL_SEND_SPECIAL;

	//
	//  Store the result
	//
	*EventResult = EfiResult;

	return (EFI_SUCCESS);
}

static EFI_STATUS EFIAPI
GetSockOpt(
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT32              Level,
    IN  UINT32              OptName,
    OUT void                *OptVal,
    IN OUT  UINT32          *OptLen
    )
/*++

Routine Description:

    Get options on a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct sockopt	sopt;
	struct linger	BsdLinger;
	struct socket	*so = GETSOCKET (Socket);
	int				error;

	VALIDATE_SOCKET (so);

	if ((OptVal == NULL) || (OptLen == NULL) || (*OptLen == 0) || ((int)(*OptLen) < 0)) {
		error = EINVAL;
		goto Error;
	}

	if (OptName == EFI_SOCKOPT_LINGER && *OptLen > sizeof(EFI_SOCK_LINGER)) {
		error = EINVAL;
		goto Error;
	}

	sopt.sopt_dir     = SOPT_GET;
	sopt.sopt_level   = Level;
	sopt.sopt_name    = ConverEfiToBsdOption (OptName);	// Convert from EFI to BSD names
	sopt.sopt_p       = NULL;

	//
	//  Check for structure conversion
	//

	switch (OptName) {
		case EFI_SOCKOPT_LINGER:
			sopt.sopt_val     = &BsdLinger;
			sopt.sopt_valsize = sizeof(BsdLinger);
			break;

		default:
			sopt.sopt_val     = OptVal;
			sopt.sopt_valsize = *OptLen;
	}

	error = sogetopt (so, &sopt);
	if (error == 0) {
		//
		//  Check for structure conversion
		//

		switch (OptName) {
			case EFI_SOCKOPT_LINGER: {
				EFI_SOCK_LINGER *pLinger = (EFI_SOCK_LINGER*)OptVal;

				pLinger->OnOff = BsdLinger.l_onoff;
				pLinger->Seconds = BsdLinger.l_linger;
				sopt.sopt_valsize = sizeof(EFI_SOCK_LINGER);	// Size checked on entry
				break;
			}
		}

		*OptLen = (UINT32)sopt.sopt_valsize;
	}

Error:
	so->so_lastError = error;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
SetSockOpt(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               Level,
    IN UINT32               OptName,
    IN void                 *OptVal,
    IN UINT32               OptLen
    )
/*++

Routine Description:

    Set options on a socket.

Arguments:
    
    TBD

Returns:

    0 on success, -1 on error

--*/
{
	struct sockopt	sopt;
	struct linger	BsdLinger;
	struct socket	*so = GETSOCKET (Socket);
	int				error;

	VALIDATE_SOCKET (so);

	if ((OptVal == 0 && OptLen != 0) || (INT32)OptLen < 0) {
		error = EFAULT;
		goto Error;
	}

	sopt.sopt_dir     = SOPT_SET;
	sopt.sopt_level   = Level;
	sopt.sopt_name    = ConverEfiToBsdOption (OptName);	// Convert from EFI to BSD names;
	sopt.sopt_p       = NULL;

	//
	//  Check for structure conversion
	//

	switch (OptName) {
		case EFI_SOCKOPT_LINGER: {
			EFI_SOCK_LINGER *pLinger = (EFI_SOCK_LINGER*)OptVal;

			if (OptLen < sizeof(EFI_SOCK_LINGER)) {
				error = EFAULT;
				goto Error;
			}

			BsdLinger.l_onoff  = pLinger->OnOff;
			BsdLinger.l_linger = pLinger->Seconds; 

			sopt.sopt_val     = &BsdLinger;
			sopt.sopt_valsize = sizeof(BsdLinger);
			break;
		}

		default:
			sopt.sopt_val     = OptVal;
			sopt.sopt_valsize = OptLen;
	}

	error = sosetopt (so, &sopt);

Error:
	so->so_lastError = error;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
Shutdown(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               How
	)
/*++

Routine Description:

    Shut down part of a full-duplex connection.

Arguments:
    
    TBD

Returns:

    0 on success, -1 on error

--*/
{
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	so->so_lastError = soshutdown (so, ConverEfiToBsdShutdown(How));
	return (ConvertToEfiStatus (so->so_lastError));
}

static EFI_STATUS EFIAPI
SocketIoctl(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               Cmd,
    IN OUT VOID             *Argp
	)
/*++

Routine Description:

    Socket I/O control funtion.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct socket	*so = GETSOCKET (Socket);
	int				error;

	VALIDATE_SOCKET (so);

	so->so_lastError = 0;

	switch (Cmd) {

	case 0: {
		extern void if_force_poll();
		if_force_poll();
		return (EFI_SUCCESS);
	}

	case FIONBIO:
		if (*(UINT32 *)Argp)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return (EFI_SUCCESS);

	case FIOASYNC:
		if (*(EFI_EVENT *)Argp) {
			so->so_state |= SS_ASYNC;
			so->so_rcv.sb_flags |= SB_ASYNC;
			so->so_snd.sb_flags |= SB_ASYNC;
			so->so_sigio = *(EFI_EVENT *)Argp;
		} else {
			so->so_state &= ~SS_ASYNC;
			so->so_rcv.sb_flags &= ~SB_ASYNC;
			so->so_snd.sb_flags &= ~SB_ASYNC;
			so->so_sigio = NULL;
		}
		return (EFI_SUCCESS);

	case FIONREAD:
		*(UINT32 *)Argp = so->so_rcv.sb_cc;
		return (EFI_SUCCESS);

	case SIOCATMARK:
		*(UINT32 *)Argp = (so->so_state&SS_RCVATMARK) != 0;
		return (EFI_SUCCESS);

	case IOCSYSCTL: {
		extern sysctl_rtsock(), tcp_pcblist(), udp_pcblist();

		int		*name;
		u_int	namelen;
		struct sysctl_req	*req;
		struct pseudo_sysctl *SysctlArgs = (struct pseudo_sysctl*)Argp;

		name = &SysctlArgs->name[2];
		namelen = SysctlArgs->namelen - 2;
		req = &SysctlArgs->req;

		switch (SysctlArgs->name[1]) {
			case PF_ROUTE:
				error = sysctl_rtsock (NULL, name, namelen, req);
				break;

			case TCPCTL_PCBLIST:
				error = tcp_pcblist (NULL, name, namelen, req);
				break;

			case UDPCTL_PCBLIST:
				error = udp_pcblist( NULL, name, namelen, req);
				break;

			default:
				error = EINVAL;
		}
		goto out;
	  }
	}

	//
	// Interface/routing/protocol specific ioctls:
	// interface and routing ioctls should have a
	// different entry since a socket's unnecessary
	//

	if (IOCGROUP(Cmd) == 'i')
		error = ifioctl (so, Cmd, Argp, NULL);
	else if (IOCGROUP(Cmd) == 'r')
		error = rtioctl (Cmd, Argp, NULL);
	else
		error = (*so->so_proto->pr_usrreqs->pru_control) (so, Cmd, Argp, 0, NULL);

out:
	so->so_lastError = error;
	return (ConvertToEfiStatus (error));
}

static EFI_STATUS EFIAPI
GetPeerAddr(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
	OUT	EFI_SOCKADDR		*Addr,
	IN OUT UINT32			*AddrLen
	)
/*++

Routine Description:

    Get peer address for a socket

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct sockaddr *sa = NULL;
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);


	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		so->so_lastError = ENOTCONN;
		return (EFI_SOCKERR_NOTCONN);
	}

	if (!(so->so_lastError = (*so->so_proto->pr_usrreqs->pru_peeraddr)(so, &sa))) {

		//
		//  Convert address
		//

		ConvertBsdToEfiSockAddr( sa, Addr, *AddrLen );	
	}

	if (sa)
		free(sa, 0);

	return (ConvertToEfiStatus (so->so_lastError));
}

static EFI_STATUS EFIAPI
GetSockAddr(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
	OUT	EFI_SOCKADDR		*Addr,
	IN OUT UINT32			*AddrLen
	)
/*++

Routine Description:

    Get local address for a socket

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct sockaddr *sa = NULL;
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	if (!(so->so_lastError = (*so->so_proto->pr_usrreqs->pru_sockaddr)(so, &sa))) {

		//
		//  Convert address
		//

		ConvertBsdToEfiSockAddr( sa, Addr, *AddrLen );	
	}

	if (sa)
		free(sa, 0);

	return (ConvertToEfiStatus (so->so_lastError));
}

static EFI_STATUS EFIAPI
CloseSocket(
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket
	)
/*++

Routine Description:

    Close a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct socket	*so = GETSOCKET (Socket);

	VALIDATE_SOCKET (so);

	LastError = soclose (so);

	SocketTable[ Socket ] = NULL;

	return (ConvertToEfiStatus (LastError));
}

static EFI_STATUS EFIAPI
GetLastError(
    IN  struct _EFI_SOCKET  *This,
    IN	SOCKET               Socket,
	OUT	UINT32				*pLastError
	)
/*++

Routine Description:

    Return the last error on a socket.

Arguments:
    
    TBD

Returns:

    EFI_STATUS

--*/
{
	struct socket *so = GETSOCKET (Socket);
	*pLastError = (so ? so->so_lastError : LastError);
	return (EFI_SUCCESS);
}

VOID
InitSocketInterface(
	VOID
	)
/*++

Routine Description:

    Initialize socket interface.

Arguments:
    
    TBD

Returns:

    None

--*/
{
	int	i;

	for (i = 0; i < MAX_SOCKETS; i++)
		SocketTable[ i ] = NULL;
}

//
//  The EFI Socket Interface Table
//
EFI_SOCKET_INTERFACE EfiSocketInterface = {

	EFI_SOCKET_INTERFACE_REVISION,

	GetVendorGuid,
	GetProtocols,
	Socket,
	Bind,
	Listen,
	Accept,
	Connect,
	Send,
	Receive,
	PollSocket,
	GetSockOpt,
	SetSockOpt,
	Shutdown,
	SocketIoctl,
	GetPeerAddr,
	GetSockAddr,
	CloseSocket,
	GetLastError,
};

/*++

Routine Description:

    EFI emulation routine.

Arguments:
    
    TBD

Returns:

    Pointer to an EFI socket interface

--*/
EFI_SOCKET_INTERFACE*
GetEfiSocketInterface() { return &EfiSocketInterface; }

