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

    map.c
    
Abstract:

   	Device and protocol mapping support. 


Revision History

--*/

#include <efi_interface.h>
#include <sys/syslimits.h>
#include <string.h>

#include "map.h"

#include <libc_debug.h>

//
//  Protocol Mapping Table
//
static EFILIBC_MAPPROTOCOL_T		MapProtocolTable[MAX_MAPPROTOCOL];

//
//  Device Mapping Table
//
static EFILIBC_MAPDEVICE_T			MapDeviceTable[MAX_MAPDEVICE];

//
//  === Internal Interfaces ===
//


//
//  Name:
//      _InitializeMapProtocolTable
//
//  Description:
//		Initializes all entries in the Map Protocol Table.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//
VOID
_InitializeMapProtocolTable(
	VOID
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPPROTOCOL ; i++ )
	{
		_RemoveMapProtocolEntry( i ) ;
	}
}


//
//  Name:
//      _InitializeMapDeviceTable
//
//  Description:
//		Initializes all entries in the Map Protocol Table.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//
VOID
_InitializeMapDeviceTable(
	VOID
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPDEVICE ; i++ )
	{
		_RemoveMapDeviceEntry( i ) ;
	}
}


//
//  Name:
//      _DeInitializeMapProtocolTable
//
//  Description:
//		Removes all used entries in the Map Protocol Table.  This function acts 
//		as a placeholder in case if in the future something additional needs
//		to be done when cleaning up the Map Protocol Table.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//
VOID
_DeInitializeMapProtocolTable(
	VOID
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPPROTOCOL ; i++ )
	{
		if( MapProtocolTable[i].UsedEntry == TRUE )
		{
			_RemoveMapProtocolEntry( i ) ;
		}
	}
}


//
//  Name:
//      _DeInitializeMapDeviceTable
//
//  Description:
//		Removes all used entries in the Map Device Table.  This function acts 
//		as a placeholder in case if in the future something additional needs
//		to be done when cleaning up the Map Device Table.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//
VOID
_DeInitializeMapDeviceTable(
	VOID
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPDEVICE ; i++ )
	{
		if( MapDeviceTable[i].UsedEntry == TRUE )
		{
			_RemoveMapDeviceEntry( i ) ;
		}
	}
}


//
//  Name:
//      _RemoveMapProtocolEntry
//
//  Description:
//		Removes the specified entry from the Map Protocol Table.
//
//  Arguments:
//		EntryNum:	The index of the entry to remove	
//
//  Returns:
//		None
//
static VOID
_RemoveMapProtocolEntry(
	UINT32			EntryNum
	)
{
	EXCLUSION_LOCK_DECL;

	EXCLUSION_LOCK_TAKE();

	MapProtocolTable[EntryNum].Guid = NULL ;
	MapProtocolTable[EntryNum].OpenFunc = NULL ;
	MapProtocolTable[EntryNum].UsedEntry = FALSE ;

	EXCLUSION_LOCK_RELEASE();
}


//
//  Name:
//      _RemoveMapDeviceEntry
//
//  Description:
//		Removes the specified entry from the Map Device Table.
//
//  Arguments:
//		EntryNum:	The index of the entry to remove	
//
//  Returns:
//		None
//
static VOID
_RemoveMapDeviceEntry(
	UINT32			EntryNum
	)
{
	EXCLUSION_LOCK_DECL;

	EXCLUSION_LOCK_TAKE();

	MapDeviceTable[EntryNum].DevPath = NULL ;
	MapDeviceTable[EntryNum].DevName = NULL ;
	MapDeviceTable[EntryNum].EntryNumMapProtocol = -1 ;
	MapDeviceTable[EntryNum].UsedEntry = FALSE ;

	EXCLUSION_LOCK_RELEASE();
}


