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

    atk_libc.h
    
Abstract:

    Primary interfaces to EFI-AT for libc


Revision History

--*/

#ifndef _ATK_LIBC_H_
#define _ATK_LIBC_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <efi.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syslimits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <wchar.h>

/*
 *  Globals exported to application
 */
extern  EFI_HANDLE          _LIBC_EFIImageHandle ;
extern  EFI_SYSTEM_TABLE    *_LIBC_EFISystemTable ;
extern  EFI_GUID            _LIBC_VendorGuid ;

/*
 *  Device mapping support
 */
typedef
EFI_STATUS
(*EFI_DEV_OPEN)(
    char            *FilePath,
    char            *DevName,
    int             Flags,
    mode_t          Mode,
    EFI_DEVICE_PATH *DevPath,
    EFI_GUID        *Guid,
    INT32           *fd
    ) ;

EFI_STATUS
_LIBC_MapProtocol(
    EFI_GUID        *Guid,
    EFI_DEV_OPEN    OpenFunc
    ) ;

EFI_STATUS
_LIBC_MapDevice(
    EFI_GUID        *Guid,
    EFI_DEVICE_PATH *DevPath,
    char            *DevName
    ) ;

/*
 *  File descriptor support
 */
typedef
EFI_STATUS
(*EFILIBC_READ_T) (
    OUT    VOID   *Buffer,
    IN OUT UINTN  *BufSize,
    IN     VOID   *DevSpecific
    );

typedef
EFI_STATUS
(*EFILIBC_WRITE_T) (
    OUT    VOID   *Buffer,
    IN OUT UINTN  *BufSize,
    IN     VOID   *DevSpecific
    );

typedef
EFI_STATUS
(*EFILIBC_CLOSE_T) (
    IN VOID  *DevSpecific
    );

typedef
EFI_STATUS
(*EFILIBC_LSEEK_T) (
    IN UINT64  *Position,
    IN UINT32  Whence,
    IN VOID    *DevSpecific
    );

typedef
EFI_STATUS
(*EFILIBC_FSTAT_T) (
    IN struct stat *StatBuf,
    IN VOID        *DevSpecific
    );

typedef
EFI_STATUS
(*EFILIBC_IOCTL_T) (
    IN VOID    *DevSpecific,
    IN UINT32  Request,
    va_list    ArgList
    );

typedef
EFI_STATUS
(*EFILIBC_POLL_T) (
    IN UINT32  Mask,
    IN VOID    *DevSpecific
    );

EFI_STATUS
_LIBC_AllocateNewFileDescriptor( 
    CHAR16           *FileName,
    UINT32           Flags,
    UINT32           Mode,
    BOOLEAN          IsATTy,
    VOID             *DevSpecific,
    EFILIBC_READ_T   read,
    EFILIBC_WRITE_T  write,
    EFILIBC_CLOSE_T  close,
    EFILIBC_LSEEK_T  lseek,
    EFILIBC_FSTAT_T  fstat,
    EFILIBC_IOCTL_T  ioctl,
    EFILIBC_POLL_T   poll,
    INT32            *fd
    );

INT32
_LIBC_GetOpenFileDevSpecific(
    IN  INT32 fd,
    OUT VOID  **DevSpecific
    );

VOID
_LIBC_EfiExit(
    IN    EFI_STATUS   Status,
    IN    UINTN        DataSize,
    IN    CHAR16       *ExitData
) __dead2;

/*
 *  To be used by EFI protocols/drivers to do complete cleanup but without
 *  calling EFI Exit.
 */
VOID
_LIBC_Cleanup( VOID ) __dead2;

/*
 *  LIBC initialization 
 */
int
InitializeLibC(
    IN    EFI_HANDLE       ImageHandle,
    IN    EFI_SYSTEM_TABLE *SystemTable
    );

//
// Check EFI version,  used by InitializeLibC
//
void
LibcVersionCheck(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


/*
 *  Get command line arguments from a string
 */
VOID
_LIBC_GetArgsFromString(
    OUT int     *Argc,
    OUT CHAR16  ***Argv,
    IN  CHAR16  *pCommandLine
    );

/*
 *  Get command line arguments from LoadOptions buffer.
 */
VOID
_LIBC_GetArgsFromLoadOptions(
    OUT   int      *Argc,
    OUT   CHAR16   ***Argv
	);

/*
 *  Frees memory from allocated by either
 *  _LIBC_GetArgsFromString or _LIBC_GetArgsFromLoadOptions
 */
VOID
_LIBC_FreeArgs(
    IN    CHAR16   **Argv
    );

/*
 *  Utility functions
 */
int
is_valid_addr(
    uintptr_t address,
    size_t    size
    );

/*
 *  EFI image related routines
 */
EFI_STATUS
LoadImage(
	IN	CHAR16		*Path,
	OUT	EFI_HANDLE	*ImageHandle
	);

EFI_STATUS
StartImage(
	IN	EFI_HANDLE			ImageHandle,
	IN	UINT32				LoadOptionsSize,
	IN	CHAR16				*LoadOptions,
	IN	EFI_SYSTEM_TABLE	*SystemTable,
	OUT	UINTN				*ExitDataSize,
	OUT CHAR16				**ExitData
	);

EFI_STATUS
UnloadImage(
	IN	EFI_HANDLE	ImageHandle
	);

wchar_t*
ResolveFilename(
	IN	wchar_t	*Path
	);


UINT64
LIBC_LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    );
   
UINT64
LIBC_RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    );

UINT64
LIBC_MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    );
    
UINT64
LIBC_DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    );         
    
    
    UINT64 
LIBC_MulU64x64 (
  IN UINT64 Value1, 
  IN UINT64 Value2, 
  OUT UINT64 *ResultHigh
  );
  
 UINT64
LIBC_DivU64x64 (
  IN  UINT64   Dividend,
  IN  UINT64   Divisor,
  OUT UINT64   *Remainder OPTIONAL,
  OUT UINT32   *Error
  );
  
  
  INT64
LIBC_DivS64x64 (
  IN INT64 Dividend,
  IN INT64 Divisor,
  OUT INT64 *Remainder OPTIONAL,
  OUT UINT32 *Error
  );


#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif  /* !_ATK_LIBC_H_ */
