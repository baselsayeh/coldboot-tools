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

    efi_netiface.c

Abstract:

    This file contains the interface slick between FreeBSD's notion
	of a network interface and an EFI network interface protocol.


Revision History

--*/

#include <efisocket.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>
#include <sys/syslog.h>

//
//  Shorten the interface name
//
#define EFI_SNI	EFI_SIMPLE_NETWORK

//
//  Guess on how much extra rx/tx resources we might use based on
//  system hz value
//
#define	BITS_PER_SECOND		(100 * 1000000)	/* 100Mbits */

#define	BYTES_PER_SECOND	(BITS_PER_SECOND / 8)
#define	FRAMES_PER_SECOND	(BYTES_PER_SECOND / ETHER_MAX_LEN)
#define EXTRA_RX_BUF_SIZE	((FRAMES_PER_SECOND / hz) * ETHER_MAX_LEN)
#define EXTRA_TX_BUF_SIZE	0

extern	EFI_BOOT_SERVICES		*TcpIpBS;

typedef struct {
	struct	arpcom		arpcom;
	EFI_HANDLE			Handle;
	EFI_SNI				*pIface;
	u_char				MacAddr[ ETHER_ADDR_LEN ];
} softc_t;


static EFI_GUID	NetworkInterfaceGuid = EFI_SIMPLE_NETWORK_PROTOCOL;
static short	UnitNumber = 0;		// Monotomically incremented for each adapter attached.
static int		TxPosted  = 0;		// Number of posted transmits in SNI

//
//  SNI protocol registration key
//
static VOID	*EfiNetIfaceNotifyReg;

//
//  This routine is equivalent to m_devget() except that it insures that the
//  network header following the ethernet header will be on an aligned boundary
//
static struct mbuf*
aligned_m_devget(
	char			*buf,
	int				totlen,
	int				off,
	struct ifnet 	*ifp,
	void			*DummyCopyFunc
	)
{
	struct mbuf *m;
	struct mbuf *top = 0, **mp = &top;
	int			len;

	if (off) {
		buf += off + 2 * sizeof(u_short);
		totlen -= 2 * sizeof(u_short);
	}

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return (0);
	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = totlen;
	len = MHLEN;

	while (totlen > 0) {
		if (top) {
			MGET(m, M_DONTWAIT, MT_DATA);
			if (m == 0) {
				m_freem(top);
				return 0;
			}
			len = MLEN;
		}
		if (totlen >= MINCLSIZE) {
			MCLGET(m, M_DONTWAIT);
			if ((m->m_flags & M_EXT) == 0) {
				m_free(m);
				m_freem(top);
				return 0;
			}
			len = MCLBYTES;
		}
		if (!top) {
			int pad = ALIGN(sizeof(struct ether_header)) -
							sizeof(struct ether_header);
			m->m_data += pad;
			len -= pad;
		}
		m->m_len = len = min(totlen, len);
		bcopy(buf, mtod(m, caddr_t), (unsigned)len);
        buf += len;
        totlen -= len;
        *mp = m;
        mp = &m->m_next;
    }

    return (top);
}

static EFI_STATUS
ResetInterface( softc_t *sc )
/*++

Routine Description:

    Reset SNI interface

Arguments:
    
    sc	- Pointer to softc_t for interface

Returns:

    EFI status

--*/

