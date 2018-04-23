/*-
 * Copyright (c) 1982, 1986, 1990, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 */

/*++

Module Name:

    efi_kern_support.c

Abstract:

    This file contains all of the kern support routines that make explicit
    use of EFI services.  The heart of many of the routines came from
    FreeBSD kernel source. 



Revision History

--*/

#include "efi.h"	/* must be first */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <machine/limits.h>
#include <atk_libc.h>

extern	EFI_BOOT_SERVICES	*TcpIpBS;

MALLOC_DEFINE(M_TEMP, "", "");
MALLOC_DEFINE(M_DEVBUF, "", "");
MALLOC_DEFINE(M_TTYS, "", "");

/*****************************************************************************
 *
 *  The following routines are supplied by the toolkit libc.  Currently,
 *  we must supply them under emulation.
 *
 *****************************************************************************/

#ifdef _EFI_EMULATION_

/*
 * Allocate a block of memory
 */
void*
malloc(size, type, flags)
	unsigned long size;
	struct malloc_type *type;
	int flags;
{
	void	*pMem;

	if ( TcpIpBS->AllocatePool( EfiBootServicesData, size, &pMem ) != EFI_SUCCESS ) {
		return NULL;
	}

	return ( pMem );
}


/*
 * Free a block of memory allocated by malloc.
 */
void
free(addr, type)
	void *addr;
	struct malloc_type *type;
{
	TcpIpBS->FreePool( addr );
}


/*
 * sizeof(word) MUST BE A POWER OF TWO
 * SO THAT wmask BELOW IS ALL ONES
 */
typedef	int word;		/* "word" used for optimal copy speed */

#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

/*
 * Copy a block of memory, handling overlap.
 * This is the routine that actually implements
 * (the portable versions of) bcopy, memcpy, and memmove.
 */
void
bcopy(src0, dst0, length)
	const void *src0;
	void *dst0;
	register size_t length;
{
	register char *dst = dst0;
	register const char *src = src0;
	register size_t t;

	if (length == 0 || dst == src)		/* nothing to do */
		goto done;

	/*
	 * Macros: loop-t-times; and loop-t-times, t>0
	 */
#define	TLOOP(s) if (t) TLOOP1(s)
#define	TLOOP1(s) do { s; } while (--t)

	if ((unsigned long)dst < (unsigned long)src) {
		/*
		 * Copy forward.
		 */
		t = (int)src;	/* only need low bits */
		if ((t | (int)dst) & wmask) {
			/*
			 * Try to align operands.  This cannot be done
			 * unless the low bits match.
			 */
			if ((t ^ (int)dst) & wmask || length < wsize)
				t = length;
			else
				t = wsize - (t & wmask);
			length -= t;
			TLOOP1(*dst++ = *src++);
		}
		/*
		 * Copy whole words, then mop up any trailing bytes.
		 */
		t = length / wsize;
		TLOOP(*(word *)dst = *(word *)src; src += wsize; dst += wsize);
		t = length & wmask;
		TLOOP(*dst++ = *src++);
	} else {
		/*
		 * Copy backwards.  Otherwise essentially the same.
		 * Alignment works as before, except that it takes
		 * (t&wmask) bytes to align, not wsize-(t&wmask).
		 */
		src += length;
		dst += length;
		t = (int)src;
		if ((t | (int)dst) & wmask) {
			if ((t ^ (int)dst) & wmask || length <= wsize)
				t = length;
			else
				t &= wmask;
			length -= t;
			TLOOP1(*--dst = *--src);
		}
		t = length / wsize;
		TLOOP(src -= wsize; dst -= wsize; *(word *)dst = *(word *)src);
		t = length & wmask;
		TLOOP(*--dst = *--src);
	}
done:
	return;
}

/*
 * bcmp -- vax cmpc3 instruction
 */
