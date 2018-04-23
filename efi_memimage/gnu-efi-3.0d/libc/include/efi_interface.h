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

    efi_interface.h
    
Abstract:

    Internal interfaces to EFI for libc


Revision History

--*/

#ifndef _EFI_INTERFACE_H_
#define _EFI_INTERFACE_H_

#include <atk_libc.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#define	EXCLUSION_LOCK 1
#define	EXCLUSION_FILELOCK 1

#ifdef EXCLUSION_LOCK
#define EXCLUSION_LOCK_DECL	     EFI_TPL __CurrentTPL__
#define EXCLUSION_LOCK_TAKE()    __CurrentTPL__ = _GetBootServices()->RaiseTPL( TPL_NOTIFY )
#define EXCLUSION_LOCK_RELEASE() _GetBootServices()->RestoreTPL( __CurrentTPL__ )
#else
#define EXCLUSION_LOCK_DECL
#define EXCLUSION_LOCK_TAKE()
#define EXCLUSION_LOCK_RELEASE()
#endif

//
//  Globals
//
extern EFI_GUID _LibcLoadedImageProtocol;
extern EFI_GUID _LibcFileSystemProtocol;
extern EFI_GUID _LibcDevicePathProtocol;
extern EFI_GUID _LibcFileInfoId;
extern EFI_GUID _LibcFileSystemInfoId;
#ifdef MAP_BLOCKIO_DEVICES
extern EFI_GUID _LibcBlockIoProtocol;
#endif

//
//  Prototypes
//

//
// efi_interface.c
//
UINTN
_InitializeEfiInterface( 
    IN EFI_HANDLE       ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable 
    );

EFI_BOOT_SERVICES *
_GetBootServices( 
    VOID 
    );
EFI_RUNTIME_SERVICES *
_GetRuntimeServices( 
    VOID 
    );

EFI_HANDLE
_GetImageHandle( 
    VOID 
    );

EFI_LOADED_IMAGE *
_GetLoadedImage( 
    VOID 
    );

EFI_MEMORY_TYPE *
_GetPoolAllocationType( 
    VOID 
    );

SIMPLE_INPUT_INTERFACE *
_GetConsoleIn( 
    VOID 
    );

SIMPLE_TEXT_OUTPUT_INTERFACE *
_GetConsoleOut( 
    VOID 
    );

SIMPLE_TEXT_OUTPUT_INTERFACE *
_GetConsoleErr( 
    VOID 
    );

wchar_t*
_ConvertToEfiPathname(
    const char    *AsciiPath
    );

UINT32
_GetPid(
    VOID
    );

VOID
_EfiExit(
    IN  EFI_STATUS  Status,
    IN  UINTN       DataSize,
    IN  CHAR16      *ExitData
    );

//
// lock.c
//
VOID
_SaveOldTPL( 
    IN EFI_TPL  OldTPL
    );

EFI_TPL
_GetOldTPL(
    VOID
    );

//
// stdlib/malloc.c   XXX this is in the wrong place XXX
//
VOID
_InitializeMalloc( 
    VOID 
);

VOID
_DeInitializeMalloc(
	VOID
);

//
// system/init.c   XXX this is in the wrong place XXX
//
int
_InitializeSystemIO( 
    VOID 
);

//
// efi/memory.c
//
VOID *
EFI_AllocatePool( 
    UINTN Size
    );

VOID  
EFI_FreePool( 
    VOID * ptr
    );

//
//  efi/fileio.c
//
EFI_STATUS
EFI_RenameFile( 
    IN  VOID             *DevSpecific,
    IN CHAR16            *NewName
    );

EFI_STATUS
EFI_UnlinkFile(
    IN VOID              *DevSpecific
    );

EFI_STATUS
EFI_FastFstatFile( 
    IN  struct stat      *StatBuffer,
    IN  VOID             *DevSpecific
    );

EFI_STATUS
EFI_UtimesFile( 
    IN  VOID             *DevSpecific,
    IN struct timeval    *Times
    );

//
//  efi/env.c
//
VOID
_InitializeEnv( EFI_SYSTEM_TABLE * );

char *
_GetFileMapping(
	IN  int		Index,
	OUT VOID	**DevPath
	);

int
_AddFileMapping(
	IN  char	*MapName,
	IN  void	*DevPath
	);

CHAR16 *
_GetParentCwd( VOID );

//
//  efi/time.c
//
time_t
EfiTimeToUnixTime(
	IN EFI_TIME *ETime
	);

int
EFI_GetTimeOfDay(
	OUT struct timeval *tp,
	OUT struct timezone *tzp
	);

//
//  efi/GetFileDevicePath.c
//
EFI_STATUS
_GetFileDevicePath(
	IN	CHAR16			*File,
	OUT	EFI_DEVICE_PATH	**FullDevPath
	);

#endif /* !_EFI_INTERFACE_H_ */