{
	EFI_STATUS	Status;
	EFI_SNI		*pIface;
	BOOLEAN		ResetAttempted = FALSE;	

	//
	//  Get the adapter into a known state
	//

	pIface = sc->pIface;

retry:
	Status = pIface->Start (pIface);
	if (EFI_ERROR(Status)) {

		//
		//  There was an error.  See if we can't get the interface reset.
		//

		if (!ResetAttempted) {
			ResetAttempted = TRUE;

			Status = pIface->Shutdown (pIface);
			if (!EFI_ERROR(Status)) {
				Status = pIface->Stop (pIface);
				if (!EFI_ERROR(Status)) {
					goto retry;
				}
			}
		}

		//
		//  Couldn't initialize interface
		//
		printf("Network initialize error %p\n", (void*)Status);

	} else {

		//
		//  Initialize the interface
		//

		Status = pIface->Initialize (
									pIface,
									0, //EXTRA_RX_BUF_SIZE,
									0  //EXTRA_TX_BUF_SIZE
									);
		if (EFI_ERROR(Status)) {

			//
			//  Couldn't start the interface
			//

			printf("Network start error %p\n", (void*)Status);

		} else {

			//
			//  Set receive filter
			//

			Status = pIface->ReceiveFilters (
										pIface,
										EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
										EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST,
										0,
										FALSE,
										0,
										NULL
										);
			if (EFI_ERROR(Status)) {

				//
				//  Error setting receive filter
				//

				printf("Network ReceiveFilter error %p\n", (void*)Status);

			} else if (pIface->Mode->State != EfiSimpleNetworkInitialized)  {

				//
				//  We're not in the expected state
				//

				printf("Network state error %x\n", pIface->Mode->State);
				Status = EFI_DEVICE_ERROR;

			} else if ( pIface->Mode->HwAddressSize != ETHER_ADDR_LEN ) {

				//
				//  Unexpected address len
				//

				printf("Incompatible MAC address lengths - wanted %d got %d\n",
							ETHER_ADDR_LEN, (int)pIface->Mode->HwAddressSize);
				Status = EFI_UNSUPPORTED;

			} else {

				//
				//  Get the MAC address
				//

				bcopy (pIface->Mode->CurrentAddress.Addr, sc->MacAddr, ETHER_ADDR_LEN);
			}
		}
	}

	//
	//  If there was an error somewhere, shutdown the interface.
	//
	if (EFI_ERROR(Status)) {
		(VOID)pIface->Shutdown (pIface);
		(VOID)pIface->Stop (pIface);
	}

	return (Status);
}

static int
DrainEfiTxQueue( EFI_SIMPLE_NETWORK *pIface )
{
	EFI_STATUS	Status;
	u_char		*TxBuf;
	int			Total = 0;

	if ( TxPosted ) {
		do {
			Status = pIface->GetStatus (pIface, NULL, (void**)&TxBuf);
			if ( EFI_ERROR( Status ) ) {
				break;
			}
			if ( TxBuf != NULL ) {
				free (TxBuf, 0);
				TxPosted--;
				Total++;
			}
		} while (TxBuf != NULL);
	}

	if (TxPosted < 0) {
		log (LOG_ERR, "DrainEfiTxQueue: TxPosted botch (%d)\n", TxPosted);
		TxPosted = 0;
	}

	return (Total);
}

static VOID
Sni_ifstart(
	struct ifnet *ifp
	)
/*++

Routine Description:

    Initiate transmit of any mbufs in the send queue

Arguments:
    
    ifp		- Pointer to the interface structure

Returns:

    None

--*/

{
	struct mbuf	*m;
	softc_t		*sc;
	EFI_SNI		*pIface;
	EFI_STATUS	Status;
	u_char		*Frame;
	int			NumXmited;	// for debug/logging
	
	sc = ((softc_t*)ifp->if_softc);
	pIface = sc->pIface;

	for (NumXmited = 0; ;NumXmited++) {

		//
		//  See if there are frames queued
		//

		IF_DEQUEUE( &ifp->if_snd, m );
		if (m == NULL )
			break;

		//
		//  Allocate memory.
		//

		Frame = (u_char*)malloc (m->m_pkthdr.len, 0, 0);
		if ( Frame == NULL ) {

			//
			//  No memory.  Put frame back and maybe we'll have better
			//  luck next time.
			//

			log (LOG_WARNING, "Sni_ifstart: No memory for frame\n");
			IF_PREPEND (&ifp->if_snd, m);
			break;
		}

		//
		//  Transfer mbuf to contiguous memory.
		//

		m_copydata (m, 0, m->m_pkthdr.len, (caddr_t)Frame);

		//
		//  Send frame
		//

		TxPosted++;	// do this here so we don't have to spl to prevent race
		Status = pIface->Transmit ( pIface,
									0,
									m->m_pkthdr.len,
									Frame,
									NULL,
									NULL,
									NULL
									);
		//
		//  See if all went well
		//

		if (EFI_ERROR(Status)) {

			//
			//  Undo earlier optimism and try again
			//

			TxPosted--;
			free (Frame, 0);

			//
			//  If we're backed up, try draining finished TX requests
			//

			if ( Status == EFI_NOT_READY ) {
				if ( TxPosted == 0 ) {
					log(LOG_ERR, "SNP returned EFI_NOT_READY with nothing posted\n");
				} else {
					(void)DrainEfiTxQueue (pIface);
				}
			} else {

				//
				//  Seems to be serious - reset the interface
				//

				log (LOG_ERR, "Error %p transmitting - resetting interface\n", Status);
				Status = ResetInterface( sc );
				if (EFI_ERROR(Status)) {
					log (LOG_ERR, "Error %p reseting interface\n", Status);
				}
			}
		}

		//
		//  Frame has been either queued to the interface or dumped so free it.
		//

		m_freem (m);
	}
}