int
bcmp(b1, b2, length)
	const void *b1, *b2;
	register size_t length;
{
	register char *p1, *p2;

	if (length == 0)
		return(0);
	p1 = (char *)b1;
	p2 = (char *)b2;
	do
		if (*p1++ != *p2++)
			break;
	while (--length);
	return(length);
}

void
bzero(void *p, size_t size) { while (size--) *(char *)p++ = 0; }

#ifdef _DEBUG
void *
memcpy( void *dst, const void *src, size_t size ) { bcopy( src, dst, size ); return dst; }
#endif

#endif /* _EFI_EMULATION_ */

/*****************************************************************************
 *
 *  The SPL code
 *
 *****************************************************************************/

#define IPL_LOW		TPL_APPLICATION
#define IPL_NET		TPL_CALLBACK
#define IPL_HIGH	TPL_NOTIFY

#define ISR_BIT( shift )	(1 << shift)

void
splx(intrmask_t ipl)
{
	TcpIpBS->RestoreTPL( ipl );
}

intrmask_t
spl( intrmask_t NewIpl )
{
	return ((intrmask_t)TcpIpBS->RaiseTPL( NewIpl ));
}

intrmask_t
splnet( void )
{
	return spl(IPL_HIGH);
}

intrmask_t
spltty( void )
{
	return spl(IPL_HIGH);
}

intrmask_t
splimp( void )
{
	return spl(IPL_HIGH);
}

intrmask_t
splhigh( void )
{
	return spl(IPL_HIGH);
}

/*****************************************************************************
 *
 *  The source of the following tsleep/wakeup routines came from
 *  kern/kern_malloc.c and vm/vm_kern.c
 *
 *****************************************************************************/

int	PageCount = 0;

typedef struct {
	char	*FirstPage;
	char	*NextPage;
	int	FreePages;
	int	TotalPages;
} map_t;

map_t	*mb_map;
int	mb_map_full;

map_t *
kmem_suballoc( void *parent, void **min, void **max, unsigned long size )
{
	EFI_PHYSICAL_ADDRESS	FirstPage;
	int			Pages = ((size-1) / PAGE_SIZE) + 1;

	mb_map = (map_t*) malloc(sizeof( map_t), 0, 0);
	if ( mb_map == NULL )
		return ( NULL );

	if ( TcpIpBS->AllocatePages(
			AllocateAnyPages,
			EfiBootServicesData,
			Pages,
			&FirstPage ) != EFI_SUCCESS ) {
		return ( NULL );
	}

	*min = (void*)FirstPage;
	*max = (void*)(FirstPage + size);

	mb_map->FirstPage  = (char*) FirstPage;
	mb_map->NextPage   = (char*) FirstPage;
	mb_map->FreePages  = Pages;
	mb_map->TotalPages = Pages;

	return ( mb_map );
}

void*
kmem_malloc(map_t *map, unsigned long size, int flags)
{
	int	Pages = ((size-1) / PAGE_SIZE) + 1;
	void	*p;

	if ( Pages > map->FreePages )
		return ( NULL );

	p = map->NextPage;
	map->NextPage += (Pages * PAGE_SIZE);
	map->FreePages -= Pages;

	return ( p );
}

void*
kmem_alloc(void *map, unsigned long size )
{
	return ( kmem_malloc( map, size, 0 ) );
}

/*****************************************************************************
 *
 *  The source of the following tsleep/wakeup routines came from
 *  kern/kern_synch.c
 *
 *****************************************************************************/

/*
 *  Our little private sleep structure
 */
struct SleepEntry {
	TAILQ_ENTRY(SleepEntry) p_sleepq;	/* run/sleep queue. */
	void			*p_wchan;
	char			*p_mesg;
	EFI_EVENT		p_event[ 2 ];
};

/*
 * We're only looking at 7 bits of the address; everything is
 * aligned to 4, lots of things are aligned to greater powers
 * of 2.  Shift right by 8, i.e. drop the bottom 256 worth.
 */
