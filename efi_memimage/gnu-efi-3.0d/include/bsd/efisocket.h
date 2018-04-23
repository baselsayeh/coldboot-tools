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

    socketproto.h

Abstract:

    EFI Socket Protocol Interface



Revision History

--*/

#ifndef _EFISOCKET_H_
#define _EFISOCKET_H_

#include "efi.h"

//
//  EFI Socket protocol
//

#define EFI_SOCKET_PROTOCOL \
    { 0x198de03a, 0x69fb, 0x11d3, 0xa7, 0x14, 0x0, 0xa0, 0xc9, 0x72, 0x37, 0xd }

#define EFI_SOCKET_INTERFACE_REVISION   0x00010000

#define EFI_SOCKERR(val)  EFIERR_OEM(0x10000 | (val))

//
//  In addition to standard EFI status codes, the following
//  EFI_SOCKET based retun values are defined which are
//  compatible with the EFI_ERROR() macro.
//

#define EFI_SOCKERR_FAILURE        EFI_SOCKERR(1)
#define EFI_SOCKERR_ADDRINUSE      EFI_SOCKERR(2)
#define EFI_SOCKERR_ADDRNOTAVAIL   EFI_SOCKERR(3)
#define EFI_SOCKERR_AFNOSUPPORT    EFI_SOCKERR(4)
#define EFI_SOCKERR_CONNABORTED    EFI_SOCKERR(5)
#define EFI_SOCKERR_CONNREFUSED    EFI_SOCKERR(6)
#define EFI_SOCKERR_CONNRESET      EFI_SOCKERR(7)
#define EFI_SOCKERR_HOSTUNREACH    EFI_SOCKERR(8)
#define EFI_SOCKERR_INPROGRESS     EFI_SOCKERR(9)
#define EFI_SOCKERR_ISCONN         EFI_SOCKERR(10)
#define EFI_SOCKERR_MSGSIZE        EFI_SOCKERR(11)
#define EFI_SOCKERR_NETDOWN        EFI_SOCKERR(12)
#define EFI_SOCKERR_NETUNREACH     EFI_SOCKERR(13)
#define EFI_SOCKERR_NOTCONN        EFI_SOCKERR(14)
#define EFI_SOCKERR_WOULDBLOCK     EFI_SOCKERR(15)

//
//  Socket options
//

#define	EFI_SOCKOPT_DEBUG			1
#define	EFI_SOCKOPT_ACCEPT_CONN		2
#define	EFI_SOCKOPT_REUSE_ADDR		3
#define	EFI_SOCKOPT_KEEP_ALIVE		4
#define	EFI_SOCKOPT_DONT_ROUTE		5
#define	EFI_SOCKOPT_BROADCAST		6
#define EFI_SOCKOPT_LINGER			7
#define	EFI_SOCKOPT_URGENT_INLINE	8
#define	EFI_SOCKOPT_SEND_BUF_SIZE	9
#define	EFI_SOCKOPT_RECV_BUF_SIZE	10
#define	EFI_SOCKOPT_SEND_LOW_WATER	11
#define	EFI_SOCKOPT_RECV_LOW_WATER	12
#define	EFI_SOCKOPT_SEND_TIMEOUT	13
#define	EFI_SOCKOPT_RECV_TIMEOUT	14
#define	EFI_SOCKOPT_ERROR_STATUS	15
#define	EFI_SOCKOPT_PROTOCOL_TYPE	16
#define	EFI_SOCKOPT_NON_BLOCKING_IO	17

//
//  Socket Shutdown defines
//

#define EFI_SOCK_SHUTDOWN_WR	0
#define EFI_SOCK_SHUTDOWN_RD	1
#define EFI_SOCK_SHUTDOWN_RDWR	2

//
//  Poll mask bits
//

#define	EFI_SOCKPOLL_CON_PENDING	0x0001
#define	EFI_SOCKPOLL_CON_COMPLETE	0x0002
#define	EFI_SOCKPOLL_CON_RESET  	0x0004
#define	EFI_SOCKPOLL_RECV_NORMAL  	0x0008
#define	EFI_SOCKPOLL_RECV_SPECIAL  	0x0010
#define	EFI_SOCKPOLL_SEND_NORMAL  	0x0020
#define	EFI_SOCKPOLL_SEND_SPECIAL  	0x0040

#define	EFI_SOCKPOLL_RECV_ANY   	(EFI_SOCKPOLL_RECV_NORMAL | \
									 EFI_SOCKPOLL_RECV_SPECIAL )
#define	EFI_SOCKPOLL_SEND_ANY   	(EFI_SOCKPOLL_SEND_NORMAL | \
									 EFI_SOCKPOLL_SEND_SPECIAL )

//
//  Socket type
//

typedef UINT32	SOCKET;

typedef struct {
    UINT8   AddressFamily;
    UINT8   AddressData[14];
} EFI_SOCKADDR;

