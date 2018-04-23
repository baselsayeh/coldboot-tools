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

    fileio.c
    
Abstract:

    Interfaces to EFI for I/O to the file system


Revision History

--*/

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
    EFI_FILE_HANDLE FileHandle;
    EFI_FILE_HANDLE RootDirHandle;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN			InUse;
#endif
} EFILIBC_FILEDEV_T;

#define PSEUDO_BLKSIZE  0x2000

//
//  Name:
//      EFI_OpenFile
//
//  Description:
//      Open a file for use with EFI File System Protocol
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
EFI_OpenFile( 
    CHAR16           *FileName,
    int              Flags,
    mode_t           Mode,									
	EFI_DEVICE_PATH  *DevPath,
    VOID             **DevSpecific
    )
{
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_FILE_IO_INTERFACE  *Vol = NULL;
    EFILIBC_FILEDEV_T      *FileDev = NULL;
    EFI_FILE_HANDLE         RootDir = NULL;
    BOOLEAN                 FileExists;
    EFI_FILE_HANDLE         FileHandle = NULL;
	EFI_HANDLE              DeviceHandle = NULL;
    EFI_BOOT_SERVICES      *BootServ     = _GetBootServices();
	EFI_DEVICE_PATH        *DevicePath   = DevPath;
    UINT64                  EfiMode      = 0;
    UINT64                  EfiAttr      = 0;
    EFI_FILE_INFO           *FileInfo    = NULL;
    UINTN                   FileInfoSize = sizeof( EFI_FILE_INFO ) 
                                           + MAXPATHLEN * sizeof(CHAR16);

	DPRINT(( L"EFI_OpenFile: FileName=%s Flags=%X Mode=%X DevPath=%X\n", FileName, Flags, Mode, DevPath )) ;

    //
    //  Init
    //
    FileDev = calloc( sizeof( EFILIBC_FILEDEV_T ), 1 );
    if ( !FileDev ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }
#ifdef EXCLUSION_FILELOCK
	FileDev->InUse = FALSE ;
#endif
    *DevSpecific = FileDev;

	//
	//  Determine device handle for fs protocol on specified device path
	//
	DPRINT(( L"EFI_OpenFile: locate device path\n" ));
	Status = BootServ->LocateDevicePath ( &_LibcFileSystemProtocol,
										  &DevicePath,
										  &DeviceHandle );
    if ( EFI_ERROR ( Status ) ) {
        DPRINT(( L"EFI_OpenFile: error from LocateDevicePath(Filesystem)\n" ));
        goto Done;
    }

	//
	//  Determine volume for file system on device path
	//
    DPRINT(( L"EFI_OpenFile: get volume\n" ));
    Status = BootServ->HandleProtocol ( DeviceHandle, 
                                        &_LibcFileSystemProtocol,
                                        (VOID*)&Vol );
    if ( EFI_ERROR ( Status ) ) {
        DPRINT(( L"EFI_OpenFile: error from HandleProtocol(Filesystem)\n" ));
        goto Done;
    }

	//
	// Open volume for file system on device path
	//
    DPRINT(( L"EFI_OpenFile: open volume\n" ));
    assert( Vol != NULL );
    assert( Vol->OpenVolume != NULL );
    Status = Vol->OpenVolume ( Vol, &RootDir );
    if ( EFI_ERROR ( Status ) ) {
        DPRINT(( L"EFI_OpenFile: error from OpenVolume\n" ));
        goto Done;
    }

    //
    //  Save the Root Directory File Handle for stat(2)
    //
    FileDev->RootDirHandle = RootDir;

    //
    //  Set up EFI file mode and handle BSD flags
    //
    DPRINT(( L"EFI_OpenFile: handle BSD flags %X and mode %X\n", Flags, Mode ));

    //
    //  If the create flag is set, then we must first check to see
	//  if the file already exists.  If it does exist, then it should
	//  simply be opened, otherwise it must be created.
    //
    if ( (Flags & O_CREAT) ) {
        assert( RootDir->Open != NULL );
        Status = RootDir->Open( RootDir, 
                               &FileHandle, 
                                FileName, 
                                EFI_FILE_MODE_READ, 
                                0 );

        if ( EFI_ERROR(Status) ) {
            DPRINT(( L"EFI_OpenFile:  file does not exist, Status %X\n", Status ));
            FileExists = FALSE;
        } else {
            DPRINT(( L"EFI_OpenFile:  file does exist, Status %X\n", Status ));
            FileExists = TRUE;
            FileHandle->Close( FileHandle );
        }
    
        if ( FileExists ) {
            DPRINT(( L"EFI_OpenFile:  file already exists\n" ));
            if ( Flags & O_EXCL ) { 
                Status = EFI_ACCESS_DENIED;
                goto Done;
            }

        } else {
            DPRINT(( L"EFI_OpenFile:  create new file: %s\n", FileName ));
            EfiMode |= EFI_FILE_MODE_CREATE;

            //
            //  set attributes (file permissions)
            //  check for any unix-style write bits - if none set readonly
            //  check if we are creating a directory instead of a file
            //
            if (Mode & S_IFDIR) {
                EfiAttr = EFI_FILE_DIRECTORY;
                EfiMode |= (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE);
            } else if ( !( (Mode & (S_IRWXO|S_IRWXG|S_IRWXU)) & 
                    (S_IWOTH|S_IWGRP|S_IWUSR) ) ) {
                EfiAttr = EFI_FILE_READ_ONLY;
            } else {
                EfiAttr = 0;
			}
        }
    }

    //
    //  Access mode
    //
    if ( (O_ACCMODE & Flags) == O_RDONLY ) {
        EfiMode |= EFI_FILE_MODE_READ;

    } else if ( (O_ACCMODE & Flags) == O_WRONLY ) {
        //
        //  EFI does not support a write only mode (per the spec).
        //  We'll catch an attempt to read from a write only file in
        //  the write routine.
        //
        EfiMode |= EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;

    } else if ( (O_ACCMODE & Flags) == O_RDWR ) { 
        EfiMode |= EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;

    } else {
        Status = EFI_INVALID_PARAMETER;
        DPRINT(( L"EFI_OpenFile: bad access mode combo %x\n", (O_ACCMODE & Flags) ));
        goto Done;
    }

    //
    //  Check that we have valid open mode and attributes
    //
    if ( !EfiMode || ( !(EfiMode & EFI_FILE_MODE_CREATE) && EfiAttr ) ) {
        DPRINT(( L"EFI_OpenFile: bad parameter combo EfiMode %x EfiAttr %x\n", 
                 EfiMode, EfiAttr ));
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    //
    //  Finally, open the file
    //
    DPRINT(( L"EFI_OpenFile: open file, mode %lX, attr %lX\n", 
             EfiMode, EfiAttr ));
    assert( RootDir->Open != NULL );
    FileHandle = NULL;
    Status = RootDir->Open( 
                         RootDir, 
                         &FileHandle, 
                         FileName, 
                         EfiMode, 
                         EfiAttr
                         );

    if ( EFI_ERROR ( Status) ) {
        DPRINT(( L"EFI_OpenFile: error from Open: %x\n", Status ));
        goto Done;
    }
	DPRINT(( L"EFI_OpenFile: successfully opened file\n" ));

    //
    //  Save the file handle
    //
    FileDev->FileHandle = FileHandle;

DPRINT(( L"SetInfo %X\n", FileHandle->SetInfo ));
DPRINT(( L"SetPosition %X\n", FileHandle->SetPosition ));

    //
    //  Handle any remaining flags for post-open operations
    //
    if ( Flags & O_APPEND ) {
        //
        //  Set file position to EOF
        //
        assert( FileHandle->SetPosition );
        Status = FileHandle->SetPosition( FileHandle, -1 );
        if ( EFI_ERROR( Status ) ) {
            DPRINT(( L"EFI_OpenFile: O_APPEND: error from SetPosition: %r\n", Status ));
            goto Done;
        }
    } 

    if ( Flags & O_TRUNC ) {
        //
        //  Get file info and reset file size to zero
        //

        FileInfo = calloc( (size_t)FileInfoSize, sizeof(CHAR8) );
        if( !FileInfo ) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

        DPRINT(( L"EFI_OpenFile: call GetInfo: FileInfoSize %d\n", FileInfoSize ));
        assert( FileHandle->GetInfo != NULL );
        Status = FileHandle->GetInfo( 
                     FileHandle, 
                     &_LibcFileInfoId, 
                     &FileInfoSize,
                     FileInfo
                     );
        if ( EFI_ERROR( Status ) ) {
            DPRINT(( L"EFI_OpenFile: O_TRUNC: error from GetInfo: %r\n", Status ));
            DPRINT(( L"EFI_OpenFile: O_TRUNC: GetInfo returned FileInfoSize %d\n", 
                     FileInfoSize ));
            goto Done;
        }

		if( FileInfo->FileSize != 0 )
		{
			FileInfo->FileSize = (UINT64)0;
			DPRINT(( L"EFI_OpenFile: Call SetInfo: fcn %X filename %s\n", 
					FileHandle->SetInfo, FileInfo->FileName ));
			assert( FileHandle->SetInfo != NULL );
			Status = FileHandle->SetInfo( 
					FileHandle, 
					&_LibcFileInfoId, 
					FileInfoSize,
					FileInfo
					);
			DPRINT(( L"EFI_OpenFile: After SetInfo\n" ));
			if ( EFI_ERROR( Status ) ) {
				DPRINT(( L"EFI_OpenFile: O_TRUNC: error from SetInfo: %r\n", Status ));
				goto Done;
			}
		}
    }

Done:
    if ( FileInfo ) {
        free( FileInfo );
        FileInfo = NULL;
    }
    
    if ( EFI_ERROR( Status ) &&(FileHandle != NULL)){
    	FileHandle->Close(FileHandle);	
    }
    
    if (EFI_ERROR( Status ) && (RootDir != NULL) ){
    	RootDir->Close(RootDir);	
    }
    
    if ( EFI_ERROR( Status ) && *DevSpecific  ) {
		free( *DevSpecific ) ;
		*DevSpecific = NULL ;
	}

    DPRINT(( L"EFI_OpenFile: return status %X\n", Status ));
    return Status;
}


//
//  Name:
//      EFI_CloseFile
//
//  Description:
//      Close the specfied file
//
//  Arguments:
//      DevSpecific:  File handles
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_CloseFile( 
    IN  VOID             *DevSpecific
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
    EFI_FILE_HANDLE   RootDir;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL	           CurrentTPL ;
#endif

    DPRINT(( L"EFI_CloseFile\n" ));

    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_CloseFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_CloseFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_CloseFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    Status = FileHandle->Close( FileHandle );

    RootDir = FileDev->RootDirHandle;

    if ( !RootDir) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }
	
    Status = RootDir->Close(RootDir);
    

    free( FileDev );

Done:
    return Status;
}