//
//  Name:
//      _AddMapProtocolEntry
//
//  Description:
//		Adds a new entry to the Map Protocol Table.
//
//  Arguments:
//		Guid:		Guid of entry to add
//		OpenFunc:	Pointer to open function associated with Guid
//
//  Returns:
//      EFI_STATUS
//
static EFI_STATUS
_AddMapProtocolEntry(
	EFI_GUID		*Guid,
	EFI_DEV_OPEN	OpenFunc
	)
{
	EXCLUSION_LOCK_DECL;

	INT32		i ;
	EFI_STATUS	RetVal ;

	if( !( Guid && OpenFunc ) )
	{
        return( EFI_INVALID_PARAMETER ) ;
	}

	EXCLUSION_LOCK_TAKE();
	if( _FindGuidMapProtocolEntry( Guid ) != -1 )
	{
		RetVal = EFI_NO_MAPPING ;
		goto Done ;
	}	

	for( i = 0 ; i < MAX_MAPPROTOCOL ; i++ )
	{
		if( MapProtocolTable[i].UsedEntry == FALSE )
		{
			DPRINT(( L"_AddMapProtocolEntry: i=%d Guid=%X OpenFunc=%X\n", i, Guid, OpenFunc )) ;
			MapProtocolTable[i].Guid = Guid ;
			MapProtocolTable[i].OpenFunc = OpenFunc ;
			MapProtocolTable[i].UsedEntry = TRUE ;
			RetVal = EFI_SUCCESS ;
			goto Done ;
		}
	}
	RetVal = EFI_NO_MAPPING ;

Done:
	EXCLUSION_LOCK_RELEASE();
	return( RetVal ) ;
}


//
//  Name:
//      _AddMapDeviceEntry
//
//  Description:
//		Adds a new entry to the Map Device Table.
//
//  Arguments:
//		Guid:		Guid of entry to add
//		DevPath:	Device path
//		DevName:	Name to be mapped to the device path
//
//  Returns:
//      EFI_STATUS
//
//	Note:
//		Guid must be mapped in the protocol table prior to calling
//
static EFI_STATUS
_AddMapDeviceEntry(
	EFI_GUID		*Guid,
	EFI_DEVICE_PATH	*DevPath,
	char			*DevName	
	)
{
	EXCLUSION_LOCK_DECL;

	INT32		i,
				EntryNumMapProtocol ;
	EFI_STATUS	RetVal ;

	if( !( Guid && DevName ) )
	{
        return( EFI_INVALID_PARAMETER ) ;
	}

	EXCLUSION_LOCK_TAKE();
	if( ( EntryNumMapProtocol = _FindGuidMapProtocolEntry( Guid ) ) == -1 )
	{
		RetVal = EFI_NO_MAPPING ;
		goto Done ;
	}	

	if( _FindDevNameMapDeviceEntry( DevName ) != -1 )
	{
		RetVal = EFI_NO_MAPPING ;
		goto Done ;
	}

	for( i = 0 ; i < MAX_MAPDEVICE ; i++ )
	{
		if( MapDeviceTable[i].UsedEntry == FALSE )
		{
			DPRINT(( L"_AddMapDeviceEntry: i=%d DevPath=%X DevName=%a ENMP=%d\n", i, DevPath, DevName, EntryNumMapProtocol )) ;
			MapDeviceTable[i].DevPath = DevPath ;
			MapDeviceTable[i].DevName = DevName ;
			MapDeviceTable[i].EntryNumMapProtocol = EntryNumMapProtocol ;
			MapDeviceTable[i].UsedEntry = TRUE ;
			RetVal = EFI_SUCCESS ;
			goto Done ;
		}
	}
	RetVal = EFI_NO_MAPPING ;

Done:
	EXCLUSION_LOCK_RELEASE();
	return( RetVal ) ;
}


//
//  Name:
//      _FindDevNameMapDeviceEntry
//
//  Description:
//		Locates a DevName in the map device table.
//
//  Arguments:
//		DevName:	Device name to locate 
//
//  Returns:
//		index:		Index of entry containing device name or -1 if not found
//
static INT32
_FindDevNameMapDeviceEntry(
	char			*DevName	
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPDEVICE ; i++ )
	{
		if( MapDeviceTable[i].UsedEntry == TRUE )
		{
			if(	!strcmp( MapDeviceTable[i].DevName, DevName ) )
			{
				return( i ) ;	
			}
		}
	}
	return( -1 ) ;
}