#define TABLESIZE	32
static TAILQ_HEAD(slpquehead, SleepEntry) slpque[TABLESIZE];
#define LOOKUP(x)	(((intptr_t)(x) >> 8) & (TABLESIZE - 1))
UINTN  SleepFlags=0;


void
sleepinit()
{
	int i;

	for (i = 0; i < TABLESIZE; i++)
		TAILQ_INIT(&slpque[i]);
}

VOID
EFIAPI
NullFunction(
IN EFI_EVENT Event, 
IN VOID *Context
)
{
  //
  //   none 
  //  
  SleepFlags=1;
}

int
tsleep(ident, priority, wmesg, timo)
	void *ident;
	int priority, timo;	/* in ticks */
	const char *wmesg;
{
	struct SleepEntry	*p;
	EFI_STATUS		status;
	int			error = 0;
	int			entryspl;
	UINTN			index;
	EFI_STATUS              Status;
	int                     flags = 0;
	uint64_t	        timeout = 0;

	/*
	 *  Get a sleep entry
	 */
	p = (struct SleepEntry*) malloc(sizeof(struct SleepEntry), 0, 0);
	if ( p == NULL )
		return (ENOMEM);

	/*
	 *  Create a sleep event
	 */
	 
	SleepFlags = 0; 
	status = TcpIpBS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, NullFunction, NULL, &p->p_event[0]);
	if ( EFI_ERROR( status ) ) {
		free( p, 0 );
		return (ENOMEM);
	}

	/*
	 *  To prevent race conditions, we use are own sleep event instead of
	 *  calling the timeout() function.
	 */
	if ( timo ) {
		status = TcpIpBS->CreateEvent(EVT_TIMER, TPL_CALLBACK, NULL, NULL, &p->p_event[1]);
		if ( EFI_ERROR( status ) ) {
			error = (ENOMEM);
			goto Error2;
		}
		status = TcpIpBS->SetTimer(p->p_event[1], TimerRelative, TickToEfiTimerVal( timo ));
		if ( EFI_ERROR( status ) ) {
			error = (ENOMEM);
			goto Error1;
		}
	}
	else  {
		flags = 1;
		timo = 1;
                timeout = LIBC_MultU64x32(10000000, 180);
		status = TcpIpBS->CreateEvent(EVT_TIMER, TPL_CALLBACK, NULL, NULL, &p->p_event[1]);
		if ( EFI_ERROR( status ) ) {
			timo = 0;
			flags = 0;
			goto nextstep;
		}
		status = TcpIpBS->SetTimer(p->p_event[1], TimerRelative, timeout );
		if ( EFI_ERROR( status ) ) {
			timo = 0;
			flags = 0;
			TcpIpBS->CloseEvent(p->p_event[1]);
			goto nextstep;
		}
	}
		

	/*
	 *  We've got the needed resources so queue the sleep entry
	 */

nextstep:
	p->p_wchan = ident;
	p->p_mesg  = (char*)wmesg;

	/*
	 *  Get the current TPL so we are notified at the same level
	 */
	entryspl = splhigh();
	TAILQ_INSERT_TAIL(&slpque[LOOKUP(ident)], p, p_sleepq);

	/*
	 *  XXX folks like sbwait() sleep with raised spl.  This can cause
	 *      the netisr stuff to fail. Besides, WaitForEvent must be called
	 *	at TPL_APPLICATION anyway.
	 */
	splx(TPL_APPLICATION);

	/*
	 *  Wait for the channel to be signaled
	 */
	 
		
   	 for(;;) {
        
            if ( timo ) {

		  Status = TcpIpBS->CheckEvent(p->p_event[1]);

                if (!EFI_ERROR(Status)) {
                    index =1 ;
                    break;
                }

                if (Status != EFI_NOT_READY) {
                    panic("Error waiting for tsleep event timer\n");
                }
             }
	     if ( SleepFlags== 1 ) {
		      index = 0 ;
		      break;
	     }		  
	}

	/*
	 *  Raise it back to the level that we entered on
	 */
	spl(entryspl);

	/*
	 *  See if we timed out
	 */
	if ( index == 1 ) {
		int s;
		s = splhigh();
		if (p->p_wchan) {
			TAILQ_REMOVE(&slpque[LOOKUP(p->p_wchan)], p, p_sleepq);
			p->p_wchan = 0;
		}
		splx(s);
		error = EWOULDBLOCK;
	} else if ( timo ) {
		/*
		 *  Cancel timeout
		 */
		status = TcpIpBS->SetTimer(p->p_event[1], TimerCancel, 0);
		if ( EFI_ERROR( status ) )
			panic("Error canceling tsleep timer\n");
	}

	/*
	 *  Free resources and return
	 */
