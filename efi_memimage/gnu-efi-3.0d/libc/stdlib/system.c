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

    system.c
    
Abstract:

    spawn an EFI shell to run the specified efi application or batch script.


Revision History

--*/

#include <atk_libc.h>
#include <efi_interface.h>
#include <stdio.h>   /* for fprintf() and stderr                        */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <shell.h>

#define	DEFAULT_SHELL	"nshell.efi"

static EFI_GUID ShellProto = SHELL_INTERFACE_PROTOCOL;

/*
 *  Name:
 *      wsystem
 *
 *  Description:
 *      Pass a command to the shell
 *
 *  Arguments:
 *      Unicode command line
 *
 *  Returns:
 *      return value of the command
 */
int
wsystem( const wchar_t *string )
{
	char		*shell;
	CHAR16		*UnicodeShell;
	EFI_HANDLE	ImageHandle;
	EFI_STATUS	Status;
	size_t		ShellSize;
	int			Ret = -1; // assume the worst

	/*
	 *  If a SHELL has been set in the environment, use it.
	 *  Otherwize, use the default
	 */
	if ( (shell = getenv("SHELL")) == NULL ) {
		shell = DEFAULT_SHELL;
	}

	/*
	 *  Convert to Unicode
	 */
	ShellSize = strlen( shell ) + 1;
	UnicodeShell = calloc( ShellSize, sizeof(CHAR16) );
	if ( UnicodeShell && mbstowcs( UnicodeShell, shell, ShellSize ) > 0) {

		/*
		 *  See if we can load the shell
		 */
		Status = LoadImage( UnicodeShell, &ImageHandle );
		if ( ! EFI_ERROR( Status ) ) {
			CHAR16	*LoadOptions;
			size_t	OptionsSize;
			wchar_t	*tmp, *Cwd;

			/*
			 *  Get current working directory and convert /'s to \'s.
			 *  This assumes nshell behavior - all shells may not work
			 *  like that - heavy sigh.
			 */
			Cwd = wgetcwd( NULL, 0 );
			for( tmp = Cwd; *tmp; tmp++ ) {
				if (*tmp == L'/')
					*tmp = L'\\';
			}

			/*
			 *  Create LoadOptions
			 */
			OptionsSize = (wcslen( UnicodeShell ) + 1 +
						   wcslen( string ) + 1 +
						   wcslen( Cwd ) + 1) *
						   sizeof(CHAR16);

			LoadOptions = malloc( OptionsSize );
			if ( LoadOptions ) {
				EFI_BOOT_SERVICES	*BtSrv;
				EFI_SHELL_INTERFACE *ShellIface;

				wcscpy( LoadOptions, UnicodeShell );
				wcscat( LoadOptions, L" " );
				wcscat( LoadOptions, string );

				/*
				 *  Add the CWD, as a separate string, to the end of the
				 *  load options.
				 */
				tmp = &LoadOptions[ wcslen( LoadOptions ) + 1 ];
				wcscpy( tmp, Cwd );

				/*
				 *  Run the command
				 */
				Status = StartImage( ImageHandle,
									 (UINT32)OptionsSize,
									 LoadOptions,
									 NULL,
									 0,
									 NULL );
				/*
				 *  Free options and check for errors
				 */
				free( LoadOptions );

				/*
				 *  Set the return value.
				 *
				 *  NOTE:  This will truncate the error code if the sizeof
				 *         Status is > sizeof int.
				 */
				Ret = (int)Status;

				/*
				 *  Finally, if there's a shell interface protocol on the
				 *  shell's image handle, remote it.
				 */
				BtSrv = _GetBootServices();
				Status = BtSrv->HandleProtocol( ImageHandle,
												&ShellProto,
												(VOID*)&ShellIface );
				if ( ! EFI_ERROR( Status ) ) {
					(void)BtSrv->UninstallProtocolInterface( ImageHandle,
															 &ShellProto,
															 ShellIface );	
					BtSrv->FreePool(ShellIface);
				}
			}
			free(Cwd);
    } else {
  		fprintf(stderr, "wsystem: can not load image %s\r\n",shell);
    }

		free( UnicodeShell );
	}

	return ( Ret );
}

/*
 *  Name:
 *      system
 *
 *  Description:
 *      Pass a command to the shell
 *
 *  Arguments:
 *      Ascii command line
 *
 *  Returns:
 *      return value of the command
 */
int
system( const char *string )
{
	wchar_t	*Unicode;
	size_t	Size;
	int		Ret = -1;

	Size = strlen( string );
	Unicode = calloc( Size + 1, sizeof(wchar_t) );
	if ( Unicode ) {
		if ( mbstowcs( Unicode, string, Size ) > 0 ) {
			Ret = wsystem( Unicode );
			free( Unicode );
		}
	}

	return ( Ret );
}
