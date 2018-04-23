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

    blockio.c
    
Abstract:

    Interfaces to EFI for I/O to the block devices


Revision History

--*/
#ifdef MAP_BLOCKIO_DEVICES

#include <efi_interface.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>

#include <libc_debug.h>
#include <assert.h>

//
//  Device-specific file info
//
typedef struct {
	EFI_BLOCK_IO	*BlockIo;
	EFI_LBA			CurrentLBA;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN			InUse;
#endif
} EFILIBC_BLOCKDEV_T;

//
//  Name:
//      EFI_OpenBlock
//
//  Description:
//      Open a block device for use with EFI Block I/O Protocol
//
//      Note potential confusion between BSD "mode" argument that
//      specifies the permissions on a newly created file and the
//      EFI "open mode" that specifies things analogous to the BSD
//      "flags" argument.
//
//  Arguments:
//      FileName:   name of the file to open
//      Flags:      BSD file open flags
//      Mode:       mode specifies perms for file creation (O_CREAT)
//      FileHandle: EFI_FILE_HANDLE to newly opened file
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_OpenBlock( 
	CHAR16			*FileName,
	int				Flags,
	mode_t			Mode,									
	EFI_DEVICE_PATH	*DevPath,
	VOID			**DevSpecific
	)
{
	EFI_STATUS			Status;
	EFI_HANDLE			DeviceHandle;
	EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BOOT_SERVICES	*BootServ	= _GetBootServices();
	EFI_DEVICE_PATH		*DevicePath	= DevPath;

	DPRINT((L"EFI_OpenBlock: FileName=%s Flags=%X Mode=%X DevPath=%X\n", FileName, Flags, Mode, DevPath));

	//
	//  Init
	//
	BlockDev = calloc(sizeof(EFILIBC_BLOCKDEV_T), 1);
	if (!BlockDev) {
		Status = EFI_OUT_OF_RESOURCES;
		goto Done;
	}

	BlockDev->CurrentLBA = 0;

#ifdef EXCLUSION_FILELOCK
	BlockDev->InUse = FALSE;
#endif

	*DevSpecific = BlockDev;

	//
	//  Determine device handle for blkio protocol on specified device path
	//
	DPRINT((L"EFI_OpenBlock: locate device path\n"));
	Status = BootServ->LocateDevicePath(&_LibcBlockIoProtocol, &DevicePath, &DeviceHandle);
    if (EFI_ERROR(Status)) {
		DPRINT((L"EFI_OpenBlock: error from LocateDevicePath(BlockIo)\n"));
		goto Done;
	}

	//
	//  Determine block device on device path
	//
	DPRINT((L"EFI_OpenBlock: get protocol interface\n"));
	Status = BootServ->HandleProtocol(DeviceHandle, &_LibcBlockIoProtocol, (VOID*)&(BlockDev->BlockIo));
	if (EFI_ERROR(Status)) {
		DPRINT((L"EFI_OpenBlock: error from HandleProtocol(BlockIo)\n"));
		goto Done;
	}

	DPRINT((L"EFI_OpenBlock: MediaId:          0x%X\n", BlockDev->BlockIo->Media->MediaId));
	DPRINT((L"EFI_OpenBlock: RemovableMedia:   %d\n", BlockDev->BlockIo->Media->RemovableMedia));
	DPRINT((L"EFI_OpenBlock: MediaPresent:     %d\n", BlockDev->BlockIo->Media->MediaPresent));
	DPRINT((L"EFI_OpenBlock: LogicalPartition: %d\n", BlockDev->BlockIo->Media->LogicalPartition));
	DPRINT((L"EFI_OpenBlock: ReadOnly:         %d\n", BlockDev->BlockIo->Media->ReadOnly));
	DPRINT((L"EFI_OpenBlock: WriteCaching:     %d\n", BlockDev->BlockIo->Media->WriteCaching));
	DPRINT((L"EFI_OpenBlock: BlockSize:        %d\n", BlockDev->BlockIo->Media->BlockSize));
	DPRINT((L"EFI_OpenBlock: IoAlign:          %d\n", BlockDev->BlockIo->Media->IoAlign));
	DPRINT((L"EFI_OpenBlock: LastBlock:        %d\n", BlockDev->BlockIo->Media->LastBlock));

	if (BlockDev->BlockIo->Media->ReadOnly) {
		if (((O_ACCMODE & Flags) == O_WRONLY) || ((O_ACCMODE & Flags) == O_RDWR)) {
			Status = EFI_INVALID_PARAMETER;
			DPRINT((L"EFI_OpenBlock: bad access mode combo %x\n", (O_ACCMODE & Flags)));
			goto Done;
		}
	}

Done:
	if (EFI_ERROR(Status) && *DevSpecific) {
		free(*DevSpecific);
		*DevSpecific = NULL;
	}

	DPRINT((L"EFI_OpenBlock returned: %r\n", Status));
	return Status;
}

