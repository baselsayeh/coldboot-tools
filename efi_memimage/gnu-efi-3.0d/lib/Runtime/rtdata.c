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

    data.c

Abstract:

    EFI library global data



Revision History

--*/

#include "lib.h"


//
// These globals are runtime globals
//
// N.B. The Microsoft C compiler will only put the data in the
// right data section if it is explicitly initialized..
//

#pragma BEGIN_RUNTIME_DATA()

//
// RT - pointer to the runtime table
//

EFI_RUNTIME_SERVICES    *RT = NULL;

//
// LibStandalone - TRUE if lib is linked in as part of the firmware.
// N.B. The EFI fw sets this value directly
//

BOOLEAN  LibFwInstance = FALSE;

//
// EFIDebug - Debug mask
//

UINTN    EFIDebug    = EFI_DBUG_MASK;

//
// LibRuntimeDebugOut - Runtime Debug Output device
//

SIMPLE_TEXT_OUTPUT_INTERFACE    *LibRuntimeDebugOut = NULL;

//
// LibRuntimeRaiseTPL, LibRuntimeRestoreTPL - pointers to Runtime functions from the 
//                                            Boot Services Table
//

EFI_RAISE_TPL   LibRuntimeRaiseTPL   = NULL;
EFI_RESTORE_TPL LibRuntimeRestoreTPL = NULL;