Error1:
	if ( timo )
		TcpIpBS->CloseEvent(p->p_event[1]);
Error2:
	TcpIpBS->CloseEvent(p->p_event[0]);
	free( p, 0 );

	return (error);
}

/*
 * Make all processes sleeping on the specified identifier runnable.
 */
void
wakeup(ident)
	register void *ident;
{
	register struct slpquehead *qp;
	register struct SleepEntry *p;
	int s;

	s = splhigh();
	qp = &slpque[LOOKUP(ident)];

restart:
	for (p = qp->tqh_first; p != NULL; p = p->p_sleepq.tqe_next) {
		if (p->p_wchan == ident) {
			TAILQ_REMOVE(qp, p, p_sleepq);
			p->p_wchan = 0;
			if ( TcpIpBS->SignalEvent(p->p_event[0]) != EFI_SUCCESS )
				panic("Error signaling in wakeup()\n");
			goto restart;
		}
	}
	splx(s);
}

/*
 * Make the first process sleeping on the specified identifier runnable.
 */
void
wakeup_one(ident)
	register void *ident;
{
	register struct slpquehead *qp;
	register struct SleepEntry *p;
	int s;

	s = splhigh();
	qp = &slpque[LOOKUP(ident)];

	for (p = qp->tqh_first; p != NULL; p = p->p_sleepq.tqe_next) {
		if (p->p_wchan == ident) {
			TAILQ_REMOVE(qp, p, p_sleepq);
			p->p_wchan = 0;
			if ( TcpIpBS->SignalEvent(p->p_event[0]) != EFI_SUCCESS )
				panic("Error signaling in wakeup()\n");
			break;
		}
	}
	splx(s);
}

#ifdef PPP_SUPPORT
#include <sys/tty.h>
void
ttwakeup( struct tty *tp )
{
	if (tp->t_pgrp)
		TcpIpBS->SignalEvent(*((EFI_EVENT*)tp->t_pgrp));
}
#endif

/*****************************************************************************
 *
 *  The following source came from kern/kern_clock.c
 *
 *****************************************************************************/
/*
 * Number of timecounters used to implement stable storage
 */
#define NTIMECOUNTER	2

int	tickdelta = 0;			/* current clock skew, us. per tick */
long	timedelta = 0;			/* unapplied time correction, us. */
struct	timeval boottime;
time_t	time_second;
int	ticks;

static unsigned 
dummy_get_timecount(struct timecounter *tc)
{
	static unsigned now;
	return (++now);
}

static struct timecounter dummy_timecounter = {
	dummy_get_timecount,
	0,
	~0u,
	1000000,
	"dummy"
};

struct timecounter *timecounter = &dummy_timecounter;

static void
tco_setscales(struct timecounter *tc)
{
	u_int64_t scale;

	// scale = 1000000000i64 << 32;
  scale = LIBC_LShiftU64( 1000000000ll, 32 );
	scale += tc->tc_adjustment;
	scale =LIBC_DivU64x32( scale, tc->tc_tweak->tc_frequency, NULL);
	tc->tc_scale_micro = (u_int32_t)LIBC_DivU64x32(scale , 1000, NULL);
	tc->tc_scale_nano_f = (u_int32_t)(scale & 0xffffffff);
	//tc->tc_scale_nano_i = (u_int32_t)(scale >> 32);
	tc->tc_scale_nano_i = (u_int32_t)(LIBC_RShiftU64(scale, 32));
}

