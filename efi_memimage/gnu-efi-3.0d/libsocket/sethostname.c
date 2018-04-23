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

#include <atk_libc.h>
#include <atk_guid.h>
#include <wchar.h>
#include <stdlib.h>
#ifndef KERNEL
#define KERNEL
#include <errno.h>
#undef KERNEL
#else
#include <errno.h>
#endif

/*++

Module Name:

    sethostname.c
    
Abstract:

    Map FreeBSD sethostname call to EFI Interface


Revision History

--*/

int
sethostname(
	char	*name,
	int		namelen
	)
/*++

Routine Description:

    Set the hostname for this system.

Arguments:
    
	name		- Pointer to hostname.
	namelen		- Length of name

Returns:

	0 on success, -1 if not set

--*/
{
	EFI_STATUS	Status;
	wchar_t		*pHost;

	//
	//  Allocate memory for conversion
	//

	pHost = (wchar_t*)calloc (namelen + 1, sizeof(wchar_t));
	if (pHost == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	//
	//  Convert name.
	//
	mbstowcs (pHost, name, namelen + 1);
	pHost[ namelen ] = 0;	// make sure it is terminated

	//
	//  Set in EFI environment
	//
	Status = _LIBC_EFISystemTable->RuntimeServices->SetVariable(
							L"HOSTNAME",
							&_LIBC_VendorGuid,
							EFI_VARIABLE_BOOTSERVICE_ACCESS,
							(namelen + 1) * sizeof(wchar_t),
							pHost
							);
	free (pHost);

	if (EFI_ERROR(Status)) {
		errno = EFAULT; 	// Do you have any better ideas???
		return (-1);
	} else {
		//
		//  Put it in the libc environment
		//
		setenv ("HOSTNAME", name, TRUE);
		return (0);
	}
}
