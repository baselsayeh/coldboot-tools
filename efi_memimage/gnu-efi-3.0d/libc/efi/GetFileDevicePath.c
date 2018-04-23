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

    GetFileDevicePath.c
    
Abstract:

    C library interface to convert a Unicode fully qualified filename
    into it's associated device path.


Revision History

--*/

#include <atk_libc.h>
#include <efilib.h>
#include <efi_interface.h>
#include <stdlib.h>
#include <string.h>

static
EFI_DEVICE_PATH*
CreateFullDevPath(
	IN	EFI_DEVICE_PATH	*Dev,
	IN	CHAR16			*FilePath
	)
{
	EFI_DEVICE_PATH			*FullPath, *Node;
	FILEPATH_DEVICE_PATH	*FileNode;
	size_t					DevLen, FileNameLen;

	/*
	 *  Determine the current device path length sans end node
	 */
	Node = Dev;
	DevLen = 0;
	while ( ! IsDevicePathEndType( Node ) ) {
		DevLen += DevicePathNodeLength( Node );
		Node = NextDevicePathNode( Node );
	}

	/*
	 *  Allocate memory for full device path
	 */
	FileNameLen = (wcslen( FilePath ) + 1) * sizeof(wchar_t);
	if ( FullPath = malloc( DevLen +
							SIZE_OF_FILEPATH_DEVICE_PATH +
							FileNameLen +
							sizeof(EFI_DEVICE_PATH) ) ) {
		/*
		 *  Copy the Dev portion
		 */
		memcpy( FullPath, Dev, DevLen );

		/*
		 *  Create a pointer to the filepath node and fill it in.
		 */
		FileNode = (FILEPATH_DEVICE_PATH*) ((char*)FullPath + DevLen);
		FileNode->Header.Type = MEDIA_DEVICE_PATH;
		FileNode->Header.SubType = MEDIA_FILEPATH_DP;
		SetDevicePathNodeLength (&FileNode->Header, FileNameLen + SIZE_OF_FILEPATH_DEVICE_PATH);
        memcpy( FileNode->PathName, FilePath, FileNameLen );

		/*
		 *  Create the end node
		 */
		Node = NextDevicePathNode( &FileNode->Header );
		SetDevicePathEndNode( Node );
	}

	return ( FullPath );
}

/*
 *  Name:
 *      _GetFileDevicePath
 *
 *  Description:
 *      Convert a fully qualified Unicode filename into a device path.
 *
 *  Arguments:
 *      Fully qualified Unicode filename
 *      Pointer to storage for returned device path.
 *
 *  Returns:
 *      EFI status codes
 */
EFI_STATUS
_GetFileDevicePath(
	IN	CHAR16			*File,
	OUT	EFI_DEVICE_PATH	**FullDevPath
	)
{
	EFI_STATUS	Status = EFI_INVALID_PARAMETER;
	wchar_t		*Name;

	/*
	 *  Start by finding and converting the device name to ascii
	 */
	Name = wcschr( File, L':' );
	if ( Name++ ) {
		EFI_DEVICE_PATH	*DevicePath;
		int				devlen = (int)(Name - File);
		char			*DeviceName = calloc( (devlen + 1), sizeof(char) );

		if ( DeviceName ) {
			int		i;
			char	*tmp;

			/*
			 *  Convert it to ASCII
			 */
			wcstombs( DeviceName, File, devlen );

			/*
			 *  Now find the device path
			 */
			for ( i = 0; tmp = _GetFileMapping( i, (VOID*)&DevicePath ); i++ ) {
				if ( strcmp( tmp, DeviceName ) == 0 )
					break;
			}

			/*
			 *  See if we found a match
			 */
			if ( tmp ) {
				/*
				 *  Append a file path node to the device node.
				 */
				*FullDevPath = CreateFullDevPath( DevicePath, Name );
				if ( *FullDevPath ) {
					Status = EFI_SUCCESS;
				} else {
					Status = EFI_OUT_OF_RESOURCES; 
				}
			}

			/*
			 *  Free the ASCII name
			 */
			free( DeviceName );
		} else {
			Status = EFI_OUT_OF_RESOURCES; 
		}
	}
	return ( Status );
}
