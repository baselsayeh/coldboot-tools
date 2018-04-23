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

    filedesc.c
    
Abstract:

    Functions and data implementing file descriptors


Revision History

--*/


#include <efi_interface.h>
#include <sys/syslimits.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#define KERNEL
#include <errno.h>
#undef KERNEL
#include "./filedesc.h"
#include <string.h>

#include <libc_debug.h>

//
//  File Descriptor Table
//
static EFILIBC_FILE_DESCRIPTOR_T  *FileDescriptorTable[ OPEN_MAX ];

//
//  Name:
//      _IsValidOpenFd
//
//  Description:
//      Checks fd to see if it is a valid open file descriptor
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      TRUE if fd is valid fd for open object, FALSE otherwise
//
static BOOLEAN
_IsValidOpenFd(
    INT32  fd
    )
{
	BOOLEAN		RetVal = TRUE ; 

    //
    // Out of range?
    //

    if ( fd < 0 || fd > OPEN_MAX ) {
        return FALSE;
    }

    //
    // Open file?
    //
    
    if ( FileDescriptorTable[ fd ] == NULL ) {
        return FALSE;
    }

    return TRUE;
}


//
//  Name:
//      InitializeFileDescriptorTable
//
//  Description:
//      Initializes the File Descriptor Table on startup initialization
//
//  Arguments:
//      -- none --
//
//  Returns:
//      EFI Status code.
//

EFI_STATUS
_InitializeFileDescriptorTable(
    VOID
    )
{
    int i;

    for ( i = 0; i < OPEN_MAX; i++ ) {
		FileDescriptorTable[ i ] = NULL;
    }
    return EFI_SUCCESS;
}



//
//  === External Interfaces ===
//


//
//  Name:
//      _LIBC_AllocateNewFileDescriptor
//
//  Description:
//      Assigns a new file descriptor with the specified values
//      NOTE: FileName == NULL means file descriptor is free
//
//  Arguments:
//      IN *FileName:  The name of the file or device
//      IN Flags:      Flags as defined in fcntl or implementation specific
//      IN Mode:       Permission mode
//      IN IsATTy:     True if the device is a tty (i.e. the console)
//      IN *DevSpecific:   The device specific data
//      IN read:       Pointer to the function to read the file or device
//      IN write:      Pointer to the function to write the file or device
//      IN close:      Pointer to the function to close the file or device
//      IN lseek:      Pointer to the function to seek on the file or device
//      IN fstat:      Pointer to the function to fstat the file or device
//      IN ioctl:      Pointer to the function to control the file or device
//      OUT *fd:
//
//  Returns:
//      EFI status code
//

