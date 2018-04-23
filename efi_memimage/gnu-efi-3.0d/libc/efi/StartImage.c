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

    StartImage.c
    
Abstract:

    C library interface to start a loaded EFI image.


Revision History

--*/

#include <efi.h>
#include <efi_interface.h>

/*
 *  Name:
 *      StartImage
 *
 *  Description:
 *      Start a loaded EFI image
 *
 *  Arguments:
 *      ImageHandle
 *      LoadOptionsSize
 *      LoadOptions (optional)
 *      SystemTable (optional)
 *      Exit data size
 *      Exit data (optional)
 *
 *  Returns:
 *      EFI status codes
 */
EFI_STATUS
StartImage(
	IN	EFI_HANDLE			ImageHandle,
	IN	UINT32				LoadOptionsSize,
	IN	CHAR16				*LoadOptions,
	IN	EFI_SYSTEM_TABLE	*SystemTable,
	OUT	UINTN				*ExitDataSize,
	OUT CHAR16				**ExitData
	)
{
	/*
	 *  See if we need to get the loaded image protocol
	 */
	if ( LoadOptionsSize || SystemTable ) {
		EFI_LOADED_IMAGE	*Image;
		EFI_STATUS			Status;

        /*
         *  Get image protocol
         */
        Status = _GetBootServices()->HandleProtocol (
                        ImageHandle,
                        &_LibcLoadedImageProtocol,
                        (VOID*)&Image
                        );

        if ( EFI_ERROR( Status ) ) {
            return (Status);
        }

        /*
         *  Make requested adjustments
         */
		if ( LoadOptionsSize ) {
			if ( LoadOptions == NULL ) {
				return ( EFI_INVALID_PARAMETER );
			} else {
				Image->LoadOptions = LoadOptions;
				Image->LoadOptionsSize = LoadOptionsSize;
			}
		}

        if ( SystemTable ) {
        	Image->SystemTable = SystemTable;
        }
	}
	return ( _GetBootServices()->StartImage( ImageHandle, ExitDataSize, ExitData ) );
}
