/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    Acpi.c
    
Abstract:

    Code to find System Configuration Tables

    This code should ONLY be used to populate the EFI System Table.
    All other code should get these pointers from the EFI System Table.
    This code is only for platforms that have a legacy BIOS. This is
    in no way required by EFI.

Revision History

--*/

#include "efi.h"
#include "efilib.h"

#define ACPI_RSD_PTR      0x2052545020445352
#define MPS_PTR           EFI_SIGNATURE_32('_','M','P','_')
#define SMBIOS_PTR        EFI_SIGNATURE_32('_','S','M','_')

#define EBDA_BASE_ADDRESS 0x40e

VOID *
LibFindAcpiRsdPtr (
    VOID
    )
{
    UINT64                          Address;
    UINTN                           i;

    //
    // First Seach 0x0e0000 - 0x0fffff for RSD Ptr
    //
    for (Address = 0xe0000; Address < 0xfffff; Address += 0x10) {
        if (*(UINT64 *)(Address) == ACPI_RSD_PTR) {
            return (VOID *)Address;
        }
    }

    //
    // Search EBDA
    //

    Address = (*(UINT16 *)(EBDA_BASE_ADDRESS)) << 4;
    for (i = 0; i < 0x400 ; i += 16) {
        if (*(UINT64 *)(Address + i) == ACPI_RSD_PTR) {
            return (VOID *)Address;
        }
    }
    return NULL;
}


VOID *
LibFindSMBIOSPtr (
    VOID
    )
{
    UINT64                          Address;

    //
    // First Seach 0x0f0000 - 0x0fffff for SMBIOS Ptr
    //
    for (Address = 0xf0000; Address < 0xfffff; Address += 0x10) {
        if (*(UINT32 *)(Address) == SMBIOS_PTR) {
            return (VOID *)Address;
        }
    }
    return NULL;
}

VOID *
LibFindMPSPtr (
    VOID
    )
{
    UINT64                          Address;
    UINTN                           i;

    //
    // First Seach 0x0e0000 - 0x0fffff for MPS Ptr
    //
    for (Address = 0xe0000; Address < 0xfffff; Address += 0x10) {
        if (*(UINT32 *)(Address) == MPS_PTR) {
            return (VOID *)Address;
        }
    }

    //
    // Search EBDA
    //

    Address = (*(UINT16 *)(EBDA_BASE_ADDRESS)) << 4;
    for (i = 0; i < 0x400 ; i += 16) {
        if (*(UINT32 *)(Address + i) == MPS_PTR) {
            return (VOID *)Address;
        }
    }
    return NULL;
}