EFI_STATUS
_LIBC_AllocateNewFileDescriptor( 
    IN  CHAR16          *FileName,
    IN  UINT32          Flags,
    IN  UINT32          Mode,
    IN  BOOLEAN         IsATTy,
    IN  VOID            *DevSpecific,
    IN  EFILIBC_READ_T   read,
    IN  EFILIBC_WRITE_T  write,
    IN  EFILIBC_CLOSE_T  close,
    IN  EFILIBC_LSEEK_T  lseek,
    IN  EFILIBC_FSTAT_T  fstat,
    IN  EFILIBC_IOCTL_T  ioctl,
    IN  EFILIBC_POLL_T   poll,
    OUT INT32           *fd
    )
{
	EXCLUSION_LOCK_DECL;

	EFILIBC_FILE_DESCRIPTOR_T  *Fdp;
    int                        i;

    if ( FileName == NULL || read == NULL || write == NULL || close == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    /*
     *  Find the lowest-numbered file descriptor, and fill it in
     */

	EXCLUSION_LOCK_TAKE();
    Fdp = NULL;
    for ( i = 0; i < OPEN_MAX; i++ ) {

        /*
         *  Look for an empty slot
         */
        if ( FileDescriptorTable[ i ] == NULL ) {

        	/*
        	 *  Allocate memory for descriptor
        	 */
        	Fdp = calloc( 1, sizeof(*Fdp) );
        	if ( Fdp == NULL ) {
	            DPRINT((L"AllocateNewFileDescriptor: no memory for descriptor\n"));
        		goto Error;
        	}

        	/*
        	 *  Allocate memory for device name
        	 */
            Fdp->FileName = calloc( wcslen(FileName)+1, sizeof(CHAR16) );
            if( Fdp->FileName == NULL ) {
	            DPRINT((L"AllocateNewFileDescriptor: no memory for name\n"));
	            goto Error;
	        }

        	/*
        	 *  We have the resources we need so initialize descriptor
        	 */
        	FileDescriptorTable[ i ] = Fdp;

            wcscpy( Fdp->FileName, FileName );
            Fdp->Flags       = Flags;
            Fdp->Mode        = Mode;
            Fdp->IsATTy      = IsATTy;
            Fdp->Refs        = 0;	// It's up to others to take references
            Fdp->Unlink      = 0;
            Fdp->DevSpecific = DevSpecific;
            Fdp->read        = read;
            Fdp->write       = write;
            Fdp->close       = close;
            Fdp->lseek       = lseek;
            Fdp->fstat       = fstat;
            Fdp->ioctl       = ioctl;
            Fdp->poll        = poll;

            *fd = i;

            EXCLUSION_LOCK_RELEASE();
            return EFI_SUCCESS;
        }
    }

    DPRINT((L"AllocateNewFileDescriptor: not found\n"));

Error:
	EXCLUSION_LOCK_RELEASE();

    *fd = -1;

	/*
	 *  If we made allocations, free the resources
	 */
	if ( Fdp ) {
		if ( Fdp->FileName )
			free( Fdp->FileName );
		free( Fdp );

        return EFI_OUT_OF_RESOURCES;
	}

    return EFI_NOT_FOUND;
}

//
//  Name:
//      _ReleaseFileDescriptor
//
//  Description:
//      Frees all resources associated with a file descriptor
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      EFI_INVALID_PARAMETER or EFI_SUCCESS
//

EFI_STATUS
_ReleaseFileDescriptor(
    INT32 Fd
    )
{
	EXCLUSION_LOCK_DECL;

    if ( Fd < 0 || Fd >= OPEN_MAX ) {
        return EFI_INVALID_PARAMETER;
    }

    /*
     *  Note that we do unconditional release.  It is up to the call
     *  to check reference counts if needed.
     */
	EXCLUSION_LOCK_TAKE();
    if ( FileDescriptorTable[ Fd ] != NULL ) {
		if ( FileDescriptorTable[ Fd ]->Refs ) {
			DPRINT((L"_ReleaseFilesDescriptor: fd=%d Refs=%d\n",
							Fd, FileDescriptorTable[ Fd ]->Refs));
        }
        free( FileDescriptorTable[ Fd ]->FileName );
        free( FileDescriptorTable[ Fd ] );
        FileDescriptorTable[ Fd ] = NULL;
    }
	EXCLUSION_LOCK_RELEASE();

    return EFI_SUCCESS ; 
}
    
//
//  Name:
//      _ReleaseFileDescriptorSlot
//
//  Description:
//      Free a file descriptor slot.  The file descriptor remains.
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      EFI_INVALID_PARAMETER or EFI_SUCCESS
//

EFI_STATUS
_ReleaseFileDescriptorSlot(
    INT32 Fd
    )
{
	EXCLUSION_LOCK_DECL;

    if ( Fd < 0 || Fd >= OPEN_MAX ) {
        return EFI_INVALID_PARAMETER;
    }

    /*
     *  Note that we do unconditional release the slot.  It is up to the call
     *  to check reference counts if needed.
     */
	EXCLUSION_LOCK_TAKE();
    if ( FileDescriptorTable[ Fd ] != NULL ) {
		if ( FileDescriptorTable[ Fd ]->Refs == 0 ) {
			DPRINT((L"_ReleaseFilesDescriptorSlot: fd=%d Refs=%d\n",
					Fd, FileDescriptorTable[ Fd ]->Refs));
        }
        FileDescriptorTable[ Fd ] = NULL;
    }
	EXCLUSION_LOCK_RELEASE();

    return EFI_SUCCESS ; 
}

//
//  Name:
//      _DupFileDescritor
//
//  Description:
//      Allocate a descriptor table slot and point it to the file descriptor
//      referenced by OldFd.
//
//  Arguments:
//      Oldfd  - File Descriptor to duplicate
//      NewFd  - Pointer to new File Descriptor index storage
//
//  Returns:
//      EFI_INVALID_PARAMETER or EFI_SUCCESS
//

EFI_STATUS
_DupFileDescriptor(
	IN      int OldFd,
	IN OUT  int *NewFd
	)
{
	EXCLUSION_LOCK_DECL;

	EFI_STATUS	Status;
	int			i;

    if ( OldFd < 0 || OldFd >= OPEN_MAX ) {
        return EFI_INVALID_PARAMETER;
    }

	EXCLUSION_LOCK_TAKE();

    /*
     *  If NewFd == -1, find an empty slot (i.e. dup(2)).
     */
    if ( *NewFd == -1 ) {
	    /*
	     *  Look for an empty slot.
	     */
	    for ( i = 0; i < OPEN_MAX; i++ ) {
	        if ( FileDescriptorTable[ i ] == NULL ) {
				FileDescriptorTable[ i ] = FileDescriptorTable[ OldFd ];
			    FileDescriptorTable[ i ]->Refs++;
	            *NewFd = i;
	            Status = EFI_SUCCESS;
	            goto Done;
	        }
	    }

	    /*
	     *  Out of empty slots
	     */
	    *NewFd = -1;
	    Status = EFI_NOT_FOUND;

    } else {

    	/*
    	 *  Caller wants to specify slot (i.e dup2(2))
    	 */
    	Status = _ReleaseFileDescriptorSlot( *NewFd );
    	if (EFI_ERROR(Status)) {
			goto Done;
    	}

		FileDescriptorTable[ *NewFd ] = FileDescriptorTable[ OldFd ];
	    FileDescriptorTable[ *NewFd ]->Refs++;
	    Status = EFI_SUCCESS;
    }

Done:
	EXCLUSION_LOCK_RELEASE();
    return Status; 
}

//
//  Name:
//      _GetOpenFileIsATTy
//
//  Description:
//      Returns IsAtty element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      IsAtty Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileIsATTy(
    IN  INT32   fd,
    OUT BOOLEAN *tty
    ) 
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *tty = FileDescriptorTable[ fd ]->IsATTy;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileFLags
//
//  Description:
//      Returns Flags element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Flags Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileFlags(
    IN  INT32  fd,
    OUT UINT32 *flags
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *flags = FileDescriptorTable[ fd ]->Flags;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _SetOpenFileFLags
//
//  Description:
//      Set Flags element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Flags Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_SetOpenFileFlags(
    IN  INT32  fd,
    OUT UINT32 flags
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    FileDescriptorTable[ fd ]->Flags = flags;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _LIBC_GetOpenFileDevSpecific
//
//  Description:
//      Returns DevSpecific element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Name:
//      _LIBC_GetOpenFileDevSpecific
//
//  Description:
//      Returns DevSpecific element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      DevSpecific Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_LIBC_GetOpenFileDevSpecific(
    IN  INT32 fd,
    OUT VOID  **DevSpecific
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *DevSpecific = FileDescriptorTable[ fd ]->DevSpecific;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}

