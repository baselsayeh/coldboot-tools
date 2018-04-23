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

    map.h
    
Abstract:

   	Device and protocol mapping data structures. 


Revision History

--*/

#ifndef _MAP_H_
#define _MAP_H_

#include <efi_interface.h>
#include <atk_libc.h>
#include <sys/types.h>

#define	MAX_MAPPROTOCOL	OPEN_MAX
#define	MAX_MAPDEVICE	OPEN_MAX
#define	DFT_DEVICE		"default:"

typedef struct	_EFILIBC_MAPPROTOCOL {
	EFI_GUID		*Guid ;					/* pointer to guid associated with protocol */
	EFI_DEV_OPEN	OpenFunc ;				/* pointer to open function associated with protocol */
	BOOLEAN			UsedEntry ;				/* marks if entry is used */
} EFILIBC_MAPPROTOCOL_T ;

typedef struct	_EFILIBC_MAPDEVICE {
	EFI_DEVICE_PATH	*DevPath ;				/* pointer to EFI device path associated with guid */
	char			*DevName ;				/* pointer to device name associated with guid */
	INT32			EntryNumMapProtocol ;	/* index into EFILIBC_MAPPROTOCL table */
	BOOLEAN			UsedEntry ;				/* marks if entry is used */
} EFILIBC_MAPDEVICE_T ;

//
// map.h
//
VOID
_InitializeMapProtocolTable(
	VOID
	) ;

VOID
_InitializeMapDeviceTable(
	VOID
	) ;

VOID
_DeInitializeMapProtocolTable(
	VOID
	) ;

VOID
_DeInitializeMapDeviceTable(
	VOID
	) ;

static VOID
_RemoveMapProtocolEntry(
	UINT32			EntryNum
	) ;

static VOID
_RemoveMapDeviceEntry(
	UINT32			EntryNum
	) ;

static EFI_STATUS
_AddMapProtocolEntry(
	EFI_GUID		*Guid, 
	EFI_DEV_OPEN	OpenFunc
	) ;

static EFI_STATUS
_AddMapDeviceEntry(
	EFI_GUID		*Guid,
	EFI_DEVICE_PATH	*DevPath,
	char			*DevName
	) ;

static INT32
_FindDevNameMapDeviceEntry(
	char			*DevName	
	) ;

static INT32
_FindGuidMapProtocolEntry(
	EFI_GUID		*Guid
	) ;

EFI_STATUS
_CallDevNameOpenFunc(
	char			*FilePath,
	char			*DevName,
    int				flags, 
    mode_t          mode,
    int             *fd
	) ;

#endif /* !_MAP_H_ */
