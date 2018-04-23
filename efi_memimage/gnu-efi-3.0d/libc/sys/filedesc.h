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

    filedesc.h
    
Abstract:

    File descriptor data structure


Revision History

--*/

#ifndef _FILEDESC_H_
#define _FILEDESC_H_

#include <atk_libc.h>

typedef struct _EFILIBC_FILE_DESCRIPTOR {
    CHAR16            *FileName;       /* name of the file or device */
    UINT32            Flags;           /* BSD open(2) flags argument */
    UINT32            Mode;            /* BSD open(2) mode argument */
    BOOLEAN           IsATTy;          /* True if it's a TTY (DevSpecific?) */
    int               Refs;            /* Reference count for dup(2) */
    int               Unlink;          /* Remove file on last close */
    VOID              *DevSpecific;    /* Device specific data */
    EFILIBC_READ_T    read;            /* Pointer to read function */
    EFILIBC_WRITE_T   write;           /* Pointer to write function */
    EFILIBC_CLOSE_T   close;           /* Pointer to close function */
    EFILIBC_LSEEK_T   lseek;           /* Pointer to lseek function */
    EFILIBC_FSTAT_T   fstat;           /* Pointer to fstat function */
    EFILIBC_IOCTL_T   ioctl;           /* Pointer to ioctl function */
    EFILIBC_POLL_T    poll;            /* Pointer to poll function */
} EFILIBC_FILE_DESCRIPTOR_T;

EFI_STATUS
_InitializeFileDescriptorTable(
    VOID
    );

EFI_STATUS
_ReleaseFileDescriptor(
    INT32 Fd
    );

EFI_STATUS
_ReleaseFileDescriptorSlot(
    INT32 Fd
    );

EFI_STATUS
_DupFileDescriptor(
	IN      int OldFd,
	IN OUT  int *NewFd
	);

INT32
_GetOpenFileIsATTy(
    IN  INT32   fd,
    OUT BOOLEAN *tty
    );

INT32
_GetOpenFileFlags(
    IN  INT32  fd,
    OUT UINT32 *flags
    );

INT32
_SetOpenFileFlags(
    IN  INT32  fd,
    OUT UINT32 flags
    );

INT32
_GetOpenFileRead(
    IN  INT32          fd,
    OUT EFILIBC_READ_T *read
    );

INT32
_GetOpenFileWrite(
    IN  INT32           fd,
    OUT EFILIBC_WRITE_T *write
    );

INT32
_GetOpenFileClose(
    IN  INT32           fd,
    OUT EFILIBC_CLOSE_T *close
    );

INT32
_GetOpenFileLseek(
    IN  INT32           fd,
    OUT EFILIBC_LSEEK_T *lseek
    );

INT32
_GetOpenFileFstat(
    IN  INT32           fd,
    OUT EFILIBC_FSTAT_T *fstat
    );

INT32
_GetOpenFileIoctl(
    IN  INT32           fd,
    OUT EFILIBC_IOCTL_T *ioctl
    );

INT32
_GetOpenFilePoll(
    IN  INT32           fd,
    OUT EFILIBC_POLL_T  *poll
    );

wchar_t*
_ConvertToEfiPathname(
    const char    *AsciiPath
    );

INT32
_MarkFileForUnlink(
    IN  INT32           fd
    );

INT32
_GetUnlinkFlagCount(
    IN  INT32           fd,
    OUT int             *UnlinkFlag
    );

int
_AddFileReference(
    IN  INT32           fd
    );

int
_RemoveFileReference(
    IN  INT32           fd
    );

void
_FileDescriptorTableShutdown(
    void
    );

#endif /* !_FILEDESC_H */
