/*
 * Copyright (c) 1999, 2000
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

#ifndef _ATK_MP_H_
#define _ATK_MP_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <efi.h>
#ifdef EFI64
#include <salproc.h>
#endif

#define EFI_MP_PROTOCOL \
	{ 0x2a3b8670, 0xe0c9, 0x11d3, 0x8c, 0x17, 0x0, 0xa0, 0xc9, 0x5b, 0xc2, 0x22 }

#define EFI_MPERR( val )	EFIERR_OEM( 0x10000 | ( val ) )

//
// Error codes
//
#define EFI_MP_FAILURE				EFI_MPERR(1)
#define EFI_MP_ACPI_ID_FAILURE		EFI_MPERR(2)
#define EFI_MP_AP_NOT_STOP			EFI_MPERR(3)
#define EFI_MP_AP_NOT_START			EFI_MPERR(4)
#define EFI_MP_ADDRESS_FAILURE		EFI_MPERR(5)
#define EFI_MP_ENUM_FAILURE			EFI_MPERR(6)
#define EFI_MP_WAKEUP_FAILURE		EFI_MPERR(7)
#define EFI_MP_INTHANDLER_FAILURE	EFI_MPERR(8)
#define EFI_MP_COMM_FAILURE			EFI_MPERR(9)


//
// Flag definitions
//
#define	EFI_BSP_FLAG				0x00000000
#define	EFI_AP_FLAG					0x00000001


//
// Structure definitions
//
typedef struct {
	UINT16		ACPIProcessorID ;
	UINT32		Flags ;
} EFI_MP_PROC_INFO ;


//
// Function prototypes
//
typedef
EFI_STATUS
(EFIAPI *EFI_INIT_MP_PROTOCOL) (
    IN  struct _EFI_MP_INTERFACE	*This
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_DEINIT_MP_PROTOCOL) (
    IN  struct _EFI_MP_INTERFACE	*This
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_GET_ENABLED_PROCESSORS_INFO) (
    IN  struct _EFI_MP_INTERFACE	*This,
	IN	OUT	EFI_MP_PROC_INFO		*Buffer,
	IN	OUT	UINTN					*BufferSize
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_GET_NUM_ENABLED_PROCESSORS) (
    IN  struct _EFI_MP_INTERFACE	*This,
	OUT	UINT8						*NumProcessors
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_START_PROCESSOR) (
	IN	struct _EFI_MP_INTERFACE	*This,
	IN	UINT16						ACPIProcessorID
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_STOP_PROCESSOR) (
	IN	struct _EFI_MP_INTERFACE	*This,
	IN	UINT16						ACPIProcessorID
	) ;


typedef
EFI_STATUS
(EFIAPI *EFI_START_PROCESSOR_ADDRESS) (
	IN	struct _EFI_MP_INTERFACE	*This,
	IN	UINT16						ACPIProcessorID,
	IN	VOID						*Address,
	IN	VOID						*Argument
	) ;

typedef struct _EFI_MP_INTERFACE {

	EFI_INIT_MP_PROTOCOL			InitMpProtocol ;
	EFI_DEINIT_MP_PROTOCOL			DeInitMpProtocol ;
	EFI_GET_ENABLED_PROCESSORS_INFO	GetEnabledProcessorsInfo ;
	EFI_GET_NUM_ENABLED_PROCESSORS	GetNumEnabledProcessors ;
	EFI_START_PROCESSOR				StartProcessor ;
	EFI_STOP_PROCESSOR				StopProcessor ;
	EFI_START_PROCESSOR_ADDRESS		StartProcessorAddress ;

} EFI_MP_INTERFACE ;

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif /* !_ATK_MP_H_ */
