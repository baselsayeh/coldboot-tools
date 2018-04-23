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

/*++

Module Name:

    malloc.c
    
Abstract:

    Functions implementing the standard C library malloc interfaces.


Revision History

--*/

#include <efi_interface.h>
#include <efilist.h>
#include <stdlib.h>
#include <string.h>

//#define MALLOC_MEMORY_TRACE 1

//
//  List of pointers and corresponding allocation sizes
//  Uses the EFI library's linked list macros & types
//
#define _STACK_SIZE 4
#define _MALLOC_SIZE_SIGNATURE 'mlss'
typedef struct {
    UINTN      Signature;
    LIST_ENTRY Link;
    void *     Address;
    size_t     Size;
#ifdef MALLOC_MEMORY_TRACE
    char *    CallStack[ _STACK_SIZE ];
#endif
} _MALLOC_SIZE_ENTRY;
static LIST_ENTRY MallocSizeList;

static BOOLEAN MallocInitialized=FALSE;

int		_MallocListLength = 0;
size_t	_MallocListAggregateSize = 0;
int    	_FreeMissMatches = 0;


#ifdef MALLOC_MEMORY_TRACE

#define	RECORD_TRACE( e, s ) \
do { \
	char **ebp = (char**)(&s); \
	int i; \
	ebp -= 2; \
	for (i = 0; i < _STACK_SIZE; i++) { \
		e->CallStack[i] = *(ebp+1); \
		ebp = (char**)*(ebp); \
	} \
} while (0)

int
_MallocTraceBack( char **Buf, size_t Size, int *Depth )
{
	EXCLUSION_LOCK_DECL;

    _MALLOC_SIZE_ENTRY *ListEntry;
    LIST_ENTRY         *Current;
    char               *Base = (char*)_GetLoadedImage()->ImageBase;
    int                Count = 0;

    *Depth = _STACK_SIZE + 2;
    Size = (Size/sizeof(char*))/(_STACK_SIZE + 2);

	EXCLUSION_LOCK_TAKE();
    if ( !IsListEmpty( &MallocSizeList ) ) {
        for ( Current =  MallocSizeList.Flink; 
              Size && Current != &MallocSizeList; 
              Current =  Current->Flink, Size-- ) {

			int i;

            ListEntry = CR( Current, 
                            _MALLOC_SIZE_ENTRY, 
                            Link,
                            _MALLOC_SIZE_SIGNATURE);

            *Buf++ = ListEntry->Address;
            *Buf++ = (char*)ListEntry->Size;
            for ( i = 0; i < _STACK_SIZE; i++ ) {
            	*Buf++ = (char*)(ListEntry->CallStack[i] - Base);
            }

            Count++;
        }
    } 
	EXCLUSION_LOCK_RELEASE();
    return Count;
}

int
_MallocTraceBackDiff( char **InBuf, int InCount, char **OutBuf, int OutCount )
{
	EXCLUSION_LOCK_DECL;

    _MALLOC_SIZE_ENTRY *ListEntry;
    LIST_ENTRY         *Current;
	char               **p;
    char               *Base = (char*)_GetLoadedImage()->ImageBase;
    int                New = 0;

	EXCLUSION_LOCK_TAKE();
    if ( !IsListEmpty( &MallocSizeList ) ) {
        for ( Current =  MallocSizeList.Flink; 
              OutCount && Current != &MallocSizeList; 
              Current =  Current->Flink ) {

			int i;

            ListEntry = CR( Current, 
                            _MALLOC_SIZE_ENTRY, 
                            Link,
                            _MALLOC_SIZE_SIGNATURE);

            for ( p = InBuf, i = 0; i < InCount; i++, p += (_STACK_SIZE + 2) ) {
            	if ( *p == (char*)ListEntry->Address &&
            	     *(p+1) == (char*)ListEntry->Size ) {
            		goto next;
            	}
            }

            *OutBuf++ = ListEntry->Address;
            *OutBuf++ = (char*)ListEntry->Size;
            for ( i = 0; i < _STACK_SIZE; i++ ) {
            	*OutBuf++ = (char*)(ListEntry->CallStack[i] - Base);
            }
            OutCount--;
            New++;
next:
        ;
        }
    } 
	EXCLUSION_LOCK_RELEASE();
    return New;
}

#else
#define RECORD_TRACE( e, s )
#endif

//
//  === Internal Functions ===
//
    
//
//  Name:
//      InitializeMalloc
//
//  Description:
//      Check if any malloc functions have been called
//      If not, do initialization
//
//  Arguments:
//      --none--
//
//  Returns:
//      --none--
//
void
_InitializeMalloc(
    void
    )
{
	EXCLUSION_LOCK_DECL;

	EXCLUSION_LOCK_TAKE();
    if ( !MallocInitialized ) {
        InitializeListHead( &MallocSizeList );
        MallocInitialized = TRUE;
    }
	EXCLUSION_LOCK_RELEASE();
}

//
//	Name:
//		DeInitializeMalloc
//
//	Description:
//		Check if malloc has been initialized and if so,
//		clean up by walking linked list and freeing all
//		entries.
//
//	Arguments:
//		--none--
//
//	Returns:
//		--none--
//
void
_DeInitializeMalloc(
	void
	)
{
	EXCLUSION_LOCK_DECL;

    _MALLOC_SIZE_ENTRY *ListEntry;
    LIST_ENTRY         *Current;

	if( !MallocInitialized )
	{
		return ;
	}

	EXCLUSION_LOCK_TAKE();
	while( !IsListEmpty( &MallocSizeList ) )
	{
		Current = MallocSizeList.Flink ;

        ListEntry = CR( Current, 
                        _MALLOC_SIZE_ENTRY, 
                        Link,
                        _MALLOC_SIZE_SIGNATURE);
		if( ListEntry->Address )
		{
        	EFI_FreePool( ListEntry->Address );
		}
		RemoveEntryList( &ListEntry->Link ) ;
		_MallocListLength--;
		_MallocListAggregateSize -= ListEntry->Size;
		EFI_FreePool( ListEntry ) ;
	}
	MallocInitialized = FALSE ;
	EXCLUSION_LOCK_RELEASE();
}