//
//  Name:
//      _GetOpenFileRead
//
//  Description:
//      Returns Read element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Read Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileRead(
    IN  INT32          fd,
    OUT EFILIBC_READ_T *read
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *read = FileDescriptorTable[ fd ]->read;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileWrite
//
//  Description:
//      Returns Write element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Write Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileWrite(
    IN  INT32           fd,
    OUT EFILIBC_WRITE_T *write
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *write = FileDescriptorTable[ fd ]->write;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileClose
//
//  Description:
//      Returns Close element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Close Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileClose(
    IN  INT32           fd,
    OUT EFILIBC_CLOSE_T *close
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *close = FileDescriptorTable[ fd ]->close;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileLseek
//
//  Description:
//      Returns Lseek element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Lseek Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileLseek(
    IN  INT32           fd,
    OUT EFILIBC_LSEEK_T *lseek
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *lseek = FileDescriptorTable[ fd ]->lseek;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileFstat
//
//  Description:
//      Returns Fstat element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Fstat Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileFstat(
    IN  INT32           fd,
    OUT EFILIBC_FSTAT_T *fstat
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *fstat = FileDescriptorTable[ fd ]->fstat;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}


//
//  Name:
//      _GetOpenFileIoctl
//
//  Description:
//      Returns Ioctl element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Ioctl Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFileIoctl(
    IN  INT32           fd,
    OUT EFILIBC_IOCTL_T *ioctl
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *ioctl = FileDescriptorTable[ fd ]->ioctl;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}

//
//  Name:
//      _GetOpenFilePoll
//
//  Description:
//      Returns Poll element of file descriptor structure
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Poll Element
//
//  Returns:
//      EFI_STATUS
//

INT32
_GetOpenFilePoll(
    IN  INT32           fd,
    OUT EFILIBC_POLL_T  *poll
    )
{
	EXCLUSION_LOCK_DECL;

    INT32 RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

    *poll = FileDescriptorTable[ fd ]->poll;

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}

