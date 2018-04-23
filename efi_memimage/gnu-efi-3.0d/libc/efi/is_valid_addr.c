/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
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
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/*++

Module Name:

    validate_address.c

Abstract:

    This file contains functions to validate an address in the EFI 
    environment.

Revision History

--*/


#include <atk_libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inttypes.h>
#include <machine/param.h>
#include <assert.h>

// defining KERNEL before errno.h allows us to assign errno
#define KERNEL
#include <errno.h>


//#define DEBUG_MEMVAL
#ifdef DEBUG_MEMVAL
static char fmt[] = "Phys start 0x%016qx, Size 0x%016qx\n";
static char fmt2[] = "Validate range start 0x%016qx, end 0x%016qx\n";
#endif

static 
BOOLEAN
ValidType(
    UINT32 Type
    )
{
    switch( Type ) {
    case EfiReservedMemoryType:
    case EfiUnusableMemory:
    case EfiMemoryMappedIO:
    case EfiMemoryMappedIOPortSpace:
        return FALSE;
    default:
        return TRUE;
    }
}
    

int
is_valid_addr( 
    uintptr_t address, 
    size_t    size
    )
/*++

Routine Description:

    Check whether a specified address is in the EFI address map.

    Note that this function assumes it gets to run atomically - if it is
    pre-empted between determining the memory map size and actually
    retrieving the memory map, it may fail to validate an address that it 
    should validate.

Arguments:

    address  - start address of the range to be validated
    size     - size of the range to be validated

Returns:

    TRUE if all of the addresses in the specified range are defined as
    valid addresses in the EFI address map, FALSE otherwise.  If an
    error is encountered while checking the addresses, errno is set 
    and the function returns FALSE.

--*/
{
    EFI_STATUS            Status         = EFI_SUCCESS;
    EFI_BOOT_SERVICES     *myBS          = NULL;
    UINTN                 MemoryMapSize  = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap     = NULL;
    UINTN                 MapKey;
    UINTN                 DescriptorSize = 0;
    UINT32                DescriptorVersion;
    EFI_MEMORY_DESCRIPTOR *CurrentDesc   = NULL;
    UINTN                 NumDescriptors = 0;
    UINTN                 i;
    EFI_PHYSICAL_ADDRESS  RangeStart;
    EFI_PHYSICAL_ADDRESS  RangeEnd;
    EFI_PHYSICAL_ADDRESS  CurrentEnd;
    int                   IsValid        = FALSE;


    errno = 0;

    assert( _LIBC_EFISystemTable != NULL );
    myBS = _LIBC_EFISystemTable->BootServices;
    assert( myBS != NULL );

    //
    //  Get the buffer size needed for the memory map
    //
    Status = myBS->GetMemoryMap(
                       &MemoryMapSize,
                       NULL,
                       &MapKey,
                       &DescriptorSize,
                       &DescriptorVersion
                       );
    if ( Status == EFI_BUFFER_TOO_SMALL ) {
        //
        //  Add space for one extra descriptor in case 
        //  our calloc grows the memory map
        //
        MemoryMapSize += DescriptorSize;
        MemoryMap = calloc( (UINT32)MemoryMapSize, sizeof(UINT8) );
        if ( !MemoryMap ) {
            errno = ENOMEM;
            goto ErrExit;
        }
    } else if ( EFI_ERROR( Status ) ) {
        errno = _EfiErrToErrno( Status );
        goto ErrExit;
    } else {
        errno = EINVAL;
        goto ErrExit;
    }

    //
    //  Get the memory map
    //
    Status = myBS->GetMemoryMap(
                       &MemoryMapSize,
                       MemoryMap,
                       &MapKey,
                       &DescriptorSize,
                       &DescriptorVersion
                       );
    if ( EFI_ERROR(Status) ) {
        errno = _EfiErrToErrno( Status );
        goto ErrExit;
    }

    //
    //  Iterate through the memory descriptors, looking for one whose
    //  range contains the specified addresses
    //
    RangeStart = (EFI_PHYSICAL_ADDRESS)address;
    RangeEnd = RangeStart + size - 1;
    CurrentDesc = MemoryMap;
    NumDescriptors = MemoryMapSize / DescriptorSize;


#ifdef DEBUG_MEMVAL
fprintf( stderr, fmt2, RangeStart, RangeEnd );
#endif

    for ( i = 0; i < NumDescriptors; i++ ) {

#ifdef DEBUG_MEMVAL
fprintf( stderr,                                                       
             fmt,                                                         
             CurrentDesc->PhysicalStart,                                 
             CurrentDesc->NumberOfPages * PAGE_SIZE );                  
#endif

        CurrentEnd = CurrentDesc->PhysicalStart 
                     + (CurrentDesc->NumberOfPages * PAGE_SIZE);

        if ( RangeStart >= CurrentDesc->PhysicalStart &&
             RangeStart <= CurrentEnd &&
             ValidType( CurrentDesc->Type ) ) {
            if ( RangeEnd <= CurrentEnd ) {
                IsValid = TRUE;
                break;
            } else {
                RangeStart = CurrentEnd + 1;
            }
        } else if ( RangeEnd <= CurrentEnd && 
                    RangeEnd >= CurrentDesc->PhysicalStart &&
                    ValidType( CurrentDesc->Type ) ) {
            RangeEnd = CurrentDesc->PhysicalStart - 1;
        }

        //
        //  Get the next descriptor  (using macro from efiapi.h)
        //
        CurrentDesc = NextMemoryDescriptor( CurrentDesc, DescriptorSize );
    }

ErrExit:
    return IsValid;
}