static VOID
Sni_ifinit( VOID* x )
{
	softc_t		*sc = x;
	struct ifnet	*ifp = &sc->arpcom.ac_if;

	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;
}

static int
Sni_ifioctl(struct ifnet * ifp, u_long cmd, caddr_t data)
{
	struct ifreq		*ifr = (struct ifreq *) data;
	int			error;

	switch( cmd ) {
		case SIOCSIFADDR:
		case SIOCGIFADDR:
		case SIOCSIFMTU:
			error = ether_ioctl (ifp, cmd, data);
			break;
		default:
			error = EINVAL;
			break;
	}

	return (error);
}

static VOID
Sni_ifwatchdog(struct ifnet *ifp)
{
	return;
}

static int
Sni_ifpoll_recv(struct ifnet *ifp, int *unused)
{
	struct mbuf			*m;
	struct ether_header	*e;
	UINTN				len;
	int					DidRecv;
	softc_t				*sc;
	EFI_SNI				*pIface;
	EFI_STATUS			Status;
	u_char				Frame[ ETHER_MAX_LEN ];
	
	sc = ((softc_t*)ifp->if_softc);
	pIface = sc->pIface;

	//
	//  Let's take this opportunity to drain completed transmit buffers
	//

	(void)DrainEfiTxQueue (pIface);

	//
	//  Take as many frames as the interface will give us
	//

	for (DidRecv = 0; ;) {
		len = sizeof(Frame);
		Status = pIface->Receive (pIface, NULL, &len, Frame, NULL, NULL, NULL);

		//
		//  Deal with the error
		//

		if (EFI_ERROR(Status)) {
			if (Status == EFI_NOT_READY) {
				break;
			} else {
				printf("Network receive error %p\n", (void*)Status);
				continue;
			}
		}

		//
		//  Put the frame into an mbuf
		//
		m = aligned_m_devget( (char*)Frame, (int)len, 0, ifp, NULL );

		//
		//  If mbuf was allocated, process the frame
		//

		if (m) {
			e = mtod (m, struct ether_header *);
			m_adj (m, sizeof(struct ether_header));
			ether_input (ifp, e, m);
			DidRecv++;
		} else {
			printf("Network receive: no buffers\n");
			break;
		}
	}

	return DidRecv;
}

static int  Sni_ifpoll_xmit(struct ifnet *ifp, int *unused) { return 0; }
static void Sni_ifpoll_intren(struct ifnet *ifp) { return; }

static VOID
AttachSniDevice(
	softc_t		*sc
	)
{
	struct ifnet  * const ifp = &sc->arpcom.ac_if;
	struct arpcom * const arp = &sc->arpcom;

	bcopy (sc->MacAddr, arp->ac_enaddr, sizeof(arp->ac_enaddr));

	ifp->if_softc          = sc;
	ifp->if_flags          = (ushort)(IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST);
	ifp->if_init           = Sni_ifinit;
	ifp->if_ioctl          = Sni_ifioctl;
	ifp->if_start          = Sni_ifstart;
	ifp->if_poll_recv      = Sni_ifpoll_recv;
	ifp->if_poll_xmit      = Sni_ifpoll_xmit;
	ifp->if_poll_intren    = Sni_ifpoll_intren;
	ifp->if_name           = "sni";
	ifp->if_unit           = UnitNumber++;
	ifp->if_watchdog       = Sni_ifwatchdog;
	ifp->if_timer          = 1;
	ifp->if_output         = ether_output;
	ifp->if_snd.ifq_maxlen = 20; /* send queue length */

//	printf("%02x:%02x:%02x:%02x:%02x:%02x ",
//		arp->ac_enaddr[0], arp->ac_enaddr[1], arp->ac_enaddr[2],
//		arp->ac_enaddr[3], arp->ac_enaddr[4], arp->ac_enaddr[5]);

	if_attach (ifp);
	ether_ifattach (ifp);
}

