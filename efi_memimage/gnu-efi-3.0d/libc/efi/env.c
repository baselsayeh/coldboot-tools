/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <efi.h>
#include <efi_interface.h>
#include <stdlib.h>
#include <ctype.h>
#include <wctype.h>
#include <string.h>

#define	MAX_ENVIRONMENT_NAME	256
#define MAX_ENVIRONMENT_DATA	4098

//
//  Private storage for any shell filesystem mappings
//
#define DEVICE_PATH_MAPPING_ID  \
    { 0x47c7b225, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define MAX_SHELL_MAPPING	32

typedef struct {
	char	*MapName;
	void	*DevicePath;
} ShellDevMap_t;

static ShellDevMap_t	ShellDevMap[ MAX_SHELL_MAPPING ] = { {0, 0}, };

static EFI_GUID	ShellDevPathMapGuid = DEVICE_PATH_MAPPING_ID;

//
//  This table can be filled in to "seed" the environment.
//
static	char	*EnvSeed[] = {
	NULL,
};

char	**environ = EnvSeed;

static int
IsUnicodeStr( char *AsciiName, CHAR16 *Data, UINTN DataSize)
{
	UINTN i;

	for ( i = 0; i < DataSize / sizeof(CHAR16); i++ ) {
		if ( !iswprint(Data[ i ]) ) {
			//
			//  Sometimes the size includes the termination character
			//
			if ( (Data[ i ] == 0) && (i == ((DataSize / sizeof(CHAR16)) - 1)) )
				continue;
			else
				return ( FALSE );
		}
	}

	//
	//  Typecast will limit lengyth of string to be converted
	//
	wcstombs( AsciiName, Data, (UINT32)DataSize);
	return ( TRUE );
}

static int
IsAsciiStr( char *AsciiName, char *Data, UINTN DataSize)
{
	UINTN i;

	for ( i = 0; i < DataSize; i++ ) {
		if ( !isprint(Data[ i ]) ) {
			//
			//  Sometimes the size includes the termination character
			//
			if ( (Data[ i ] == 0) && (i == (DataSize - 1)) )
				continue;
			else
				return ( FALSE );
		}
	}

	strcpy( AsciiName, Data );
	return ( TRUE );
}

void
_InitializeEnv( EFI_SYSTEM_TABLE *pST )
{
	EFI_STATUS				Status;
	CHAR16					*VariableName;
	void					*VariableData;
	char					*AsciiName;
	char					*AsciiData;
	UINTN					VariableNameSize;
	UINTN					VariableDataSize;
	int						ShellMapIndex;
	EFI_GUID				VendorGuid;
	EFI_RUNTIME_SERVICES	*pRT;

	pRT = pST->RuntimeServices; 

	//
	//  Allocate memory for conversions
	//
	VariableName = (CHAR16*)calloc(MAX_ENVIRONMENT_NAME, sizeof(CHAR16));
	if ( VariableName == NULL ) {
		return;
	}

	VariableData = (CHAR16*)calloc(MAX_ENVIRONMENT_DATA, sizeof(CHAR16));
	if ( VariableData == NULL ) {
		free( VariableName );
		return;
	}

	AsciiName = (char*)calloc(MAX_ENVIRONMENT_NAME, sizeof(char));
	if ( AsciiName == NULL ) {
		free( VariableData );
		free( VariableName );
		return;
	}

	AsciiData = (char*)calloc(MAX_ENVIRONMENT_DATA, sizeof(char));
	if ( AsciiData == NULL ) {
		free( VariableData );
		free( VariableName );
		free( AsciiName );
		return;
	}

	//
	//  set all variable found
	//
	ShellMapIndex = 0;
	do {
		VariableNameSize = MAX_ENVIRONMENT_NAME;
		Status = pRT->GetNextVariableName( &VariableNameSize,
										   VariableName,
										   &VendorGuid );
		if ( ! EFI_ERROR( Status ) ) {
			VariableDataSize = MAX_ENVIRONMENT_DATA;
			Status = pRT->GetVariable( VariableName,
									   &VendorGuid,
									   NULL,
									   &VariableDataSize,
									   VariableData );

			if ( ! EFI_ERROR( Status ) ) {
				//
				//  We'll first look for ShellDevPathMap GUIDs so we can
				//  map shell filesystem names.
				//
				if ( memcmp( &VendorGuid, &ShellDevPathMapGuid, sizeof(VendorGuid) ) == 0 ) {
					ShellDevMap_t	*pDev = &ShellDevMap[ ShellMapIndex ];

					//
					//  Convert mapping to ASCII
					//
					wcstombs( AsciiName, VariableName, (UINT32)VariableNameSize);

					//
					//  Allocate memory for name and device path
					//
					pDev->MapName = malloc( (size_t)(strlen(AsciiName) + 2)); // ":\000"
					if ( pDev->MapName == NULL )
						continue;

					pDev->DevicePath = malloc( (size_t)VariableDataSize );
					if ( pDev->DevicePath == NULL )
						continue;

					//
					//  Store name and device path
					//
					strcpy( pDev->MapName, AsciiName );
					strcat( pDev->MapName, ":");
					bcopy( VariableData, pDev->DevicePath, (size_t)VariableDataSize );

					//
					//  Setup for next entry looking for overflow.
					//
					ShellMapIndex++;
					if ( ShellMapIndex == MAX_SHELL_MAPPING )
						ShellMapIndex--;

					continue;
				}

				//
				//  EFI variables can be in ascii, unicode or raw data.
				//  we are only interested in ascii and unicode strings
				//
				if ( IsUnicodeStr( AsciiData, VariableData, VariableDataSize) ||
					 IsAsciiStr( AsciiData, VariableData, VariableDataSize) ) {
			 
					//
					//  Convert and set variable.  Overwrite any seeded variables
					//  Typecast will limit lengyth of string to be converted
					//
					wcstombs( AsciiName, VariableName, (UINT32)VariableNameSize);
					setenv( AsciiName, AsciiData, TRUE );
				}
			}
		}
	} while (Status == EFI_SUCCESS);

	free( VariableData );
	free( VariableName );
	free( AsciiName );
	free( AsciiData );
}

char *
_GetFileMapping(
	int		Index,
	void	**DevPath
	)
{
	if ( (Index < 0)                  ||
		 (Index >= MAX_SHELL_MAPPING) ||
		 (ShellDevMap[ Index ].MapName == NULL) ) {

		*DevPath = NULL;
		return ( NULL );

	} else {
		*DevPath = ShellDevMap[ Index ].DevicePath;
		return ( ShellDevMap[ Index ].MapName );
	}
}

int
_AddFileMapping(
	char	*MapName,
	void	*DevPath
	)
{
	int	i;

	for ( i = 0; i < MAX_SHELL_MAPPING; i++ ) {
		if ( ShellDevMap[ i ].MapName == NULL ) {
			ShellDevMap[ i ].MapName = MapName;
			ShellDevMap[ i ].DevicePath = DevPath;
			return ( 0 );
		}
	}

	return ( 1 );
}
