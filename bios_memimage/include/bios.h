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

#ifndef _ASM
extern void bios_pxe (void);
extern int bios_memmap (void);
extern void bios_memmap2 (void);
extern int bios_apm_shutdown (void);
extern void bios_getgeom (uint8_t);
extern void bios_getextgeom (uint8_t);
extern void bios_reset (uint8_t);
extern uint16_t bios_extchk (uint8_t);
extern int bios_rd(uint8_t);
extern int bios_wr(uint8_t);
extern int bios_extrd(uint8_t);
extern int bios_extwr(uint8_t);
#endif

/* BIOS id */
#define APM_BIOS                0x53
#define APM_INT                 0x15

/* APM flags */
#define APM_16BIT_SUPPORT       0x01
#define APM_32BIT_SUPPORT       0x02
#define APM_CPUIDLE_SLOW        0x04
#define APM_DISABLED            0x08
#define APM_DISENGAGED          0x10

/* APM functions */
#define APM_INSTCHECK           0x00
#define APM_REALCONNECT         0x01
#define APM_PROT16CONNECT       0x02
#define APM_PROT32CONNECT       0x03
#define APM_DISCONNECT          0x04
#define APM_CPUIDLE             0x05
#define APM_CPUBUSY             0x06
#define APM_SETPWSTATE          0x07
#define APM_ENABLEDISABLEPM     0x08
#define APM_RESTOREDEFAULT      0x09
#define APM_GETPWSTATUS         0x0a
#define APM_GETPMEVENT          0x0b
#define APM_GETPWSTATE          0x0c
#define APM_ENABLEDISABLEDPM    0x0d
#define APM_DRVVERSION          0x0e
#define APM_ENGAGEDISENGAGEPM   0x0f
#define APM_GETCAPABILITIES     0x10
#define APM_RESUMETIMER         0x11
#define APM_RESUMEONRING        0x12
#define APM_TIMERREQUESTS       0x13
#define APM_OEMFUNC             0x80

/* error code */
#define APME_OK                 0x00
#define APME_PMDISABLED         0x01
#define APME_REALESTABLISHED    0x02
#define APME_NOTCONNECTED       0x03
#define APME_PROT16ESTABLISHED  0x05
#define APME_PROT16NOTSUPPORTED 0x06
#define APME_PROT32ESTABLISHED  0x07
#define APME_PROT32NOTDUPPORTED 0x08
#define APME_UNKNOWNDEVICEID    0x09
#define APME_OUTOFRANGE         0x0a
#define APME_NOTENGAGED         0x0b
#define APME_CANTENTERSTATE     0x60
#define APME_NOPMEVENT          0x80
#define APME_NOAPMPRESENT       0x86


/* device code */
#define PMDV_APMBIOS            0x0000
#define PMDV_ALLDEV             0x0001
#define PMDV_DISP0              0x0100
#define PMDV_DISP1              0x0101
#define PMDV_DISPALL            0x01ff
#define PMDV_2NDSTORAGE0        0x0200
#define PMDV_2NDSTORAGE1        0x0201
#define PMDV_2NDSTORAGE2        0x0202
#define PMDV_2NDSTORAGE3        0x0203
#define PMDV_PARALLEL0          0x0300
#define PMDV_PARALLEL1          0x0301
#define PMDV_SERIAL0            0x0400
#define PMDV_SERIAL1            0x0401
#define PMDV_SERIAL2            0x0402
#define PMDV_SERIAL3            0x0403
#define PMDV_SERIAL4            0x0404
#define PMDV_SERIAL5            0x0405
#define PMDV_SERIAL6            0x0406
#define PMDV_SERIAL7            0x0407
#define PMDV_NET0               0x0500
#define PMDV_NET1               0x0501
#define PMDV_NET2               0x0502
#define PMDV_NET3               0x0503
#define PMDV_PCMCIA0            0x0600
#define PMDV_PCMCIA1            0x0601
#define PMDV_PCMCIA2            0x0602
#define PMDV_PCMCIA3            0x0603
/* 0x0700 - 0x7fff      Reserved                        */
#define PMDV_BATT_BASE          0x8000
#define PMDV_BATT0              0x8001
#define PMDV_BATT1              0x8002
#define PMDV_BATT_ALL           0x80ff
/* 0x8100 - 0xdfff      Reserved                        */
/* 0xe000 - 0xefff      OEM-defined power device IDs    */
/* 0xf000 - 0xffff      Reserved                        */

/* Power state */
#define PMST_APMENABLED         0x0000
#define PMST_STANDBY            0x0001
#define PMST_SUSPEND            0x0002
#define PMST_OFF                0x0003
#define PMST_LASTREQNOTIFY      0x0004
#define PMST_LASTREQREJECT      0x0005
/* 0x0006 - 0x001f      Reserved system states          */
/* 0x0020 - 0x003f      OEM-defined system states       */
/* 0x0040 - 0x007f      OEM-defined device states       */
/* 0x0080 - 0xffff      Reserved device states          */

/*
 * BIOS keyboard buffer address and size
 * The BIOS keyboard buffer starts at address 0x41E and can
 * contain up to 16 keystrokes. Each keystroke is a 16-bit (2 byte)
 * keycode. The address at location 0x41A indicates the head of the
 * ring buffer and the address at location 0x41C indicates the end.
 * If the start address and end address differ, there's a keystroke
 * waiting to be read.
 */

#define BIOS_KEYBDBUF_BASE	0x41E
#define BIOS_KEYBDBUF_STARTPTR	0x41A
#define BIOS_KEYBDBUF_ENDPTR	0x41C
#define BIOS_KEYBDBUF_SIZE	32