//
//  Name:
//      EFI_ReadFile
//
//  Description:
//      Read a file using EFI File System Protocol
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
EFI_ReadFile( 
    OUT    VOID             *Buffer,
    IN OUT UINTN            *BufSize,
    IN     VOID             *DevSpecific
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL            CurrentTPL ;
#endif

    DPRINT(( L"EFI_ReadFile: *BufSize %d\n", *BufSize ));

    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_ReadFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_ReadFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_ReadFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    Status = FileHandle->Read( FileHandle, BufSize, Buffer );

    DPRINT(( L"EFI_ReadFile returned: %r\n", Status ));

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_ReadFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif
    return Status;
}

//
//  Name:
//      EFI_WriteFile
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
EFI_WriteFile( 
    OUT    VOID          *Buffer,
    IN OUT UINTN         *BufSize,
    IN     VOID          *DevSpecific
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL            CurrentTPL ;
#endif

    DPRINT(( L"EFI_WriteFile: BufSize %d\n", *BufSize ));

    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_WriteFile: Raising TPL\n" ));

	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_WriteFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_WriteFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    Status = FileHandle->Write( FileHandle, BufSize, Buffer );

    DPRINT(( L"EFI_WriteFile: returning BufSize %d\n", *BufSize ));

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_WriteFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif
    return Status;
}

