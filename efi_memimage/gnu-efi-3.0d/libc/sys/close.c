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

    close.c
    
Abstract:

    Functions implementing the standard "close" system call interface


Revision History

--*/


#include <efi_interface.h>
#include <unistd.h>
#include <stdlib.h>
#include "./filedesc.h"
#define KERNEL
#include <errno.h>
#undef KERNEL

#include <libc_debug.h>
#include <assert.h>

//
//  Name:
//      close
//
//  Description:
//      BSD close interface for libc
//
//  Arguments:
//      File Descriptor (index into file descriptor table)
//
//  Returns:
//      0 on success, -1 otherwise
//

int
close( int fd )
{
    EFILIBC_CLOSE_T Close;
    EFI_STATUS      Status;
    VOID            *DevSpecific;
    int             Refs, UnlinkFlagCount;
    int             ret = 0;

    /*
     *  Remove a reference.  If this is the last one, release the
     *  file descriptor.
     */
    Refs = _RemoveFileReference( fd );
    if ( Refs < 0 ) {
        /*
         *  Something is wrong.  Probably a bad fd
         */
        DPRINT((L"close: Error removing reference\n"));
        ret = -1;
        goto Done;
    } else if ( Refs > 0 ) {
    	/*
    	 *  Outstanding references on descriptor.  Only free the fd slot.
    	 */
        DPRINT((L"close: %d references remain on fd %d\n", Refs, fd));

    	_ReleaseFileDescriptorSlot( fd );
    	ret = 0;
    	goto Done;
    }

	if ( _LIBC_GetOpenFileDevSpecific( fd, &DevSpecific ) != 0 ) {
		ret = -1;
		goto Done;
	}

    /*
     *  No outstanding references.  Check unlink flag and remove if needed.
     */
    if ( _GetUnlinkFlagCount( fd, &UnlinkFlagCount ) ) {

    	DPRINT((L"close: error getting unlink flags\n" ));
		goto Done;

    } else if ( UnlinkFlagCount == 1 ) {

		/*
		 *  An unlink flag is set and this is the last file reference.
		 *  EFI_UnlinkFile will do close processing.
		 */
    	DPRINT((L"close: unlinking file\n" ));

        Status = EFI_UnlinkFile( DevSpecific );
        if ( EFI_ERROR(Status) ) {
    		DPRINT((L"close: EFI_UnlinkFile returned 0x%x\n", Status));
		    ret = -1;
			goto Done;
        }
		
		free ( DevSpecific ); 

		/* fall through to release file descriptor */

    } else {

		/*
		 *  No references and no pending unlink.  Call device specific close
		 */

		DPRINT((L"close: close file\n" ));

		if ( _GetOpenFileClose( fd, &Close ) != 0 ) {
			ret = -1;
			goto Done;
		}
		assert ( Close != NULL );

		Status = Close( DevSpecific );
		if ( EFI_ERROR( Status ) ) {
			DPRINT((L"EFI error encountered in close: %d\n", Status ));
			errno = _EfiErrToErrno( Status );
			ret = -1;

			/* fall through to release file descriptor */
		}
	}

	_ReleaseFileDescriptor( fd );
	DPRINT((L"close: complete\n"));

Done:
    return ret;
}