//
//  Name:
//      SaveSize
//
//  Description:
//      Save the address and size of the newly allocated buffer
//      in MallocSizeList, so we can find it later for realloc()
//
//  Arguments:
//      void * NewPtr:  pointer to newly allocated buffer
//
//  Returns:
//      --none--
//
static VOID
SaveSize(
    VOID * NewPtr,
    size_t Size
    )
{
	EXCLUSION_LOCK_DECL;

    if ( NewPtr ) {
        _MALLOC_SIZE_ENTRY *NewEntry = NULL;
        NewEntry = EFI_AllocatePool( sizeof( _MALLOC_SIZE_ENTRY ) );
        if ( NewEntry ) {
            NewEntry->Signature = _MALLOC_SIZE_SIGNATURE;
            NewEntry->Address   = NewPtr;
            NewEntry->Size      = Size;

			EXCLUSION_LOCK_TAKE();
            InsertHeadList( &MallocSizeList, &NewEntry->Link );
			EXCLUSION_LOCK_RELEASE();

			_MallocListLength++;
			_MallocListAggregateSize += NewEntry->Size;
			RECORD_TRACE( NewEntry, NewPtr );
        } else {
            free( NewPtr );
        }
    }
}

//
//  Name:
//      FindMemObj
//
//  Description:
//      Find the entry in the MallocSizeList corresponding to the given
//      pointer.  Only pointers previously returned by a malloc function
//      will be found.
//
//  Arguments:
//      void *memobj: address of memory object whose entry is to be retrieved
//
//  Returns:
//      _MALLOC_SIZE_ENTRY* : pointer to the list entry for the given address,
//                            or NULL if not found.
//
static _MALLOC_SIZE_ENTRY*
FindMemObj( 
    VOID *MemObj
    )
{
	EXCLUSION_LOCK_DECL;

    _MALLOC_SIZE_ENTRY *ListEntry;
    LIST_ENTRY         *Current;

	EXCLUSION_LOCK_TAKE();
    if ( !IsListEmpty( &MallocSizeList ) ) {

        for ( Current =  MallocSizeList.Flink; 
              Current != &MallocSizeList; 
              Current =  Current->Flink ) {

            ListEntry = CR( Current, 
                            _MALLOC_SIZE_ENTRY, 
                            Link,
                            _MALLOC_SIZE_SIGNATURE);

            if ( ListEntry->Address == MemObj ) {
				EXCLUSION_LOCK_RELEASE();
                return ListEntry;
            }
        }
    } 
	EXCLUSION_LOCK_RELEASE();
    return NULL;
}


//
//  === Public Interface Functions ===
//


void *
calloc(
    size_t NMemb,
    size_t MembSize )
{
    void *NewMem = NULL;
    size_t NewSize = NMemb * MembSize;

    NewMem = EFI_AllocatePool( NewSize );
    if ( NewMem ) {
        memset( NewMem, 0, NewSize );
    }
    SaveSize( NewMem, (NewSize) );
    return NewMem;
}

void
free( 
    void *Ptr
    )
{
    _MALLOC_SIZE_ENTRY *ListEntry = NULL;

    //
    // if NULL pointer do nothing
    //
    if ( !Ptr ) {
        return;
    }

    /* find and remove this memory object's entry in the list */
    ListEntry = FindMemObj( Ptr );
    if ( ListEntry ) {
        RemoveEntryList( &ListEntry->Link );
		_MallocListLength--;
		_MallocListAggregateSize -= ListEntry->Size;
        EFI_FreePool( ListEntry );
        ListEntry = NULL;
    	EFI_FreePool( Ptr );
    } else {
    	_FreeMissMatches++;
    }
}

void *
malloc( 
    size_t Size
    )
{
    void *NewMem;

    NewMem = EFI_AllocatePool( (UINTN)Size );
    SaveSize( NewMem, Size );
    return NewMem;
}

void *
realloc( 
    void   *OldPtr, 
    size_t NewSize
    )
{
    _MALLOC_SIZE_ENTRY * ListEntry;
    size_t               OldSize;
    void *               NewPtr;

    if ( !OldPtr ) {
        /*
         *  NULL pointer means just do malloc
         */
        return malloc( NewSize );

    } else if ( NewSize == 0 ) {
        /*
         *  Non-NULL pointer and zero size means just do free
         */
        free( OldPtr );
    } else {

        /* Look up the old size in the list */
        ListEntry = FindMemObj( OldPtr );
        if ( ListEntry ) {
            OldSize = ListEntry->Size;
            NewPtr = EFI_AllocatePool( NewSize );
            if ( NewPtr ) {
                memcpy( NewPtr, OldPtr, (OldSize<NewSize?OldSize:NewSize) );
                free( OldPtr );  /* deletes list entry */
    
                /* Save a new entry in MallocSizeList */
                SaveSize( NewPtr, NewSize );

            	return NewPtr;
            }
        }
    }

    return NULL;
}


void *
reallocf(
    void   *OldPtr,
    size_t NewSize
    )
{
    void *NewPtr;

    if ( (NewPtr = realloc( OldPtr, NewSize )) == NULL ) {
        if ( OldPtr ) {
            free( OldPtr );
        }
    }
    return NewPtr;
}
