/*
 * Copyright (c) 1997, 1998 John S. Dyson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice immediately at the beginning of the file, without modification,
 *	this list of conditions, and the following disclaimer.
 * 2. Absolutely no warranty of function or purpose is made by the author
 *	John S. Dyson.
 *
 * $Id: vm_zone.c,v 1.1.1.1 2006/05/30 06:17:22 hhzhou Exp $
 */

/*
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>

#ifdef _ORG_FREEBSD_
#include <sys/sysctl.h>
#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_prot.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_extern.h>
#else
vm_offset_t kmem_alloc (void*, unsigned int);
#endif

#include <vm/vm_zone.h>


#define INVARIANTS	1

static MALLOC_DEFINE(M_ZONE, "ZONE", "Zone header");

/*
 * This file comprises a very simple zone allocator.  This is used
 * in lieu of the malloc allocator, where needed or more optimal.
 *
 * Note also that the zones are type stable.  The only restriction is
 * that the first two longwords of a data structure can be changed
 * between allocations.  Any data that must be stable between allocations
 * must reside in areas after the first two longwords.
 *
 * zinit is the initialization routine.
 * zalloc, zfree, are the interrupt/lock unsafe allocation/free routines.
 * zalloci, zfreei, are the interrupt/lock safe allocation/free routines.
 */

static struct vm_zone *zlist;
static int zone_kmem_pages, zone_kern_pages, zone_kmem_kvaspace;


/*
 * Subroutine same as zinitna, except zone data structure is allocated
 * automatically by malloc.  This routine should normally be used, except
 * in certain tricky startup conditions in the VM system -- then
 * zbootinit and zinitna can be used.  Zinit is the standard zone
 * initialization call.
 */
vm_zone_t
zinit(char *name, int size, int nentries, int flags, int zalloc)
{
	vm_zone_t z;

	z = (vm_zone_t) malloc(sizeof (struct vm_zone), M_ZONE, M_NOWAIT);
	if (z == NULL)
		return NULL;

	z->zsize = (size + ZONE_ROUNDING - 1) & ~(ZONE_ROUNDING - 1);
	simple_lock_init(&z->zlock);
	z->zfreecnt = 0;
	z->ztotal = 0;
	z->zmax = 0;
	z->zname = name;
	z->znalloc = 0;
	z->zitems = NULL;

	if (zlist == 0) {
		zlist = z;
	} else {
		z->znext = zlist;
		zlist = z;
	}

	z->zmax = 0;

	if (z->zsize > PAGE_SIZE)
		z->zfreemin = 1;
	else
		z->zfreemin = PAGE_SIZE / z->zsize;

	z->zpagecount = 0;
	if (zalloc)
		z->zalloc = zalloc;
	else
		z->zalloc = 1;

	return z;
}


/*
 * Zone critical region locks.
 */
static __inline int
zlock(vm_zone_t z)
{
	int s;

	s = splhigh();
	simple_lock(&z->zlock);
	return s;
}

static __inline void
zunlock(vm_zone_t z, int s)
{
	simple_unlock(&z->zlock);
	splx(s);
}

/*
 * void *zalloc(vm_zone_t zone) --
 *	Returns an item from a specified zone.
 *
 * void zfree(vm_zone_t zone, void *item) --
 *  Frees an item back to a specified zone.
 *
 * void *zalloci(vm_zone_t zone) --
 *	Returns an item from a specified zone, interrupt safe.
 *
 * void zfreei(vm_zone_t zone, void *item) --
 *  Frees an item back to a specified zone, interrupt safe.
 *
 */

/*
 * Zone allocator/deallocator.  These are interrupt / (or potentially SMP)
 * safe.  The raw zalloc/zfree routines are in the vm_zone header file,
 * and are not interrupt safe, but are fast.
 */
void *
zalloci(vm_zone_t z)
{
	int s;
	void *item;

	s = zlock(z);
	item = _zalloc(z);
	zunlock(z, s);
	return item;
}

void
zfreei(vm_zone_t z, void *item)
{
	int s;

	s = zlock(z);
	_zfree(z, item);
	zunlock(z, s);
	return;
}

/*
 * Internal zone routine.  Not to be called from external (non vm_zone) code.
 */
void *
_zget(vm_zone_t z)
{
#ifndef	_ORG_FREEBSD_
	extern	void*	mb_map;
#endif
	int i;
	int nitems, nbytes;
	void *item;

	if (z == NULL)
		panic("zget: null zone");

	nbytes = z->zalloc * PAGE_SIZE;

	/*
	 * We can wait, so just do normal map allocation in the appropriate
	 * map.
	 */
	item = (void *) kmem_alloc(mb_map, nbytes);
	zone_kern_pages += z->zalloc;
	bzero(item, nbytes);
	nitems = nbytes / z->zsize;
	z->ztotal += nitems;

	/*
	 * Save one for immediate allocation
	 */
	if (nitems != 0) {
		nitems -= 1;
		for (i = 0; i < nitems; i++) {
			((void **) item)[0] = z->zitems;
#ifdef INVARIANTS
			((void **) item)[1] = (void *) ZENTRY_FREE;
#endif
			z->zitems = item;
			item = (char*)item + z->zsize;
		}
		z->zfreecnt += nitems;
	} else if (z->zfreecnt > 0) {
		item = z->zitems;
		z->zitems = ((void **) item)[0];
#ifdef INVARIANTS
		if (((void **) item)[1] != (void *) ZENTRY_FREE)
			zerror(ZONE_ERROR_NOTFREE);
		((void **) item)[1] = 0;
#endif
		z->zfreecnt--;
	} else {
		item = NULL;
	}

	return item;
}


#ifdef INVARIANTS
void
zerror(int error)
{
	char *msg;

	switch (error) {
	case ZONE_ERROR_INVALID:
		msg = "zone: invalid zone";
		break;
	case ZONE_ERROR_NOTFREE:
		msg = "zone: entry not free";
		break;
	case ZONE_ERROR_ALREADYFREE:
		msg = "zone: freeing free entry";
		break;
	default:
		msg = "zone: invalid error";
		break;
	}
	panic(msg);
}
#endif
