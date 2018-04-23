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

/******************************************************************************
*
*   Revision History:
*   $Log: efiipmi.h,v $
*   Revision 1.1.1.1  2006/05/30 06:12:29  hhzhou
*   no message
*
 * 
 *    Rev 1.1   14 Jul 2000 10:51:54   eamaya
 * - Updated KCS protocol per addenda to IPMI spec.
 *   Note that this update is not backward compatible
 *   to previous KCS protocol.
 * - Added ability to unload the driver.
 * 
 *    Rev 1.0   14 Jun 2000 12:48:38   eamaya
 * - Updated to work with SMBIOS table to look up KCS
 * address in Record 38 and override default KCS address CA2.
 * - Updated revision from 1.0 to 1.1
*
******************************************************************************/

#ifndef _EFIIPMI_H_
#define _EFIIPMI_H_

#include "efi.h"

/*
 *  EFI IPMI protocol
 */

#define EFI_IPMI_PROTOCOL \
    { 0x767f1800, 0x91c2, 0x11d3, 0x93, 0x98, 0x00, 0x80, 0xc7, 0x2a, 0x36, 0xc6 }
#define EFI_IPMI_INTERFACE_REVISION   0x00010002

#define EFI_IPMIERR(val)				EFIERR_OEM(0x20000 | val)

#define EFI_IPMIERR_FAILURE				EFI_IPMIERR(1)
#define EFI_IPMIERR_TIMEOUT				EFI_IPMIERR(2)
#define EFI_IPMIERR_BMC_BUSY			EFI_IPMIERR(3)

INTERFACE_DECL(_EFI_IPMI);

/*
 *  IPMI interface
 */

typedef
EFI_STATUS
(EFIAPI *EFI_SENDMESSAGE) (
	IN	struct _EFI_IPMI	*This,
	IN	UINT8				NetFn,
	IN	UINT8				LUN,
	IN	UINT8				Cmd,
	IN	UINT8				*Data,
	IN	UINT32				DataLen
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GETMESSAGE) (
	IN		struct _EFI_IPMI	*This,
	OUT		UINT8				*NetFn,
	OUT		UINT8				*LUN,
	OUT		UINT8				*Cmd,
	OUT		UINT8				*CompletionCode,
	OUT		UINT8				*Data,
	IN OUT	UINT32				*DataLen
	);

typedef struct _EFI_IPMI {
    UINT64				Revision;
    EFI_SENDMESSAGE		SendMessage;
    EFI_GETMESSAGE		GetMessage;
} EFI_IPMI_INTERFACE;


#endif /* _EFIIPMI_H_ */