static __inline unsigned
tco_delta(struct timecounter *tc)
{
	return ((tc->tc_get_timecount(tc) - tc->tc_offset_count) & 
	    tc->tc_counter_mask);
}

static struct timecounter *
sync_other_counter(void)
{
	struct timecounter *tc, *tcn, *tco;
	unsigned delta;

	tco = timecounter;
	tc = tco->tc_other;
	tcn = tc->tc_other;
	*tc = *tco;
	tc->tc_other = tcn;
	delta = tco_delta(tc);
	tc->tc_offset_count += delta;
	tc->tc_offset_count &= tc->tc_counter_mask;

	tc->tc_offset_nano += LIBC_MultU64x32((u_int64_t)delta , tc->tc_scale_nano_f);
	tc->tc_offset_nano += LIBC_LShiftU64(LIBC_MultU64x32((u_int64_t)delta , tc->tc_scale_nano_i) , 32);
	return (tc);
}

static void
tco_forward(int force)
{
	struct timecounter *tc, *tco;

	tco = timecounter;
	tc = sync_other_counter();
	/*
	 * We may be inducing a tiny error here, the tc_poll_pps() may
	 * process a latched count which happens after the tco_delta()
	 * in sync_other_counter(), which would extend the previous
	 * counters parameters into the domain of this new one.
	 * Since the timewindow is very small for this, the error is
	 * going to be only a few weenieseconds (as Dave Mills would
	 * say), so lets just not talk more about it, OK ?
	 */
	if (tco->tc_poll_pps) 
		tco->tc_poll_pps(tco);
	if (timedelta != 0) {
		tc->tc_offset_nano += LIBC_LShiftU64((u_int64_t)(tickdelta * 1000) ,32);
		timedelta -= tickdelta;
		force++;
	}

	while (tc->tc_offset_nano >= 1000000000ull << 32) {
		tc->tc_offset_nano -= 1000000000ull << 32;
		tc->tc_offset_sec++;
		tco_setscales(tc);
		force++;
	}

	if (!force)
		return;

	tc->tc_offset_micro = (u_int32_t)(LIBC_RShiftU64((LIBC_DivU64x32( tc->tc_offset_nano , 1000, NULL)) , 32));

	/* Figure out the wall-clock time */
	tc->tc_nanotime.tv_sec = tc->tc_offset_sec + boottime.tv_sec;
	tc->tc_nanotime.tv_nsec = (long)
	    (LIBC_RShiftU64(tc->tc_offset_nano , 32) + LIBC_MultU64x32(boottime.tv_usec , 1000));
	tc->tc_microtime.tv_usec = tc->tc_offset_micro + boottime.tv_usec;
	if (tc->tc_nanotime.tv_nsec >= 1000000000) {
		tc->tc_nanotime.tv_nsec -= 1000000000;
		tc->tc_microtime.tv_usec -= 1000000;
		tc->tc_nanotime.tv_sec++;
	}
	time_second = tc->tc_microtime.tv_sec = tc->tc_nanotime.tv_sec;

	timecounter = tc;
}