//
//  Name:
//      EFI_SeekFile
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
EFI_SeekFile( 
    IN  UINT64           *Position,
    IN  UINT32           whence,
    IN  VOID             *DevSpecific
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
    UINT64             CurrentPosition;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL            CurrentTPL ;
#endif

    DPRINT(( L"EFI_SeekFile: pos %lX, whence %x\n", *Position, whence ));

    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_SeekFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_SeekFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_SeekFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }


    switch ( whence ) {
    case SEEK_SET:
        //
        //  Seek to the absolute position
        //
        Status = FileHandle->SetPosition( FileHandle, *Position );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }
        break;

    case SEEK_END:
        //
        //  Seek to the end of the file, then seek off the current
        //  position as for SEEK_CUR
        //
        Status = FileHandle->SetPosition( FileHandle, (UINT64)(-1) );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }
        /* FALLTHROUGH */

    case SEEK_CUR:
        //
        //  Get the current file position
        //
        Status = FileHandle->GetPosition( FileHandle, &CurrentPosition );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }

        //
        //  Add the specified offset to the current position and seek there
        //
        *Position = *Position + CurrentPosition;
        Status = FileHandle->SetPosition( FileHandle, *Position );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }
        break;

    default:
        Status = EFI_INVALID_PARAMETER;
        break;
    }

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_SeekFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif
    return Status;
}