typedef struct {
    UINT32  Domain;
    UINT32  Type;
    UINT32  Protocol;
} EFI_SOCKET_PROTO;

//
//  The following structures are used in
//  GetSockOpt() and SetSockOpt() calls.
//

typedef struct {
    UINT32  Seconds;
    UINT32  Microseconds;
} EFI_SOCK_TIMEOUT;

typedef struct {
    UINT32  OnOff;
    UINT32  Seconds;
} EFI_SOCK_LINGER;

typedef UINT32  SOCKET_EVENTS;

INTERFACE_DECL(_EFI_SOCKET);

typedef
EFI_STATUS
(EFIAPI *EFI_GETVENDORGUID) (
    IN  struct _EFI_SOCKET  *This,
    OUT EFI_GUID            *VendorGuid
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GETPROTOCOLS) (
    IN  struct _EFI_SOCKET  *This,
    IN OUT UINTN            *ArraySize,
    OUT EFI_SOCKET_PROTO    *ProtoArray
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SOCKET) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              *Socket,
    IN  UINT32              Domain,
    IN  UINT32              Type,
    IN  UINT32              Protocol
    );

typedef
EFI_STATUS
(EFIAPI *EFI_BIND) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  EFI_SOCKADDR        *Addr,
    IN  UINT32              AddrLen
    );

typedef
EFI_STATUS
(EFIAPI *EFI_LISTEN) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT32              Backlog
    );

typedef
EFI_STATUS
(EFIAPI *EFI_ACCEPT) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    OUT SOCKET              *NewSocket,
    OUT EFI_SOCKADDR        *Addr,
    IN OUT UINT32           *AddrLen
    );

typedef
EFI_STATUS
(EFIAPI *EFI_CONNECT) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  EFI_SOCKADDR        *Addr,
    IN  UINT32              AddrLen
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SEND) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT8               *Buffer,
    IN  UINT32              BufferSize, /* Note: not size_t */
    IN  UINT32              Flags,
    IN  EFI_SOCKADDR        *Addr,
    IN  UINT32              AddrLen,
    IN  UINT32              *BytesSent /* Note: not size_t */
    );

typedef
EFI_STATUS
(EFIAPI *EFI_RECEIVE) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    OUT UINT8               *Buffer,
    IN  UINT32              BufferSize, /* Note: not size_t */
    IN  UINT32              Flags,
    OUT EFI_SOCKADDR        *FromAddr,
    IN OUT UINT32           *FromLen,
    IN  UINT32              *BytesReceived /* Note: not size_t */
    );

typedef
EFI_STATUS
(EFIAPI *EFI_POLLSOCKET) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  SOCKET_EVENTS       EventMask,
    OUT SOCKET_EVENTS       *EventResult
   );

typedef
EFI_STATUS
(EFIAPI *EFI_GETSOCKOPT) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT32              Level,
    IN  UINT32              Option,
    OUT void                *Buffer,
    IN OUT  UINT32          *BufferSize
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SETSOCKOPT) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               Level,
    IN UINT32               Option,
    IN void                 *Buffer,
    IN UINT32               BufferSize
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHUTDOWN) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               How
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SOCKETIOCTL) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    IN UINT32               Cmd,
    IN OUT VOID             *Argp
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GETPEERADDR) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    OUT EFI_SOCKADDR        *Addr,
    IN OUT UINT32           *AddrLen
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GETSOCKADDR) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket,
    OUT EFI_SOCKADDR        *Addr,
    IN OUT UINT32           *AddrLen
    );

typedef
EFI_STATUS
(EFIAPI *EFI_CLOSESOCKET) (
    IN  struct _EFI_SOCKET  *This,
    IN SOCKET               Socket
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GETLASTERROR) (
    IN  struct _EFI_SOCKET  *This,
    IN  SOCKET              Socket,
    IN  UINT32              *LastError
    );

typedef struct _EFI_SOCKET {
    UINT64              Revision;

    EFI_GETVENDORGUID   GetVendorGuid;
	EFI_GETPROTOCOLS	GetProtocols;
    EFI_SOCKET          Socket;
    EFI_BIND            Bind;
    EFI_LISTEN          Listen;
    EFI_ACCEPT          Accept;
    EFI_CONNECT         Connect;
    EFI_SEND            Send;
    EFI_RECEIVE         Receive;
    EFI_POLLSOCKET      PollSocket;
    EFI_GETSOCKOPT      GetSockOpt;
    EFI_SETSOCKOPT      SetSockOpt;
    EFI_SHUTDOWN        Shutdown;
    EFI_SOCKETIOCTL     SocketIoctl;
    EFI_GETPEERADDR     GetPeerAddr;
    EFI_GETSOCKADDR     GetSockAddr;
    EFI_CLOSESOCKET     CloseSocket;
    EFI_GETLASTERROR    GetLastError;

} EFI_SOCKET_INTERFACE;
#endif /* _EFISOCKET_H_ */