static EFI_STATUS
InitSniDevice(
	EFI_HANDLE	Handle,
	softc_t		*sc
	)
/*++

Routine Description:

    Create a softc data structure for this SNI device and attach it
	to the network stack

Arguments:
    
    Handle	- EFI handle for the SNI protocol to attach

Returns:

    EFI_STATUS

--*/
{
	EFI_STATUS	Status;
	EFI_SNI		*pIface;
	BOOLEAN		ResetAttempted = FALSE;	

	//
	//  Get the interface for this instance of the protocol
	//

	Status = TcpIpBS->HandleProtocol (Handle, &NetworkInterfaceGuid, (void*)&pIface);
	if ( ! EFI_ERROR (Status)) {
		sc->pIface = pIface;

		//
		//  Reset interface
		//

		Status = ResetInterface( sc );
	}
	
	return (Status);
}

static VOID
NotifyNewSni (
    EFI_EVENT       Event,
    VOID            *RegContext
    )
/*++

Routine Description:

    Notification routine that is invoked whenever a
    handle supports the SNI protocol is added
    to the system.  It will attach the SNI device to
	the network stack.

Arguments:

    Event       - The signaled event
    RegContext  - Registration key

Returns:

    None

--*/
{
    EFI_HANDLE      Handle;
    UINTN           BufferSize;
    EFI_STATUS      Status;
	softc_t			*sc;

    //
    // Examine all new handles
    //

    for (; ;) {

        //
        // Get the next handle
        //

        BufferSize = sizeof(Handle);
        Status = TcpIpBS->LocateHandle (
							ByRegisterNotify,
							NULL,
							*(VOID**)RegContext,
							&BufferSize,
							&Handle
							);
        //
        // If not found, we're done
        //

        if (Status == EFI_NOT_FOUND) {
            break;
        }

        if (EFI_ERROR (Status)) {
			printf("NotifyNewSni: LocateHandle() returned %p\n", (void*)Status);
			goto bad;
		}

		//
		//  Allocate memory and clear our softc
		//

		sc = (softc_t*)malloc (sizeof(softc_t), 0, 0);
		if (sc == NULL) {
			printf("NotifyNewSni: No memory for softc_t\n");
			goto bad;
		}
		bzero (sc, sizeof(softc_t));	// Must be zeroed

		//
		//  Initialize Adapter
		//
		Status = InitSniDevice (Handle, sc);
		if (EFI_ERROR (Status)) {
			printf ("Error %p intializing SNI interface\n", (void*)Status);
			free (sc, 0);
		} else {

			//
			//  Attach adapter to network stack
			//

			AttachSniDevice (sc);
		}
	}
bad:
        TcpIpBS->CloseEvent(Event);
    
	return;
}

EFI_STATUS
SniAttach(
	VOID
	)
{
	EFI_STATUS	Status;
	EFI_EVENT	Event;

    //
    // Install notification handler for SNI devices
    //

    Status = TcpIpBS->CreateEvent (
						EVT_NOTIFY_SIGNAL,
						TPL_CALLBACK,
						NotifyNewSni,
						&EfiNetIfaceNotifyReg,
						&Event
						);
    if (!EFI_ERROR (Status)) {

		Status = TcpIpBS->RegisterProtocolNotify (
								&NetworkInterfaceGuid,
								Event,
								&EfiNetIfaceNotifyReg
								);
		if (!EFI_ERROR (Status)) {

			//
			// Kick the event so we will perform an initial pass of
			// current installed drivers
			//

			Status = TcpIpBS->SignalEvent (Event);
		}
	}

	return (Status);
}