//
//  Name:
//      _FindGuidMapProtocolEntry
//
//  Description:
//		Locates a Guid in the map protocol table.
//
//  Arguments:
//		Guid:		Guid to locate 
//
//  Returns:
//		index:		Index of entry containing guid or -1 if not found
//
static INT32
_FindGuidMapProtocolEntry(
	EFI_GUID		*Guid
	)
{
	INT32	i ;

	for( i = 0 ; i < MAX_MAPPROTOCOL ; i++ )
	{
		if( MapProtocolTable[i].UsedEntry == TRUE )
		{
			if(	!memcmp( MapProtocolTable[i].Guid, Guid, sizeof( EFI_GUID ) ) )
			{
				return( i ) ;	
			}
		}
	}
	return( -1 ) ;
}


//
//  Name:
//      _CallDevNameOpenFunc
//
//  Description:
//		Calls the open function associated with the device name as mapped via
//		the Map Device and Map Protocol tables.
//
//  Arguments:
//		FilePath:	Path of file to open
//		DevName:	Device name to use for mapping
//		Flags:		Flags
//		Mode:		Mode
//		fd:			Pointer to file descriptor returned on success
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
_CallDevNameOpenFunc(
	char			*FilePath,
	char			*DevName,
    int				Flags, 
    mode_t          Mode,
    int             *fd
	)
{
	EXCLUSION_LOCK_DECL;

	INT32			i,
					EntryNumMapProtocol ;
    EFI_STATUS      Status ;
	EFI_DEV_OPEN	OpenFunc ;

	EXCLUSION_LOCK_TAKE();
	for( i = 0 ; i < MAX_MAPDEVICE ; i++ )
	{
		if( ( MapDeviceTable[i].UsedEntry == TRUE ) &&
			!( strcmp( MapDeviceTable[i].DevName, DevName ) ) )
		{
			EntryNumMapProtocol = MapDeviceTable[i].EntryNumMapProtocol ;
			OpenFunc = MapProtocolTable[EntryNumMapProtocol].OpenFunc ;
			DPRINT(( L"_CallDevNameOpenFunc: FilePath %a DevName %a Flags %X Mode %X DevPath %X Guid %X Fd %X\n",
				FilePath, DevName, Flags, Mode, MapDeviceTable[i].DevPath, MapProtocolTable[EntryNumMapProtocol].Guid, fd )) ;

			EXCLUSION_LOCK_RELEASE();

			Status = OpenFunc( FilePath, DevName, Flags, Mode, MapDeviceTable[i].DevPath,
				MapProtocolTable[EntryNumMapProtocol].Guid, fd ) ;
			
			return( Status ) ;
		}
	}
	EXCLUSION_LOCK_RELEASE();
	return( EFI_NO_MAPPING ) ;
}

//
//  === External Interfaces ===
//


//
//  Name:
//		_LIBC_MapProtocol
//
//  Description:
//		Maps a protocol into the Map Protocol Table.
//
//  Arguments:
//		Guid:		Guid of entry to add
//		OpenFunc:	Pointer to open function associated with Guid
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
_LIBC_MapProtocol(
	EFI_GUID		*Guid,
	EFI_DEV_OPEN	OpenFunc
	)
{
	return( _AddMapProtocolEntry( Guid, OpenFunc ) ) ;
}


//
//  Name:
//		_LIBC_MapDevice
//
//  Description:
//		Maps a device into the Map Device Table.
//
//  Arguments:
//		Guid:		Guid of entry to add
//		DevPath:	Device path
//		DevName:	Name to be mapped to the device path
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
_LIBC_MapDevice(
	EFI_GUID		*Guid,
	EFI_DEVICE_PATH	*DevPath,
	char			*DevName
	)
{
	return( _AddMapDeviceEntry( Guid, DevPath, DevName	) ) ;
}
