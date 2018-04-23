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

#ifndef _LOCALE_PROTOCOL_H_
#define _LOCALE_PROTOCOL_H_

#include <efi.h>
#include <efilib.h>
#include <runetype.h>

/*
 *  EFI locale protocol
 */

// {0D58ED19-DAE9-401d-A2DC-02EC387E36D6}
#define EFI_LOCALE_PROTOCOL \
    { 0x0d58ed19, 0xdae9, 0x401d, 0xa2, 0xdc, 0x2, 0xec, 0x38, 0x7e, 0x36, 0xd6 }

#define EFI_LOCALE_INTERFACE_REVISION   0x00010000

#define EFI_LOCALE_ERR(val)             EFIERR_OEM(0x50000 | val)

#define EFI_LOCALE_FAILURE              EFI_LOCALE_ERR(1)

INTERFACE_DECL(_EFI_LOCALE);


/*
 *  locale protocol interface
 */

typedef
char *
(EFIAPI *EFI_GETLOCALE) (
    );

typedef
rune_t
(EFIAPI *EFI_SGETRUNE) (
    IN      const char  *string,
    IN      size_t      n,
    IN OUT  char const  **result
    );

typedef
int
(EFIAPI *EFI_SPUTRUNE) (
    IN      rune_t      c,
    IN      char        *string,
    IN      size_t      n,
    IN OUT  char        **result
    );

typedef
int
(EFIAPI *EFI_GETMBCURMAX) (
    );

typedef
_ToUnicodeMap *
(EFIAPI *EFI_GETUNICODEMAP) (
    );

typedef
int
(EFIAPI *EFI_GETUNICODEMAPENTRYCOUNT) (
    );

typedef
UINT8 *
(EFIAPI *EFI_GETCTYPEDATA) (
    );

typedef
int
(EFIAPI *EFI_GETCTYPEDATASIZE) (
    );

typedef
UINT8 *
(EFIAPI *EFI_GETCOLLATEDATA) (
    );

typedef
int
(EFIAPI *EFI_GETCOLLATEDATASIZE) (
    );
    
typedef
UINT8 *
(EFIAPI *EFI_GETTIMEDATA) (
    );

typedef
int
(EFIAPI *EFI_GETTIMEDATASIZE) (
    );
    
typedef
EFI_STATUS
(EFIAPI *EFI_SETINVALIDRUNE) (
    rune_t val
    );
    
typedef struct _EFI_LOCALE {
    UINT64                          Revision;
    EFI_GETLOCALE                   getlocale;
    EFI_SGETRUNE                    sgetrune;
    EFI_SPUTRUNE                    sputrune;
    EFI_GETMBCURMAX                 getmbcurmax;
    EFI_GETUNICODEMAP               getunicodemap;
    EFI_GETUNICODEMAPENTRYCOUNT     getunicodemapentrycount;
    EFI_GETCTYPEDATA                getctypedata;
    EFI_GETCTYPEDATASIZE            getctypedatasize;
    EFI_GETCOLLATEDATA              getcollatedata;
    EFI_GETCOLLATEDATASIZE          getcollatedatasize;
    EFI_GETTIMEDATA                 gettimedata;
    EFI_GETTIMEDATASIZE             gettimedatasize;
    EFI_SETINVALIDRUNE              setinvalidrune;
} EFI_LOCALE_INTERFACE;


#endif /* _LOCALE_PROTOCOL_H_ */
