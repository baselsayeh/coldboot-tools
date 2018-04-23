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

/*
 * This code is used to intialize the page tables for long mode.
 * We have a bit of a chicken and the egg problem with it though,
 * since if we compile it with the 64-bit compiler, it will use
 * 64-bit instructions, but we can't use those until we've entered
 * long mode, which we can't do until we've set up the page tables.
 * Consequently, this code is pre-compiled with a 32-bit version
 * of gcc in order to generate 32-bit assembly. The x86-64 assembler
 * does understand 32-bit instuctions.
 */

#include "types.h"
#include "data.h"
#include "stand.h"

extern p4_entry_t PT4[];
extern p3_entry_t PT3[];
extern p2_entry_t PT2[];

void
pageinit (void)
{
	int		i;

	/*
         * Initially, we set up mappings for only the first 1GB of RAM.
	 * Each 1GB VM segment is mapped to the first 1GB of physical
	 * memory. This is enough to get us bootstrapped. Later, we'll
	 * add additional page table entries once it's safe to scribble
	 * on more memory.
	 */

	for (i = 0; i < 512; i++) {
		/*
		 * Each slot of the level 4 page table points to
		 * the same level 3 page directory
		 */

		PT4[i] = (p4_entry_t)((uintptr_t)&PT3[0]);
		PT4[i] |= PG_V | PG_RW | PG_U;

		/*
		 * Each slot of the level 3 page directories points
		 * to the same level 2 page directory
		 */

		PT3[i] = (p3_entry_t)((uintptr_t)&PT2[0]);
		PT3[i] |= PG_V | PG_RW | PG_U;

		/*
		 * The level 2 page slots are mapped with
		 * 2MB pages for 1GB.
		 */

		PT2[i] = i * (2 * 1024 * 1024);
		PT2[i] |= PG_V | PG_RW | PG_PS | PG_U;
	}

	return;
}
