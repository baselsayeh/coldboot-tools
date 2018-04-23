/*-
 * Copyright (c) 2007-2008
 *      Bill Paul <wpaul@windriver.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "types.h"
#include "data.h"
#include "stand.h"

/*
 * This module updates the page tables so that we can map additional
 * memory. With x86-64, paging is required when running in long mode,
 * which means we must have page tables, and those page tables consume
 * RAM, so we want to keep them small. The minimum case requires 3
 * 4K page directories: one level 4 top level directory, one level 3
 * directory, and one level 2 directory. We can set the page attribute
 * bits in the level 2 directory to use 2MB pages, which, with 512
 * page table entries, allows us to map 1GB of RAM. That's enough to
 * get started, but eventually we want more. That means we have to add
 * more 2nd level page tables to map additional RAM. To avoid trampling
 * over too much memory to begin with, we leave the startup page table
 * in place until after we've dumped the first 640K of RAM. After that,
 * we're free to scribble on it, and we can add in some more pages.
 */

extern p3_entry_t PT3[];
 
p2_entry_t * l2map = (p2_entry_t *)PAGE_TABLES;
#define MAPS	63
#define PTES	512

void mapmem (void)
{
	int		i;

	bzero ((char *)l2map, sizeof(p2_entry_t) * 512 * MAPS);

	/* Populate page tables */

	for (i = 0; i < PTES * MAPS; i++) {
		l2map[i] = (512 + i) * (2 * 1024 * 1024);
		l2map[i] |= PG_V | PG_RW | PG_PS | PG_U;
	}

	/* Add them to the level 3 page directory */

	for (i = 0; i < MAPS; i++) {
		PT3[i + 1] = (p3_entry_t)((uintptr_t)(l2map + (i * 512)));
		PT3[i + 1] |= PG_V | PG_RW | PG_U;
	}

	return;
}
