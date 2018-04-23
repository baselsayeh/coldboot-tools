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

    lock.c

Abstract:

    Implements FLOCK



Revision History

--*/


#include "lib.h"



#pragma RUNTIME_CODE(RtAcquireLock)
VOID
RtAcquireLock (
    IN FLOCK    *Lock
    )
/*++

Routine Description:

    Raising to the task priority level of the mutual exclusion
    lock, and then acquires ownership of the lock.
    
Arguments:

    Lock        - The lock to acquire
    
Returns:

    Lock owned

--*/
{
    if (LibRuntimeRaiseTPL != NULL) {
        Lock->OwnerTpl = LibRuntimeRaiseTPL(Lock->Tpl);
    } else if (BS->RaiseTPL != NULL) {
        Lock->OwnerTpl = BS->RaiseTPL(Lock->Tpl);
    }
    Lock->Lock += 1;
    ASSERT (Lock->Lock == 1);
}


#pragma RUNTIME_CODE(RtReleaseLock)
VOID
RtReleaseLock (
    IN FLOCK    *Lock
    )
/*++

Routine Description:

    Releases ownership of the mutual exclusion lock, and
    restores the previous task priority level.
    
Arguments:

    Lock        - The lock to release
    
Returns:

    Lock unowned

--*/
{
    EFI_TPL     Tpl;

    Tpl = Lock->OwnerTpl;
    ASSERT(Lock->Lock == 1);
    Lock->Lock -= 1;
    if (LibRuntimeRestoreTPL != NULL) {
        LibRuntimeRestoreTPL(Tpl);
    } else if (BS->RestoreTPL != NULL) {
        BS->RestoreTPL (Tpl);
    }
}
