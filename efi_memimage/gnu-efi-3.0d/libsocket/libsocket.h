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

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <atk_guid.h>
#include <atk_libc.h>
#include <efisocket.h>
#include <string.h>		// for memcpy()
#ifdef KERNEL
#include <errno.h>
#else
#define KERNEL
#include <errno.h>
#undef KERNEL
#endif
#include <sys/socket.h>

//
//  Socket device name
//

#define	SOCKET_DEVICE	"Socket:"

//
//  Bit mask of supported protocols
//

#define IP_PROTO		0x0001
#define ICMP_PROTO		0x0002
#define	UDP_PROTO		0x0004
#define	TCP_PROTO		0x0008
#define RAW_PROTO		0x0010
#define	NEEDED_PROTOS	(IP_PROTO | ICMP_PROTO | UDP_PROTO | TCP_PROTO | RAW_PROTO)

//
//  RFC1700 protocol ID values
//

#define IP_PROTO_ID			0
#define ICMP_PROTO_ID		1
#define	UDP_PROTO_ID		6
#define	TCP_PROTO_ID		17
#define RAW_PROTO_ID		255

#define	EFI_INTERNET_DOMAIN	2

//
//  File I/O context
//

typedef struct {
	EFI_SOCKET_INTERFACE	*pIface;
	SOCKET					Socket;
	int						ParentFd;
} LibSockContext_t;

//
//  Macros
//
#define MIN(x, y)	(x) > (y) ? (y) : (x)

#define	GetSocketInterfacePtr()	__pSocket

#define	ConvertToEfiSockAddr(pBsdAddr, BsdLen, pEfiAddr, EfiLen) \
	do { \
		(pEfiAddr) = (EFI_SOCKADDR*)malloc ((BsdLen) + 1); \
		if ( (pEfiAddr) != NULL) { \
			(EfiLen) = (BsdLen) - 1; \
			memcpy (&(pEfiAddr)->AddressFamily, &(pBsdAddr)->sa_family, (EfiLen)); \
		} \
	} while (0)

#define	ConvertToBsdSockAddr(pEfiAddr, EfiLen, pBsdAddr, BsdLen) \
	do { \
		BsdLen = (EfiLen) + 1; \
		(pBsdAddr)->sa_len = BsdLen; \
		memcpy (&(pBsdAddr)->sa_family, &(pEfiAddr)->AddressFamily, (EfiLen)); /* EfiLen is correct here */ \
	} while (0)																 /* since BsdLen includes sa_len */

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

#define ConverBsdToEfiShutdown(val)             \
	val == SHUT_RD   ? EFI_SOCK_SHUTDOWN_RD   : \
	val == SHUT_WR   ? EFI_SOCK_SHUTDOWN_WR   : \
	val == SHUT_RDWR ? EFI_SOCK_SHUTDOWN_RDWR : \
	val

#define ConverEfiToBsdShutdown(val)             \
	val == EFI_SOCK_SHUTDOWN_RD   ? SHUT_RD   : \
	val == EFI_SOCK_SHUTDOWN_WR   ? SHUT_WR   : \
	val == EFI_SOCK_SHUTDOWN_RDWR ? SHUT_RDWR : \
	val

EFI_STATUS
EfiOpenSocket(
	char			*FilePath,
	char			*DevName,
	int				Flags,
	mode_t			Mode,
	EFI_DEVICE_PATH	*DevPath,
	EFI_GUID		*Guid,
	INT32			*fd
	);

//
//  Globals
//
extern EFI_SOCKET_INTERFACE	*__pSocket;

