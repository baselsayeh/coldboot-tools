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

    LoadImage.c
    
Abstract:

    C library interface to load an EFI image.


Revision History

--*/

#include <efi.h>
#include <efi_interface.h>
#include <stdlib.h>

/*
 *  Name:
 *      LoadImage
 *
 *  Description:
 *      Load the specified file as an EFI image
 *
 *  Arguments:
 *      Unicode name of the file to load
 *      Pointer to storage for returned image handle of image
 *
 *  Returns:
 *      EFI status codes
 */
EFI_STATUS
LoadImage(
	IN	CHAR16		*Path,
	OUT	EFI_HANDLE	*ImageHandle
	)
{
	EFI_DEVICE_PATH	*FileDevicePath;
	EFI_STATUS		Status;
	CHAR16			*FullPath;

	/*
	 *  Create a fully qualified path to the file.  If there are no device
	 *  or directory components (i.e. just a filename), this will search the
	 *  directories in PATH.  Indicate we'll accept assumed .efi extensions.
	 */
	FullPath = ResolveFilename( Path );
	if ( FullPath == NULL ) {
		return ( EFI_NOT_FOUND );
	}

	/*
	 *  Get the device path for the file.
	 */
	Status = _GetFileDevicePath( FullPath, &FileDevicePath );
	if ( ! EFI_ERROR( Status ) ) {
		/*
		 *  Load the image
		 */
		Status = _GetBootServices()->LoadImage(
										FALSE,
										_GetImageHandle(),
										FileDevicePath,
										NULL,
										0,
										ImageHandle
										);
		/*
		 *  Need to free the device path
		 */
		free( FileDevicePath );
	}

	/*
	 *  Need to free the full filename
	 */
	free( FullPath );
	return Status;
}