void
init_timecounter(struct timecounter *tc)
{
	struct timespec ts1;
	struct timecounter *t1, *t2, *t3;
	int i;

	tc->tc_adjustment = 0;
	tc->tc_tweak = tc;
	tco_setscales(tc);
	tc->tc_offset_count = tc->tc_get_timecount(tc);
	MALLOC(t1, struct timecounter *, sizeof *t1, 0, 0);
	*t1 = *tc;
	t2 = t1;
	for (i = 1; i < NTIMECOUNTER; i++) {
		MALLOC(t3, struct timecounter *, sizeof *t3, 0, 0);
		*t3 = *tc;
		t3->tc_other = t2;
		t2 = t3;
	}
	t1->tc_other = t3;
	tc = t1;

	printf("Timecounter \"%s\"  frequency %lu Hz\n", 
	    tc->tc_name, (u_long)tc->tc_frequency);

	/* XXX: For now always start using the counter. */
	tc->tc_offset_count = tc->tc_get_timecount(tc);
	nanouptime(&ts1);
	// tc->tc_offset_nano = (u_int64_t)ts1.tv_nsec << 32;
	tc->tc_offset_nano = (u_int64_t)LIBC_LShiftU64(ts1.tv_nsec ,32);
	tc->tc_offset_micro = ts1.tv_nsec / 1000;
	tc->tc_offset_sec = ts1.tv_sec;
	timecounter = tc;
}

/*
 * The real-time timer, interrupting hz times per second.
 */
void
hardclock(frame)
	register struct clockframe *frame;
{
	tco_forward(0);
	ticks++;

	/*
	 * Process callouts at a very low cpu priority, so we don't keep the
	 * relatively high clock interrupt priority any longer than necessary.
	 */
	if ( TAILQ_FIRST(&callwheel[ ticks & callwheelmask ] ) != NULL) {
		softclock();
	} else if (softticks + 1 == ticks)
		++softticks;
}

/*
 * Compute number of ticks in the specified amount of time.
 */
int
tvtohz(tv)
	struct timeval *tv;
{
	register unsigned long ticks;
	register long sec, usec;

	/*
	 * If the number of usecs in the whole seconds part of the time
	 * difference fits in a long, then the total number of usecs will
	 * fit in an unsigned long.  Compute the total and convert it to
	 * ticks, rounding up and adding 1 to allow for the current tick
	 * to expire.  Rounding also depends on unsigned long arithmetic
	 * to avoid overflow.
	 *
	 * Otherwise, if the number of ticks in the whole seconds part of
	 * the time difference fits in a long, then convert the parts to
	 * ticks separately and add, using similar rounding methods and
	 * overflow avoidance.  This method would work in the previous
	 * case but it is slightly slower and assumes that hz is integral.
	 *
	 * Otherwise, round the time difference down to the maximum
	 * representable value.
	 *
	 * If ints have 32 bits, then the maximum value for any timeout in
	 * 10ms ticks is 248 days.
	 */
	sec = tv->tv_sec;
	usec = tv->tv_usec;
	if (usec < 0) {
		sec--;
		usec += 1000000;
	}
	if (sec < 0)
		ticks = 1;
	else if (sec <= LONG_MAX / 1000000)
		ticks = (sec * 1000000 + (unsigned long)usec + (tick - 1))
			/ tick + 1;
	else if (sec <= LONG_MAX / hz)
		ticks = sec * hz
			+ ((unsigned long)usec + (tick - 1)) / tick + 1;
	else
		ticks = LONG_MAX;
	if (ticks > INT_MAX)
		ticks = INT_MAX;
	return ((int)ticks);
}

void
getmicrotime(struct timeval *tvp)
{
	struct timecounter *tc;

	tc = timecounter;
	*tvp = tc->tc_microtime;
}