//
//  Name:
//      EFI_CloseBlock
//
//  Description:
//      Close the specified block device
//
//  Arguments:
//      DevSpecific:  File handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_CloseBlock( 
	IN	VOID	*DevSpecific
    )
{
    EFI_STATUS			Status		 = EFI_SUCCESS;
    EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BLOCK_IO		*BlockIo;
	EFI_BLOCK_IO_MEDIA	*Media;
#ifdef EXCLUSION_FILELOCK
	EFI_BOOT_SERVICES	*BootServ	 = _GetBootServices();
	BOOLEAN				AcquiredLock = FALSE;
	EFI_TPL				CurrentTPL;
#endif

	DPRINT((L"EFI_CloseBlock\n"));

	if (!DevSpecific) {
		Status = EFI_INVALID_PARAMETER;
		goto Done;
	}

	BlockDev = DevSpecific;
	BlockIo = BlockDev->BlockIo;
	Media = BlockIo->Media;

#ifdef EXCLUSION_FILELOCK
	DPRINT((L"EFI_CloseBlock: Raising TPL\n"));
	CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);

	// Check whether in use
	if (BlockDev->InUse == TRUE) {
		Status = EFI_NOT_READY;
		DPRINT((L"EFI_CloseBlock: Busy, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL) ;
		goto Done ;
	}
	else {
		BlockDev->InUse = TRUE;
		AcquiredLock = TRUE;
		DPRINT((L"EFI_CloseBlock: Locking, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	if (!Media->ReadOnly && Media->WriteCaching) {
		DPRINT((L"EFI_CloseBlock: Flushing\n"));
		Status = BlockIo->FlushBlocks(BlockIo);
	}

	free(BlockDev);

Done:
#ifdef EXCLUSION_FILELOCK
	if (AcquiredLock == TRUE) {
		DPRINT((L"EFI_CloseBlock: Unlocking\n"));
		CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY) ;
		BlockDev->InUse = FALSE;
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	DPRINT((L"EFI_CloseBlock returned: %r\n", Status));
	return Status;
}

//
//  Name:
//      EFI_ReadBlock
//
//  Description:
//      Read a block device using EFI Block I/O Protocol
//
//  Arguments:
//      BufSize:     on input, size of Buffer; on output, number of bytes read
//      Buffer:      pointer to data read
//      DevSpecific: file handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_ReadBlock( 
	IN OUT	VOID	*Buffer,
	IN OUT	UINTN	*BufferSize,
	IN OUT	VOID	*DevSpecific
	)
{
	EFI_STATUS			Status;
	EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BLOCK_IO		*BlockIo;
	EFI_BLOCK_IO_MEDIA	*Media;
	EFI_LBA				NumReqBlocks;
	EFI_LBA				LastReqBlock;
#ifdef EXCLUSION_FILELOCK
	EFI_BOOT_SERVICES	*BootServ	 = _GetBootServices();
	BOOLEAN				AcquiredLock = FALSE;
	EFI_TPL				CurrentTPL ;
#endif

	DPRINT((L"EFI_ReadBlock: *BufferSize %d\n", *BufferSize));

	if (!DevSpecific) {
		Status = EFI_INVALID_PARAMETER;
		goto Done;
	}

	BlockDev = DevSpecific;
	BlockIo = BlockDev->BlockIo;
	Media = BlockIo->Media;

	//
	// BufferSize must be a multiple of the intrinsic block size of the device.
	//
	if ((*BufferSize % Media->BlockSize) != 0) {
		Status = EFI_INVALID_PARAMETER;
		DPRINT((L"EFI_ReadBlock: *BufferSize not multiple of device block size %d\n",
				Media->BlockSize));
		goto Done;
	}

	//
	// If read request would go beyond the end of the block device,
	// reduce the buffer size so it fits.
	//
	NumReqBlocks = *BufferSize / Media->BlockSize;
	LastReqBlock = BlockDev->CurrentLBA + (NumReqBlocks - 1);
	if (LastReqBlock > Media->LastBlock) {
		*BufferSize -= (UINTN)(LastReqBlock - Media->LastBlock) * Media->BlockSize;
		if (*BufferSize == 0) {
			Status = EFI_SUCCESS;
			goto Done;
		}
	}

#ifdef EXCLUSION_FILELOCK
	DPRINT((L"EFI_ReadBlock: Raising TPL\n"));
	CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);

	// Check whether in use
	if (BlockDev->InUse == TRUE) {
		Status = EFI_NOT_READY ;
		DPRINT((L"EFI_ReadBlock: Busy, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL) ;
		goto Done ;
	}
	else {
		BlockDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT((L"EFI_ReadBlock: Locking, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL) ;
	}
#endif
	Status = BlockIo->ReadBlocks(BlockIo,
									Media->MediaId,
									BlockDev->CurrentLBA,
									*BufferSize,
									Buffer);

    if (EFI_ERROR(Status)) {
		goto Done;
	}

	BlockDev->CurrentLBA += *BufferSize / Media->BlockSize;

Done:
#ifdef EXCLUSION_FILELOCK
	if (AcquiredLock == TRUE) {
		DPRINT((L"EFI_ReadBlock: Unlocking\n"));
		CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY) ;
		BlockDev->InUse = FALSE;
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	DPRINT((L"EFI_ReadBlock returned: %r\n", Status));
	return Status;
}

//
//  Name:
//      EFI_WriteBlock
//
//  Description:
//      Write a file using EFI File System Protocol
//
//  Arguments:
//      BufSize:     on input, size of Buffer; on output, number bytes written
//      Buffer:      pointer to data to be written
//      DevSpecific: file handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_WriteBlock( 
	IN OUT	VOID	*Buffer,
	IN OUT	UINTN	*BufferSize,
	IN OUT	VOID	*DevSpecific
	)
{
	EFI_STATUS			Status;
	EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BLOCK_IO		*BlockIo;
	EFI_BLOCK_IO_MEDIA	*Media;
	EFI_LBA				NumReqBlocks;
	EFI_LBA				LastReqBlock;
#ifdef EXCLUSION_FILELOCK
	EFI_BOOT_SERVICES	*BootServ	 = _GetBootServices();
	BOOLEAN				AcquiredLock = FALSE ;
	EFI_TPL				CurrentTPL;
#endif

	DPRINT((L"EFI_WriteBlock: *BufferSize %d\n", *BufferSize));

	if (!DevSpecific) {
		Status = EFI_INVALID_PARAMETER;
		goto Done;
	}

	BlockDev = DevSpecific;
	BlockIo = BlockDev->BlockIo;
	Media = BlockIo->Media;

	//
	// BufferSize must be a multiple of the intrinsic block size of the device.
	//
	if ((*BufferSize % Media->BlockSize) != 0) {
		Status = EFI_INVALID_PARAMETER;
		DPRINT((L"EFI_WriteBlock: *BufferSize not multiple of device block size %d\n",
				Media->BlockSize));
		goto Done;
	}

	//
	// If write request would go beyond the end of the block device, error.
	//
	NumReqBlocks = *BufferSize / Media->BlockSize;
	LastReqBlock = BlockDev->CurrentLBA + (NumReqBlocks - 1);
	if (LastReqBlock > Media->LastBlock) {
		Status = EFI_INVALID_PARAMETER;
		DPRINT((L"EFI_WriteBlock: *BufferSize goes beyond end of block device\n"));
		goto Done;
	}

#ifdef EXCLUSION_FILELOCK
	DPRINT((L"EFI_WriteBlock: Raising TPL\n"));

	CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);

	// Check whether in use
	if (BlockDev->InUse == TRUE) {
		Status = EFI_NOT_READY;
		DPRINT((L"EFI_WriteBlock: Busy, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
		goto Done ;
	}
	else {
		BlockDev->InUse = TRUE;
		AcquiredLock = TRUE;
		DPRINT((L"EFI_WriteBlock: Locking, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	Status = BlockIo->WriteBlocks(BlockIo,
									Media->MediaId,
									BlockDev->CurrentLBA,
									*BufferSize,
									Buffer);

    if (EFI_ERROR(Status)) {
		goto Done;
	}

	BlockDev->CurrentLBA += *BufferSize / Media->BlockSize;

Done:
#ifdef EXCLUSION_FILELOCK
	if (AcquiredLock == TRUE) {
		DPRINT((L"EFI_WriteBlock: Unlocking\n"));
		CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);
		BlockDev->InUse = FALSE;
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	DPRINT((L"EFI_WriteBlock returned: %r\n", Status));
    return Status;
}

//
//  Name:
//      EFI_SeekBlock
//
//  Description:
//      Seek to a position in a file using EFI File System Protocol
//
//  Arguments:
//      Position:    the byte position from the start of the file to set
//      Whence:      how to interpret Position arg
//      DevSpecific: file handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_SeekBlock( 
	IN OUT	UINT64	*Position,
	IN		UINT32	whence,
	IN OUT	VOID	*DevSpecific
	)
{
	EFI_STATUS			Status = EFI_SUCCESS;
	EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BLOCK_IO_MEDIA	*Media;
	EFI_LBA				SeekReqBlock;
	UINTN			temp;
#ifdef EXCLUSION_FILELOCK
	EFI_BOOT_SERVICES	*BootServ	 = _GetBootServices();
	BOOLEAN				AcquiredLock = FALSE;
	EFI_TPL				CurrentTPL;
#endif

	DPRINT((L"EFI_SeekBlock: pos %lX, whence %x\n", *Position, whence));

	if (!DevSpecific) {
		Status = EFI_INVALID_PARAMETER;
		goto Done;
	}

	BlockDev = DevSpecific;
	Media = BlockDev->BlockIo->Media;

	//
	// Position must be a multiple of the intrinsic block size of the device.
	//
	LIBC_DivU64x32(*Position , Media->BlockSize, &temp);
	if (temp != 0) {
		Status = EFI_INVALID_PARAMETER;
		DPRINT((L"EFI_SeekBlock: *Position not multiple of device block size %d\n",
				Media->BlockSize));
		goto Done;
	}

#ifdef EXCLUSION_FILELOCK
	DPRINT((L"EFI_SeekBlock: Raising TPL\n"));
	CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);

	// Check whether in use
	if (BlockDev->InUse == TRUE) {
		Status = EFI_NOT_READY;
		DPRINT((L"EFI_SeekBlock: Busy, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
		goto Done;
	}
	else {
		BlockDev->InUse = TRUE;
		AcquiredLock = TRUE ;
		DPRINT((L"EFI_SeekBlock: Locking, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	switch (whence) {
	case SEEK_SET:
		//
		//  Seek to the absolute position
		//
		SeekReqBlock = LIBC_DivU64x32(*Position , Media->BlockSize, NULL);
		if (/* (SeekReqBlock < 0) || */ (SeekReqBlock > Media->LastBlock)) {
			Status = EFI_INVALID_PARAMETER;
			DPRINT((L"EFI_SeekBlock: *Position out of range\n"));
			goto Done;
		}
		break;

	case SEEK_END:
		//
		//  Seek to the end of the file, then seek off the current
		//  position as for SEEK_CUR
		//
		SeekReqBlock = Media->LastBlock + LIBC_DivU64x32(*Position , Media->BlockSize, NULL);
		if (/* (SeekReqBlock < 0) || */ (SeekReqBlock > Media->LastBlock)) {
			Status = EFI_INVALID_PARAMETER;
			DPRINT((L"EFI_SeekBlock: *Position out of range\n"));
			goto Done;
		}
		break;

	case SEEK_CUR:
		//
		//  Get the current file position,
		//  Add the specified offset to the current position and seek there
		//
		SeekReqBlock = BlockDev->CurrentLBA + LIBC_DivU64x32(*Position , Media->BlockSize, NULL);
		if ((*Position != 0) && (/* (SeekReqBlock < 0) || */ (SeekReqBlock > Media->LastBlock))) {
			Status = EFI_INVALID_PARAMETER;
			DPRINT((L"EFI_SeekBlock: *Position out of range\n"));
			goto Done;
		}
		break;

	default:
		Status = EFI_INVALID_PARAMETER;
		goto Done;
		break;
	}

	BlockDev->CurrentLBA = SeekReqBlock;
	*Position = LIBC_MultU64x32(SeekReqBlock , Media->BlockSize) ;

Done:
#ifdef EXCLUSION_FILELOCK
	if (AcquiredLock == TRUE) {
		DPRINT((L"EFI_SeekBlock: Unlocking\n"));
		CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);
		BlockDev->InUse = FALSE;
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	DPRINT((L"EFI_SeekBlock returned: %r\n", Status));
	return Status;
}


//
//  Name:
//      EFI_FstatBlock
//
//  Description:
//      Stub for fstat function
//
//  Arguments:
//      FileHandle:  file handle of the file to get info on
//      StatBuffer:  The stat structure to fill out.
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_FstatBlock( 
	IN OUT	struct stat	*StatBuffer,
	IN OUT	VOID		*DevSpecific
	)
{
	EFI_STATUS			Status		 = EFI_SUCCESS;
	EFILIBC_BLOCKDEV_T	*BlockDev;
	EFI_BLOCK_IO_MEDIA	*Media;
#ifdef EXCLUSION_FILELOCK
	EFI_BOOT_SERVICES	*BootServ	 = _GetBootServices();
	BOOLEAN				AcquiredLock = FALSE;
	EFI_TPL				CurrentTPL;
#endif
	int	i = 0;

	DPRINT((L"EFI_FstatBlock\n"));

	if (!DevSpecific) {
		Status = EFI_INVALID_PARAMETER;
		goto Done;
	}

	BlockDev = DevSpecific;
	Media = BlockDev->BlockIo->Media;

#ifdef EXCLUSION_FILELOCK
	DPRINT((L"EFI_FstatBlock: Raising TPL\n"));
	CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);

	// Check whether in use
	if (BlockDev->InUse == TRUE) {
		Status = EFI_NOT_READY;
		DPRINT((L"EFI_FstatBlock: Busy, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
		goto Done;
	}
	else {
		BlockDev->InUse = TRUE;
		AcquiredLock = TRUE;
		DPRINT((L"EFI_FstatBlock: Locking, Restoring TPL\n"));
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	StatBuffer->st_mode = S_IFREG;
	StatBuffer->st_nlink = 1;
	StatBuffer->st_blocks = Media->LastBlock + 1;
	StatBuffer->st_size =  LIBC_MultU64x32((off_t)StatBuffer->st_blocks , Media->BlockSize);
	//
	//  We use a larger than real value here as an optimization for using
	//  buffered I/O (fread/fwrite).  Buffered I/O will only read a st_blksize
	//  amount at a time.....  Dumb!
	//
	StatBuffer->st_blksize = 0x8000;

	DPRINT((L"EFI_FstatBlock: st_blksize: %d\n", StatBuffer->st_blksize));
	DPRINT((L"EFI_FstatBlock: st_size:    %ld\n", StatBuffer->st_size));
	DPRINT((L"EFI_FstatBlock: st_blocks:  %ld\n", StatBuffer->st_blocks));

	//
	//  XXX haven't figured out exactly what to do with the rest of 
	//  these yet.  Time needs to be converted from the nice EFI form
	//  into the Unix no. of seconds since 1/1/70 UTC.  May be some
	//  existing code to help with that.  Maybe calculate st_blocks
	//  from other data.  Device type st_rdev may have standard constant 
	//  def'ns.
	//
	StatBuffer->st_dev = 0;
	StatBuffer->st_ino = 0;
	StatBuffer->st_uid = 0;
	StatBuffer->st_gid = 0;
	StatBuffer->st_rdev = 0;
	StatBuffer->st_atimespec.tv_sec = 0;
	StatBuffer->st_atimespec.tv_nsec = 0;
	StatBuffer->st_mtimespec.tv_sec = 0;
	StatBuffer->st_mtimespec.tv_nsec = 0;
	StatBuffer->st_ctimespec.tv_sec = 0;
	StatBuffer->st_ctimespec.tv_nsec = 0;
	StatBuffer->st_flags = 0;
	StatBuffer->st_gen = 0;

Done:
#ifdef EXCLUSION_FILELOCK
	if (AcquiredLock == TRUE) {
		DPRINT((L"EFI_FstatBlock: Unlocking\n"));
		CurrentTPL = BootServ->RaiseTPL(TPL_NOTIFY);
		BlockDev->InUse = FALSE;
		BootServ->RestoreTPL(CurrentTPL);
	}
#endif

	DPRINT((L"EFI_FstatBlock returned: %r\n", Status));
	return Status;
}


//
//  Name:
//      EFI_IoctlBlock
//
//  Description:
//      Stub for ioctl function
//
//  Arguments:
//      DevSpecific:  file handles
//      Request:      ioctl request
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_IoctlBlock( 
	IN	VOID	*DevSpecific,
	IN	UINT32	Request,
	IN	va_list	ArgList
	)
{
	DPRINT((L"EFI_IoctlBlock\n"));
	return EFI_UNSUPPORTED;
}


//
//  Name:
//      EFI_PollBlock
//
//  Description:
//      Stub for poll function
//
//  Arguments:
//      Mask:         mask of events to poll for
//      DevSpecific:  file handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_PollBlock( 
	IN	UINT32	Mask,
	IN	VOID	*DevSpecific
	)
{
	DPRINT((L"EFI_PollBlock\n"));
	return EFI_UNSUPPORTED;
}

//
//  Name:
//      _OpenBlock
//
//  Description:
//		Opens a block device.  The function is registered in the Map Protocol Table
//		for blkio devices.
//
//  Arguments:
//		FilePath:	Path of file to open
//		DevName:	Device name to use for mapping
//		Flags:		Flags
//		Mode:		Mode
//		DevPath:	Device path
//		Guid:		Guid
//		fd:			Pointer to file descriptor returned on success
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
_OpenBlock(
	char			*FilePath,
	char			*DevName,
	int				Flags,
	mode_t			Mode,
	EFI_DEVICE_PATH	*DevPath,
	EFI_GUID		*Guid,
	INT32			*fd
	)
{
    EFI_STATUS		Status;
    wchar_t         *wcspath;
    VOID            *DevSpecific;

	DPRINT(( L"_OpenBlock: allocate wcspath\n" ));

	//  Assume the worst
	*fd = -1;

	wcspath = _ConvertToEfiPathname( FilePath );
	if ( !wcspath ) {
		return EFI_OUT_OF_RESOURCES;
	}

	DPRINT(( L"_OpenBlock: wcspath %s\n", wcspath ));

	Status = EFI_OpenBlock( (CHAR16 * )wcspath, Flags, Mode, DevPath, &DevSpecific );
	if ( EFI_ERROR( Status ) ) {
		goto Error;
	}
    
	Status = _LIBC_AllocateNewFileDescriptor( 
				wcspath,                 // FileName
				Flags,                   // Flags
				Mode,                    // Mode
				FALSE,                   // IsATTy
				DevSpecific,             // DevSpecific
				EFI_ReadBlock,           // read
				EFI_WriteBlock,          // write
				EFI_CloseBlock,          // close
				EFI_SeekBlock,           // lseek
				EFI_FstatBlock,          // fstat
				EFI_IoctlBlock,          // ioctl stub
				EFI_PollBlock,           // poll stub
				fd                       // New FileDescriptor
				);

Error:
	// Free converted filename
	free( wcspath );
	return( Status ) ;
}

#endif