//
//  Name:
//      _MarkFileForUnlink
//
//  Description:
//      Set unlink flag on all descriptors with the same file name
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      0 on success
//      -1 on error
//

INT32
_MarkFileForUnlink(
    IN  INT32   fd
    )
{
	EXCLUSION_LOCK_DECL;

	CHAR16	*FileName;
    INT32	i;
    INT32   RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

	FileName = FileDescriptorTable[ fd ]->FileName;
    if ( !FileName ) {
        errno = EINVAL;
        RetVal = -1;
        goto Done;
    }

    for ( i = 0; i < OPEN_MAX; i++ ) {
        if ( FileDescriptorTable[i] && FileDescriptorTable[ i ]->FileName &&
			wcscmp( FileName, FileDescriptorTable[ i ]->FileName ) == 0 ) {
			FileDescriptorTable[ i ]->Unlink = 1;
        }
    }

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}

//
//  Name:
//      _GetUnlinkFlagCount
//
//  Description:
//      Returns unlink flag count for this file (i.e. filename).
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//      Pointer to Unlink flag storage
//
//  Returns:
//      0 on success
//      -1 on error
//

INT32
_GetUnlinkFlagCount(
    IN  INT32   fd,
    OUT	int	    *UnlinkFlagCount
    )
{
	EXCLUSION_LOCK_DECL;

	CHAR16	*FileName;
    INT32	i;
    INT32   RetVal = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        errno = EBADF;
        RetVal = -1;
        goto Done;
    }

	FileName = FileDescriptorTable[ fd ]->FileName;
    if ( !FileName ) {
        errno = EINVAL;
        RetVal = -1;
        goto Done;
    }

	*UnlinkFlagCount = 0;
    for ( i = 0; i < OPEN_MAX; i++ ) {
        if ( FileDescriptorTable[i] && FileDescriptorTable[ i ]->FileName &&
			wcscmp( FileName, FileDescriptorTable[ i ]->FileName ) == 0 ) {
			if (FileDescriptorTable[ i ]->Unlink) {
				(*UnlinkFlagCount)++;
			}
        }
    }

Done:
	EXCLUSION_LOCK_RELEASE();
    return RetVal;
}

//
//  Name:
//      _AddFileReference
//
//  Description:
//      Add a reference to a file descriptor
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      Current reference count or -1 on error
//

int
_AddFileReference(
    IN  INT32   fd
    )
{
	EXCLUSION_LOCK_DECL;

    int         Refs;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        Refs = -1;
        goto Done;
    }

    FileDescriptorTable[ fd ]->Refs++;
    Refs = FileDescriptorTable[ fd ]->Refs;

Done:
	EXCLUSION_LOCK_RELEASE();
    return Refs;
}

//
//  Name:
//      _RemoveFileReference
//
//  Description:
//      Remove a reference to a file descriptor
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      Current reference count or -1 on error
//

int
_RemoveFileReference(
    IN  INT32   fd
    )
{
	EXCLUSION_LOCK_DECL;

    int         Refs;

	EXCLUSION_LOCK_TAKE();
    if ( !_IsValidOpenFd(fd) ) {
        Refs = -1;
        goto Done;
    }

    FileDescriptorTable[ fd ]->Refs--;
    Refs = FileDescriptorTable[ fd ]->Refs;

Done:
	EXCLUSION_LOCK_RELEASE();
    return Refs;
}

//
//  Name:
//      getdtablesize
//
//  Description:
//      Provide BSD getdtablesize interface for EFI
//
//  Arguments:
//      -- none --
//
//  Returns:
//      size of file descriptor table
//
int
getdtablesize(
    void
    )
{
    return OPEN_MAX;
}

//
//  Name:
//      _FileDescriptorShutdown
//
//  Description:
//      Close all open file descriptors - called on application exit
//
//  Arguments:
//      void
//
//  Returns:
//      void
//

void
_FileDescriptorTableShutdown( void )
{
	int		i;

	//
	//  Because EFI file operations can not be called at raied TPL, we do
	//  not do file locking here.  This should not be a problem since this
	//  is called at program exit and there is no opportunity to alter the
	//  FileDescriptorTable while we are looping through it.
	//
    for ( i = 0; i < OPEN_MAX; i++ ) {
        if ( FileDescriptorTable[ i ] != NULL ) {
        	close( i );
       	}
    }
}