void
microtime(struct timeval *tv)
{
	struct timecounter *tc;

	tc = (struct timecounter *)timecounter;
	tv->tv_sec = tc->tc_offset_sec;
	tv->tv_usec = tc->tc_offset_micro;
	tv->tv_usec += (long)LIBC_RShiftU64((tco_delta(tc)* tc->tc_scale_micro), 32);
	tv->tv_usec += boottime.tv_usec;
	tv->tv_sec += boottime.tv_sec;
	while (tv->tv_usec >= 1000000) {
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
}

void
getmicrouptime(struct timeval *tvp)
{
	struct timecounter *tc;

	tc = timecounter;
	tvp->tv_sec = tc->tc_offset_sec;
	tvp->tv_usec = tc->tc_offset_micro;
}

void
microuptime(struct timeval *tv)
{
	struct timecounter *tc;

	tc = (struct timecounter *)timecounter;
	tv->tv_sec = tc->tc_offset_sec;
	tv->tv_usec = tc->tc_offset_micro;
	//tv->tv_usec += (long)(((u_int64_t)tco_delta(tc) * tc->tc_scale_micro) >> 32);
	tv->tv_usec += (long)( LIBC_RShiftU64( LIBC_MultU64x32((u_int64_t)tco_delta(tc), tc->tc_scale_micro), 32 ));
  
	if (tv->tv_usec >= 1000000) {
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
}

void
nanouptime(struct timespec *ts)
{
	unsigned count;
	u_int64_t delta;
	struct timecounter *tc;

	tc = (struct timecounter *)timecounter;
	ts->tv_sec = tc->tc_offset_sec;
	count = tco_delta(tc);
	delta = tc->tc_offset_nano;
	delta += LIBC_MultU64x32((u_int64_t)count , tc->tc_scale_nano_f);
	//delta >>= 32;
	LIBC_RShiftU64( delta, 32 );
	delta += LIBC_MultU64x32((u_int64_t)count , tc->tc_scale_nano_i);
	if (delta >= 1000000000) {
		delta -= 1000000000;
		ts->tv_sec++;
	}
	ts->tv_nsec = (long)delta;
}

void
CalloutInit()
{
	int i;

	/*
	 * Calculate callout wheel size
	 */
	for (callwheelsize = 1, callwheelbits = 0;
	     callwheelsize < ncallout;
	     callwheelsize <<= 1, ++callwheelbits)
		;
	callwheelmask = callwheelsize - 1;

	/*
	 * Initialize callouts
	 */
	callout = (struct callout*)malloc(sizeof(struct callout) * ncallout, 0, 0);
	if ( callout == NULL ) {
		panic ("Can't allocate callout table\n");
	}
	callwheel = (struct callout_tailq*)malloc(sizeof(struct callout_tailq) * callwheelsize, 0, 0);
	if ( callwheel == NULL ) {
		panic("Can't allocate callwheel\n");
	}

	SLIST_INIT (&callfree);
	for (i = 0; i < ncallout; i++) {
		SLIST_INSERT_HEAD(&callfree, &callout[i], c_links.sle);
	}

	for (i = 0; i < callwheelsize; i++) {
		TAILQ_INIT(&callwheel[i]);
	}

	/*
	 *  Initialize global counters
	 */
	time_second = ticks = softticks = 0;
}

unsigned
EfiNet_get_timecount( struct timecounter *tc )

{
	tc->tc_offset_sec = ticks / hz;
	return (tc->tc_offset_count + 1);
};

/*
 *  EfiNet_timecounter - FreeBSD timecounter structure used for network stack.
 */
static struct timecounter EfiNet_timecounter = {
	EfiNet_get_timecount,	// get_timecount
	0,						// no poll_pps
 	~0u,					// counter_mask
	0,						// frequency
	"TcpIpv4"				// name
};

void
TimerProc( EFI_EVENT Event, void *Context )
{
	/*
	 *  Do tick related activity
	 */
	hardclock( NULL );
}

static	EFI_EVENT	HardClockEvent;

EFI_STATUS
StartTimer()

{
	EFI_STATUS	Status;

	/*
	 *  Initialize counters
	 */
	EfiNet_timecounter.tc_frequency = hz;
	init_timecounter (&EfiNet_timecounter);

	/*
	 *  Put some time on the clock.  Starting from zero breaks arp
	 */
	ticks = hz * 10;
	time_second = 10;

	/*
	 *  Initialize sleep queue
	 */
	sleepinit ();

	/*
	 *  Create and EFI event for timer operations
	 */
	Status = TcpIpBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL,
								   TPL_NOTIFY,
								   TimerProc,
								   NULL,
								   &HardClockEvent);
	if ( EFI_ERROR( Status ) ) {
		return ( Status );
	}

	/*
	 * Set the timer
	 */
	return ( TcpIpBS->SetTimer (HardClockEvent,
								TimerPeriodic,
								( TickToEfiTimerVal (1)/2 )));
}

EFI_STATUS
StopTimer()
{
	return (TcpIpBS->SetTimer (HardClockEvent, TimerCancel, 0));
}

static EFI_EVENT	CalibrateHzEvent;
static int			_CalibrateTics;

static void
_CalibrateProc( EFI_EVENT Event, void *Context )
{
	_CalibrateTics++;
}

void
CalibrateHz()
{
	EFI_STATUS	Status;

	_CalibrateTics = 0;

	/*
	 *  Get timer calibrate event
	 */
	Status = TcpIpBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, _CalibrateProc, NULL, &CalibrateHzEvent);
	if ( EFI_ERROR( Status ) ) {
		printf("Error %p getting hz calibrate event\n", (void*)Status);
		return;
	}

	/*
	 *  Create a 1msec periodic timer - we'll see what is really is later
	 */
	Status = TcpIpBS->SetTimer (CalibrateHzEvent, TimerPeriodic, 1);
	if ( EFI_ERROR( Status ) ) {
		printf("Error %p setting calibrate timer\n", (void*)Status);
		return;
	}

	/*
	 *  Wait for calibration ticks to accumulate then stop the timer
	 */
	TcpIpBS->Stall (5000000);	// 5 seconds
	Status = TcpIpBS->SetTimer (CalibrateHzEvent, TimerCancel, 0); // 1 msec
	if ( EFI_ERROR( Status ) ) {
		printf("Error %p canceling calibrate timer\n", (void*)Status);
	}
	TcpIpBS->CloseEvent (CalibrateHzEvent);

	/*
	 *  If we got any ticks, determine how many per second an set hz global.
	 */
	if ( _CalibrateTics ) {
		hz = _CalibrateTics / 5;
	} else {
		hz = 1;
		printf("Platform does not support timer tick!!! - dummy hz = %d\n", hz);
	}
	/*
	 *  Set true tick value now that we know it.
	 */
	tick = 1000000 / hz;
}

