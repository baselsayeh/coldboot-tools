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

struct x86desc GDT[] = {
	{ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
        { 0xFFFF, 0x0000, 0x00, GDT_CFLAGS, GDT_GRAN|0xF, 0x00 },
        { 0xFFFF, 0x0000, 0x00, GDT_DFLAGS, GDT_GRAN|0xF, 0x00 },
        { 0xFFFF, 0x0000, 0x00, GDT_CFLAGS, 0x00, 0x00 },
        { 0xFFFF, 0x0000, 0x00, GDT_DFLAGS, 0x00, 0x00 },
	{ 0x0000, 0x0000, 0x00, GDT_CFLAGS, GDT_LONG, 0x00 },
	{ 0x0000, 0x0000, 0x00, SEGFLAGLO_PRESENT, 0x00, 0x00 },
};

struct gdt GDESC = { sizeof(GDT) - 1, GDT };

uint16_t IDESC[3] = { 0x0000, 0x0000, 0x0000 };

uint32_t bios_arg;
uint16_t bios_seg;
uint16_t bios_off;
uint8_t bios_disk = 0;
uint16_t bios_cylsec = 0;
uint8_t bios_hds = 0;
uint8_t bios_secs = 0;
uint32_t bios_next = 0x00000000;
uint32_t bios_sig = 0x00000000;
uint32_t bios_ext1 = 0x00000000;
uint32_t bios_ext2 = 0x00000000;
uintmax_t saved_stack = 0;
uint16_t pxe_op = 0x0000;
uint16_t * pxe_addr = 0x0000;
uint8_t bios_char = 0x00;
struct bios_smap bios_smap = { 0, 0, 0 };