//
//  Name:
//      EFI_FstatFile
//
//  Description:
//      Common stat code to return information about a file in a
//      stat structure.
//
//  Arguments:
//      FileHandle:  file handle of the file to get info on
//      StatBuffer:  The stat structure to fill out.
//      TrueBlkSize: Boolean to return true filesystem block size.
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
_FstatFile( 
    IN  struct stat      *StatBuffer,
    IN  VOID             *DevSpecific,
    IN	BOOLEAN          TrueBlkSize
    )
{
    EFI_STATUS           Status;
    EFI_FILE_INFO        *FileInfo;
    EFI_FILE_HANDLE      RootDir;
    UINTN                FileInfoSize    = sizeof( EFI_FILE_INFO )
                                           + MAXPATHLEN * sizeof(CHAR16);
    EFI_FILE_SYSTEM_INFO *FileSysInfo;
    UINTN                FileSysInfoSize = sizeof( EFI_FILE_SYSTEM_INFO )
                                           + MAXPATHLEN * sizeof(CHAR16);
    EFILIBC_FILEDEV_T    *FileDev        = NULL;
    EFI_FILE_HANDLE      FileHandle      = NULL;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN              AcquiredLock = FALSE ;
	EFI_TPL              CurrentTPL ;
#endif

    DPRINT(( L"EFI_FstatFile\n" ));

    //
    //  Init
    //
    FileInfo = NULL;
    FileSysInfo = NULL;

    FileInfo = calloc( (size_t)FileInfoSize, sizeof( char ) );
    if ( !FileInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }
    FileSysInfo = calloc( (size_t)FileSysInfoSize, sizeof( char ) );
    if ( !FileInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_FstatFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_FstatFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_FstatFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;
    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    RootDir = FileDev->RootDirHandle;
    if ( !RootDir ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    //
    //  Get File info
    //

    DPRINT(( L"EFI_FstatFile: get file info fcn 0x%X\n", FileHandle->GetInfo ));
    Status = FileHandle->GetInfo( FileHandle, 
                                  &_LibcFileInfoId, 
                                  &FileInfoSize,
                                  FileInfo );
    if ( EFI_ERROR( Status ) ) {
        goto Done;
    }

    //
    //  Get File *System* Info
    //

    DPRINT(( L"EFI_FstatFile: get file system info\n" ));

#ifdef EFI_NT_EMULATOR
    //
    //  The NT emulator does not support GetInfo on the root volume so
    //  always fake it.
    //
    TrueBlkSize = FALSE;
#endif

    if ( TrueBlkSize ) {
        Status = RootDir->GetInfo( RootDir, 
                               &_LibcFileSystemInfoId, 
                               &FileSysInfoSize,
                               FileSysInfo );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }
    } else {
        FileSysInfo->BlockSize = PSEUDO_BLKSIZE;
    }

    //
    //  Translate EFI file info into struct stat.  Keep the original
    //  struct stat to enable future porting efforts.
    //
    
    DPRINT(( L"EFI_FstatFile: assign stat buffer\n" ));
    if ( FileInfo->Attribute & EFI_FILE_DIRECTORY ) {
        StatBuffer->st_mode = S_IFDIR;
    } else {
        StatBuffer->st_mode = S_IFREG;
    }
        
    StatBuffer->st_nlink = 1;
    StatBuffer->st_size = FileInfo->FileSize;
    StatBuffer->st_blksize = FileSysInfo->BlockSize;

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
    StatBuffer->st_atimespec.tv_sec = EfiTimeToUnixTime(&(FileInfo->LastAccessTime));
    StatBuffer->st_atimespec.tv_nsec = 0;
    StatBuffer->st_mtimespec.tv_sec = EfiTimeToUnixTime(&(FileInfo->ModificationTime));
    StatBuffer->st_mtimespec.tv_nsec = 0;
    StatBuffer->st_ctimespec.tv_sec = EfiTimeToUnixTime(&(FileInfo->CreateTime));
    StatBuffer->st_ctimespec.tv_nsec = 0;
    StatBuffer->st_blocks = 0;
    StatBuffer->st_flags = 0;
    StatBuffer->st_gen = 0;

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_FstatFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    if ( FileInfo ) {
        free( FileInfo );
        FileInfo= NULL;
    }
    if ( FileSysInfo ) {
        free( FileSysInfo );
        FileSysInfo = NULL;
    }

    DPRINT(( L"EFI_FstatFile: return\n" ));
    return Status;
}

//
//  Name:
//      EFI_FstatFile
//
//  Description:
//      Return information about a file in a stat structure
//
//  Arguments:
//      FileHandle:  file handle of the file to get info on
//      StatBuffer:  The stat structure to fill out.
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
EFI_FstatFile( 
    IN  struct stat      *StatBuffer,
    IN  VOID             *DevSpecific
    )
{
	return (_FstatFile( StatBuffer, DevSpecific, TRUE ));
}

//
//  Name:
//      EFI_FastFstatFile
//
//  Description:
//      Return information about a file in a stat structure.
//      To speed the call, the block size will be returned as
//      a constant.
//
//  Arguments:
//      FileHandle:  file handle of the file to get info on
//      StatBuffer:  The stat structure to fill out.
//
//  Returns:
//      EFI_STATUS
//
EFI_STATUS
EFI_FastFstatFile( 
    IN  struct stat      *StatBuffer,
    IN  VOID             *DevSpecific
    )
{
	return (_FstatFile( StatBuffer, DevSpecific, FALSE ));
}

//
//  Name:
//      EFI_IoctlFile
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
EFI_IoctlFile( 
    IN  VOID    *DevSpecific,
    IN  UINT32  Request,
    IN  va_list ArgList
    )
{
    return EFI_UNSUPPORTED;
}

//
//  Name:
//      EFI_PollFile
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
EFI_PollFile( 
    IN  UINT32  Mask,
    IN  VOID    *DevSpecific
    )
{
    return EFI_UNSUPPORTED;
}


EFI_STATUS
EFI_UnlinkFile(
    IN VOID *DevSpecific
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
    EFI_FILE_HANDLE   RootDir;    
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL            CurrentTPL ;
#endif
    
    if ( !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_UnlockFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_UnlockFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_UnlockFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    assert( FileHandle->Delete );

    DPRINT((L"EFI_UnlinkFile: Call Delete %X\n", FileHandle->Delete));
    Status = FileHandle->Delete( FileHandle );
    if ( EFI_ERROR( Status ) ) {
        DPRINT((L"error encountered in EFI_UnlinkFile: %r\n", Status ));
        goto Done;
    }

    RootDir = FileDev->RootDirHandle;

    if ( !RootDir) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }
	
    Status = RootDir->Close(RootDir);

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_UnlinkFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif
    return Status;
}

//
//  Name:
//      EFI_RenameFile
//
//  Description:
//      Rename the file to the name specified in NewName
//
//  Arguments:
//      DevSpecific:  File handles
//      NewName:  New filename for file refered to by devSpecific
//
//  Returns:
//      EFI_STATUS							   
//
EFI_STATUS
EFI_RenameFile( 
    IN  VOID             *DevSpecific,
    IN CHAR16            *NewName
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
    EFI_FILE_INFO      *FileInfo    = NULL;
    UINTN              FileInfoSize = sizeof( EFI_FILE_INFO )
                                           + ((MAXPATHLEN + 1) * sizeof(CHAR16));
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL	           CurrentTPL ;
#endif

    if ( !DevSpecific ) 
    {
        Status = EFI_INVALID_PARAMETER;
		DPRINT(( L"EFI_RenameFile: DevSpecific\n"));
        goto Done;
    }

	if ( wcslen ( NewName ) > MAXPATHLEN )
	{
        Status = EFI_INVALID_PARAMETER;
		DPRINT(( L"EFI_RenameFile: bad pathlength\n"));
        goto Done;
    }


    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_UnlockFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_UnlockFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_UnlockFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
		DPRINT(( L"EFI_RenameFile: bad file handle\n"));
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileInfo = calloc( (size_t)FileInfoSize, sizeof( char ) );
    if ( !FileInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    assert( FileHandle->GetInfo );
    assert( FileHandle->SetInfo );

	/*
	 *  Get the exising fileinfo
	 */

    Status = FileHandle->GetInfo( FileHandle, &_LibcFileInfoId, 
                                  &FileInfoSize,FileInfo );
    if ( EFI_ERROR( Status ) ) 
    {
		DPRINT(( L"EFI_RenameFile: GetInfo failed status=0x%x\n",Status));
        goto Done;				   
    }

	/*
	 *  change only the name
	 */

	wmemcpy (FileInfo->FileName, NewName, wcslen( NewName )+1);	

	/*
	 *  Set the fileinfo
	 */

	FileInfoSize=sizeof( EFI_FILE_INFO ) + ((MAXPATHLEN + 1) * sizeof(CHAR16));
	FileInfo->Size = sizeof( EFI_FILE_INFO ) + ((MAXPATHLEN + 1) * sizeof(CHAR16));
    Status = FileHandle->SetInfo( FileHandle, &_LibcFileInfoId, 
                                  FileInfoSize, FileInfo );
    if ( EFI_ERROR( Status ) ) 
    {
		DPRINT(( L"EFI_RenameFile: SetInfo failed status=0x%x\n",Status));
        goto Done;
    }

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_UnlinkFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

	if ( FileInfo ) free( FileInfo );
    return Status;
}

//
//  Name:
//      EFI_UtimesFile
//
//  Description:
//      Change a file's access and modification times
//
//  Arguments:
//      DevSpecific:  File handles
//      Times:  New access and modificatoin times
//
//  Returns:
//      EFI_STATUS							   
//
EFI_STATUS
EFI_UtimesFile( 
    IN  VOID             *DevSpecific,
    IN struct timeval    *Times
    )
{
    EFI_STATUS         Status;
    EFILIBC_FILEDEV_T  *FileDev;
    EFI_FILE_HANDLE    FileHandle;
    EFI_FILE_INFO      *FileInfo    = NULL;
    UINTN              FileInfoSize = sizeof( EFI_FILE_INFO )
                                           + ((MAXPATHLEN + 1) * sizeof(CHAR16));
    struct tm          *LocalTime   = NULL;
#ifdef EXCLUSION_FILELOCK
	BOOLEAN            AcquiredLock = FALSE ;
	EFI_TPL	           CurrentTPL ;
#endif

    if ( !DevSpecific ) 
    {
        Status = EFI_INVALID_PARAMETER;
		DPRINT(( L"EFI_UtimesFile: DevSpecific\n"));
        goto Done;
    }

	if ( !Times )
	{
        Status = EFI_INVALID_PARAMETER;
		DPRINT(( L"EFI_UtimesFile: no times\n"));
        goto Done;
    }

    FileDev = DevSpecific;

#ifdef EXCLUSION_FILELOCK
	DPRINT(( L"EFI_UtimesFile: Raising TPL\n" ));
	CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;

	// Check whether in use
	if( FileDev->InUse == TRUE ) {
		Status = EFI_NOT_READY ;
		DPRINT(( L"EFI_UtiesFile: Busy, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
		goto Done ;
	}
	else {
		FileDev->InUse = TRUE ;
		AcquiredLock = TRUE ;
		DPRINT(( L"EFI_UtimesFile: Locking, Restoring TPL\n" ));
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

    FileHandle = FileDev->FileHandle;

    if ( !FileHandle ) {
		DPRINT(( L"EFI_UtimesFile: bad file handle\n"));
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    FileInfo = calloc( (size_t)FileInfoSize, sizeof( char ) );
    if ( !FileInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    assert( FileHandle->GetInfo );
    assert( FileHandle->SetInfo );

	/*
	 *  Get the exising fileinfo
	 */

    Status = FileHandle->GetInfo( FileHandle, &_LibcFileInfoId, 
                                  &FileInfoSize,FileInfo );
    if ( EFI_ERROR( Status ) ) 
    {
		DPRINT(( L"EFI_UtimesFile: GetInfo failed status=0x%x\n",Status));
        goto Done;			   
    }

	/*
	 *  Change the access and modification times
	 */

    LocalTime = localtime(&(Times->tv_sec));
    FileInfo->LastAccessTime.Year = LocalTime->tm_year + 1900; // tm_year is years since 1900
    FileInfo->LastAccessTime.Month = LocalTime->tm_mon + 1;    // tm_mon is [0-11]
    FileInfo->LastAccessTime.Day = LocalTime->tm_mday;
    FileInfo->LastAccessTime.Hour = LocalTime->tm_hour;
    FileInfo->LastAccessTime.Minute = LocalTime->tm_min;
    FileInfo->LastAccessTime.Second = LocalTime->tm_sec;
    FileInfo->LastAccessTime.Nanosecond = 0;
    FileInfo->LastAccessTime.TimeZone = (INT16)LocalTime->tm_gmtoff;
    FileInfo->LastAccessTime.Daylight = LocalTime->tm_isdst ? EFI_TIME_IN_DAYLIGHT : 0;
    ++Times;
    LocalTime = localtime(&(Times->tv_sec));
    FileInfo->ModificationTime.Year = LocalTime->tm_year + 1900; // tm_year is years since 1900
    FileInfo->ModificationTime.Month = LocalTime->tm_mon + 1;    // tm_mon is [0-11]
    FileInfo->ModificationTime.Day = LocalTime->tm_mday;
    FileInfo->ModificationTime.Hour = LocalTime->tm_hour;
    FileInfo->ModificationTime.Minute = LocalTime->tm_min;
    FileInfo->ModificationTime.Second = LocalTime->tm_sec;
    FileInfo->ModificationTime.Nanosecond = 0;
    FileInfo->ModificationTime.TimeZone = (INT16)LocalTime->tm_gmtoff;
    FileInfo->ModificationTime.Daylight = LocalTime->tm_isdst ? EFI_TIME_IN_DAYLIGHT : 0;

	/*
	 *  Set the fileinfo
	 */

	FileInfoSize=sizeof( EFI_FILE_INFO ) + ((MAXPATHLEN + 1) * sizeof(CHAR16));
	FileInfo->Size = sizeof( EFI_FILE_INFO ) + ((MAXPATHLEN + 1) * sizeof(CHAR16));
    Status = FileHandle->SetInfo( FileHandle, &_LibcFileInfoId, 
                                  FileInfoSize, FileInfo );
    if ( EFI_ERROR( Status ) ) 
    {
		DPRINT(( L"EFI_UtimesFile: SetInfo failed status=0x%x\n",Status));
        goto Done;
    }

Done:
#ifdef EXCLUSION_FILELOCK
	if( AcquiredLock == TRUE ) {
		DPRINT(( L"EFI_UtimesFile: Unlocking\n" ));
		CurrentTPL = _GetBootServices()->RaiseTPL( TPL_NOTIFY ) ;
		FileDev->InUse = FALSE ;
		_GetBootServices()->RestoreTPL( CurrentTPL ) ;
	}
#endif

	if ( FileInfo )
		free( FileInfo );
    return Status;
}

/*****************************/
//
//  Name:
//      _OpenFile
//
//  Description:
//		Opens a file.  The function is registered in the Map Protocol Table
//		for both the SIMPLE_FILE_SYSTEM_PROTOCOL.
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
_OpenFile(
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

	DPRINT(( L"_OpenFile: allocate wcspath\n" ));

	//  Assume the worst
	*fd = -1;

	//
	//  If FilePath is null, assume root "/".  This can happen when folks
	//  try to do a stat on things like fs0:
	//
	if ( FilePath == NULL )
		FilePath = "/";

	wcspath = _ConvertToEfiPathname( FilePath );
	if ( !wcspath ) {
		return EFI_OUT_OF_RESOURCES;
	}

	DPRINT(( L"_OpenFile: wcspath %s\n", wcspath ));

	Status = EFI_OpenFile( (CHAR16 * )wcspath, Flags, Mode, DevPath, &DevSpecific );
	if ( EFI_ERROR( Status ) ) {
		goto Error;
	}
    
	Status = _LIBC_AllocateNewFileDescriptor( 
				wcspath,                 // FileName
				Flags,                   // Flags
				Mode,                    // Mode
				FALSE,                   // IsATTy
				DevSpecific,             // DevSpecific
				EFI_ReadFile,            // read
				EFI_WriteFile,           // write
				EFI_CloseFile,           // close
				EFI_SeekFile,            // lseek
				EFI_FstatFile,           // fstat
				EFI_IoctlFile,           // ioctl stub
				EFI_PollFile,            // poll stub
				fd                       // New FileDescriptor
				);

Error:
	// Free converted filename
	free( wcspath );
	return( Status ) ;
}