/*****************************************************************************
 *
 *  The source of pgsigio() can be found in kern/kern_sig.c
 *
 *****************************************************************************/

void
pgsigio( EFI_EVENT sigio, int signum, int checkctty )
{
	if ( TcpIpBS->SignalEvent( sigio ) != EFI_SUCCESS )
		printf("pgsigio failed\n");
}

/*****************************************************************************
 *
 *  The source of suser() can be found in kern/kern_prot.c
 *
 *****************************************************************************/

suser(struct ucred *cred, u_short *acflag)
{
	return (0);
}

/*****************************************************************************
 *
 *  The source of uiomove() can be found in kern/kern_subr.c
 *
 *****************************************************************************/

int
uiomove(char *cp, int n, struct uio *uio)
{
	register struct iovec *iov;
	size_t cnt;

	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;

		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > (size_t)n)
			cnt = n;

		if (uio->uio_rw == UIO_READ)
			bcopy((caddr_t)cp, iov->iov_base, cnt);
		else
			bcopy(iov->iov_base, (caddr_t)cp, cnt);

		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= (int)cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= (int)cnt;
	}
	return (0);
}

/*
 * General routine to allocate a hash table.
 */
void *
hashinit(elements, type, hashmask)
	int elements;
	struct malloc_type *type;
	u_long *hashmask;
{
	long hashsize;
	LIST_HEAD(generic, generic) *hashtbl;
	int i;

	if (elements <= 0)
		panic("hashinit: bad elements");
	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;
	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type, M_WAITOK);
	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*hashmask = hashsize - 1;
	return (hashtbl);
}

void 
ovbcopy( const void *src, void *dst, size_t size) { bcopy( src, (void*)dst, size ); }

