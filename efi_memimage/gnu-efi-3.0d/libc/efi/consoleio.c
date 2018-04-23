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

    consoleio.c
    
Abstract:

    Interfaces to EFI for I/O to the console


Revision History

--*/

#include <efi_interface.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <sys/poll.h>

#include <libc_debug.h>
#include <assert.h>

#define MIN(x,y) (x < y ? x : y )

#define EFI_CONSOLE_BACKSPACE ((CHAR16)0x0008)
#define EFI_CONSOLE_NEWLINE   ((CHAR16)0x000D)

//
//  Device-specific buffer for console in
//
typedef struct {
    UINT32   nwchars;
    CHAR16  *buf;
} EFILIBC_CONINDEV_T;

//
//  Pointers to Console interfaces
//
static SIMPLE_INPUT_INTERFACE       *ConsoleIn   = NULL;
static SIMPLE_TEXT_OUTPUT_INTERFACE *ConsoleOut  = NULL;
static SIMPLE_TEXT_OUTPUT_INTERFACE *ConsoleErr  = NULL;

int    _ConsoleTabWidth = 8;

static BOOLEAN
IsEndOfLine(
    CHAR16  KeyChar
    )
/*++
Name:
    IsEndOfLine

Description:
    Checks to see if we have reached end of line
    Looks for English Language <CR>, Unicode line and paragraph 
    separators

Arguments:
    KeyChar

Returns:
    TRUE is end of line, otherwise FALSE
--*/
{
    EFI_STATUS Status = EFI_SUCCESS;

    if ( KeyChar == (CHAR16)0x000D ||
         KeyChar == (CHAR16)0x2028 ||
         KeyChar == (CHAR16)0x2029 ) {
        return TRUE;
    }
    return FALSE;
}


static EFI_STATUS
EFI_CloseConsoleIn( 
    VOID            *DevSpecific
    )
/*++
Name:
    EFI_CloseConsoleIn

Description:
    Close the console input

Arguments:
    DevSpecific:  Buffer pointer

Returns:
    EFI_STATUS
--*/
{
    EFILIBC_CONINDEV_T *TermBuf;
    EFI_STATUS         Status = EFI_SUCCESS;

    if ( DevSpecific == NULL ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    TermBuf = DevSpecific;

    if ( TermBuf ) {
        if ( TermBuf->buf ) {
            free( TermBuf->buf );
        }
        free( TermBuf );
    }

Done:
    return Status;
}

static EFI_STATUS
EFI_CloseConsoleOut( 
    VOID            *DevSpecific
    )
/*++
Name:
    EFI_CloseConsoleOut

Description:
    Close the console output

Arguments:
    DevSpecific:  unused for console out

Returns:
    EFI_STATUS
--*/
{
    return EFI_SUCCESS;;
}

static EFI_STATUS
EFI_CloseConsoleErr( 
    VOID            *DevSpecific
    )
/*++
Name:
    EFI_CloseConsoleErr

Description:
    Close the console standard error

Arguments:
    DevSpecific:  unused for console standard error

Returns:
    EFI_STATUS
--*/
{
    return EFI_SUCCESS;
}

static EFI_STATUS
EFI_ReadConsole( 
    OUT    VOID    *Buffer,
    IN OUT UINTN   *BufSize,
    IN     VOID    *DevSpecific
    )
/*++
Name:
    EFI_ReadConsole

Description:
    Read the console using EFI SIMPLE_INPUT_INTERFACE Protocol

    In the traditional implementation of read(), encountering a
    newline is how one terminates the read from the terminal.
    However, if it's not a tty, then the read continues past a newline.
    Here (at least for now) we assume we know it is a tty.

Arguments:
    BufSize:     on input, size of Buffer; on output, number of bytes 
                 returned in Buffer
    Buffer:      pointer to data read
    DevSpecific: pointer to libc-internal console input buffer

Returns:
    EFI_STATUS
--*/
{
    EFI_STATUS        Status      = EFI_SUCCESS;
    EFI_INPUT_KEY     Key;
    EFI_EVENT         Event[1];
    UINTN             Index;
    EFI_BOOT_SERVICES *BootServ;
    UINT32             i;                 //  current buffer position (CHAR16)
    UINT32             j;    
    CHAR16            EchoBuf[3];         //  buffer for echo to screen
    EFILIBC_CONINDEV_T *TermBuf;
    size_t            CopyWchars = 0;     //  number of wchars to copy from
                                          //  internal read buffer
    size_t            ReadWchars = 0;     //  number of wchars to read from
                                          //  the console
    size_t            BufWchars  = 0;     //  total number of Wchars that will
                                          //  fit in the user buffer
    size_t            UsrWchars  = 0;     //  number of wchars actually put
                                          //  into the user buffer

    if ( !BufSize || !Buffer || !DevSpecific ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    assert( *BufSize % 2 == 0 );
    BufWchars = (size_t)*BufSize / sizeof(CHAR16);

    DPRINT(( L"EFI_ReadConsole: BufWchars %d\n", BufWchars ));

    //
    //  Because "cooked mode" console read doesn't return until newline
    //  and user buffer may be smaller than that newline, we must buffer
    //  up the extra characters.  If the internal read buffer overflows 
    //  we throw away the overflow characters.
    //
    //  If the internal read buffer (TermBuf) contains some characters
    //  from a previous read, we just copy from there until either the
    //  user's buffer is full or we run out of data.  Since we are in 
    //  "cooked" mode we know that he buffered data ends in a newline,
    //  and so when we run out of data we just return, even if the user's
    //  buffer isn't full.
    //  
    //  If the internal read buffer is empty, then we just read into the
    //  internal read buffer until either the internal buffer fills up or
    //  we read a newline.
    //
    //  XXX for now we only support "cooked mode"; we don't do "raw mode".
    //

    TermBuf = (EFILIBC_CONINDEV_T*)DevSpecific;
    assert( TermBuf != NULL );

    DPRINT(( L"EFI_ReadConsole: BufSize %d, TermBuf->nwchars %d\n", 
             *BufSize, TermBuf->nwchars ));

    if ( TermBuf->nwchars > 0 ) {

        DPRINT((L"read: TTY: copy from buffer\n"));
        CopyWchars = MIN( BufWchars, TermBuf->nwchars );

        //
        //  Copy from internal buffer into the user's buffer
        //
        wmemcpy( Buffer, TermBuf->buf, CopyWchars );
        wmemset( TermBuf->buf, L'\0', CopyWchars );
        UsrWchars = CopyWchars;

        //
        //  Shift any remaining buffer contents to the front
        //  Brute force, but then we don't have to maintain a
        //  circular buffer.  Can always optimize if need to.
        //
        for ( i=(UINT32)CopyWchars, j=0; i < TermBuf->nwchars; i++,j++ ) {
            TermBuf->buf[j] = TermBuf->buf[i];
            TermBuf->buf[i] = L'\0';
        }
        TermBuf->nwchars -= (UINT32)CopyWchars;

    } else {   /* internal read buffer is empty, so read from console */
    
        assert( TermBuf->nwchars == 0 );
        assert( ConsoleIn != NULL );

        //
        //  Loop, reading a character at a time. At this point we know
        //  that the internal buffer (TermBuf) has been emptied, or we
        //  wouldn't be reading to refill it.  As we read characters just
        //  put them into the internal buffer until we reach a newline or
        //  until the internal buffer is full.  Don't return until we get 
        //  to the end of the line. or overflow the buffer. (cooked mode)
        //

        BootServ = _GetBootServices();
        Event[0] = ConsoleIn->WaitForKey;
        Key.UnicodeChar = (CHAR16)0x0000;

        ReadWchars = 0;  /* we know TermBuf is empty, so start at 0 */

        while ( !IsEndOfLine(Key.UnicodeChar) ) {
    
            EchoBuf[0]  = (CHAR16)0;
            EchoBuf[1]  = (CHAR16)0;
            EchoBuf[2]  = (CHAR16)0;
    
            Status = BootServ->WaitForEvent( 1, Event, &Index );
            if ( EFI_ERROR(Status) ) {
                break;
            }
            
            Status = ConsoleIn->ReadKeyStroke( ConsoleIn, &Key );
            if ( EFI_ERROR(Status) ) {
                break;
            }
    
            DPRINT(( L"EFI_ReadConsole: read char 0x%x (%c)\n", 
                     (Key.UnicodeChar?Key.UnicodeChar:Key.ScanCode),
                     (Key.UnicodeChar?Key.UnicodeChar:Key.ScanCode) ));
    
            //
            //  If the buffer isn't full then put the char into it
            //  (unless backspace, then back up one char in the buffer)
            //
            //  Ignore any null or invalid chars and scan codes.  You may get
            //  keyboard events without any valid codes.  This is following the
            //  EFI spec 0.91 Appendix scan codes.
            //
            //  We also need to handle the case where the buffer has filled, but
            //  the user is backspacing enough to remove the overflow condition.
            //
            if ( ReadWchars <= _POSIX_MAX_INPUT || 
                 (Key.UnicodeChar == EFI_CONSOLE_BACKSPACE && 
                                           (ReadWchars-1) <= BufWchars ) ) {
    
                if ( Key.UnicodeChar != 0 ) {
                    if ( Key.UnicodeChar == EFI_CONSOLE_BACKSPACE ) {
                        if ( ReadWchars > 0 ) {
                           ReadWchars--;
                           TermBuf->nwchars--;
                           TermBuf->buf[ReadWchars] = (CHAR16)0x00;
                           EchoBuf[0]  = EFI_CONSOLE_BACKSPACE;
                        }
    
                    } else if ( Key.UnicodeChar == EFI_CONSOLE_NEWLINE ) {
                        TermBuf->buf[ReadWchars] = L'\n'; /* convrt from '\r' */
                        EchoBuf[0]  = L'\r';
                        EchoBuf[1]  = L'\n';
                        ReadWchars++;
                        TermBuf->nwchars++;
                        DPRINT(( L"EFI_ReadConsole: read newline, TermBuf->nwchars %d\n", TermBuf->nwchars ));
    
                    } else if ( iswprint(Key.UnicodeChar) ) {
                        TermBuf->buf[ReadWchars] = Key.UnicodeChar;
                        EchoBuf[0]  = TermBuf->buf[ReadWchars];
                        ReadWchars++;
                        TermBuf->nwchars++;
                        DPRINT(( L"EFI_ReadConsole: read char, TermBuf->nwchars %d\n", TermBuf->nwchars ));
    
                    } else {
                        DPRINT((L"%ESKIP CHAR %x\n%N", Key.UnicodeChar));
                        continue;
                    }
    
                } else {
    
                    switch ( Key.ScanCode ) {
                    case SCAN_UP:
                    case SCAN_DOWN:
                    case SCAN_RIGHT:
                    case SCAN_LEFT:
                    case SCAN_HOME:
                    case SCAN_END:
                    case SCAN_INSERT:
                    case SCAN_DELETE:
                    case SCAN_PAGE_UP:
                    case SCAN_PAGE_DOWN:
                    case SCAN_F1:
                    case SCAN_F2:
                    case SCAN_F3:
                    case SCAN_F4:
                    case SCAN_F5:
                    case SCAN_F6:
                    case SCAN_F7:
                    case SCAN_F8:
                    case SCAN_F9:
                    case SCAN_F10:
                    //case SCAN_F11:   // Not required in post 0.92 EFI Spec.
                    //case SCAN_F12:   // Not required in post 0.92 EFI Spec.
                    case SCAN_ESC:
                        TermBuf->buf[ReadWchars] = Key.ScanCode;
                        EchoBuf[0] = TermBuf->buf[ReadWchars];
                        ReadWchars++;
                        TermBuf->nwchars++;
                        break;
                    case SCAN_NULL:
                    default:
                        DPRINT((L"%ESKIP SCANCODE %x\n%N", Key.ScanCode));
                        continue;
                    }
                }
            } else {
                DPRINT(( L"%EEFI_ReadConsole: discard: buffer overflow\n%N" ));
            }
    
            //
            //  Echo the character to the console
            //
            ConsoleOut->OutputString( ConsoleOut, EchoBuf );

        } /* while !EOL */

        //
        //  Copy wchars read from console into the user's buffer following 
        //  any wchars that were copied in from the internal buffer
        //  Copy a number of wchars equal to either the number left in
        //  the user buffer (BufWchars-CopyWchars) or the number read,
        //  which ever is less.  Clear the wchars that were copied.
        //
        wmemcpy( &(((CHAR16*)Buffer)[CopyWchars]), 
                 TermBuf->buf, 
                 MIN( ReadWchars, BufWchars - CopyWchars ) );

        wmemset( TermBuf->buf, 
                 L'\0', 
                 MIN( ReadWchars, BufWchars - CopyWchars ) );

        UsrWchars += MIN( ReadWchars, BufWchars - CopyWchars );

        //
        //  Shift any remaining buffer contents to the front
        //  Brute force, but then we don't have to maintain a
        //  circular buffer.  Can always optimize if need to.
        //
        for ( i=(UINT32)MIN(ReadWchars,BufWchars-CopyWchars), j=0; i < TermBuf->nwchars; i++,j++ ){
            TermBuf->buf[j] = TermBuf->buf[i];
            TermBuf->buf[i] = L'\0';
        }
        TermBuf->nwchars = j;    // wchars remaining in buffer

    }  /* else !(TermBuf->nwchars > 0) */

Done:

    *BufSize = UsrWchars * sizeof(CHAR16);
    DHEXDMP( Buffer, *BufSize, L"EFI_ReadConsole buffer" );
    DPRINT((L"EFI_ReadConsole:  returning count %d, BufWchars %d, CopyWchars %d, TermBuf->nwchars %d\n", *BufSize, BufWchars, CopyWchars, TermBuf->nwchars ));
DPRINT((L"EFI_ReadConsole return \n" ));
    return Status;
}

static EFI_STATUS
EFI_WriteConsole( 
    IN     SIMPLE_TEXT_OUTPUT_INTERFACE *ConsoleInterface,
    IN     VOID                         *Buffer,
    IN OUT UINTN                        *BufSize
    )
/*++
Name:
    EFI_WriteConsole

Description:
    General-purpose function to write the console using EFI 
    SIMPLE_TEXT_OUTPUT_INTERFACE Protocol

Arguments:
    BufSize:     on input, size of Buffer in bytes; 
                 on output, number bytes written
    Buffer:      pointer to data to be written

Returns:
    EFI_STATUS
--*/
{
    static UINTN ColumnIndex = 0;

    EFI_STATUS   Status;
    CHAR16      *TermBuf;
    UINTN        TermBufSize;
    UINTN        i, j;
    UINTN        NumNewlines;
    UINTN        Tabs;
    UINTN        BufLen = *BufSize/sizeof(CHAR16);  //BufSize should be even

    if ( !BufSize || !Buffer || ( *BufSize % sizeof(CHAR16) != 0 ) ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    DPRINT((L"EFI_WriteConsole: size %d, BufLen %d\n", 
            *BufSize, BufLen));

    //
    //  Fix up the buffer so that it has a terminating NUL character (needed
    //  by BootServices OutputString()) and so that each linefeed is 
    //  paired with a carriage return (for basic EFI console).
    //

    //
    //  First count the newlines and tabs
    //
    NumNewlines = 0;
    Tabs = 0;
    for ( i=0; i < BufLen; i++ ) {
        if ( ((CHAR16*)Buffer)[i] == L'\n') {
            NumNewlines += 1;
        }
        if ( ((CHAR16*)Buffer)[i] == L'\t') {
        	Tabs += 1;
        }
    }
    DPRINT((L"EFI_WriteConsole: %d newlines %d tabs\n", NumNewlines, Tabs));

    //
    // Allocate the output buffer
    //
    TermBufSize = BufLen + 1 + NumNewlines + (Tabs * _ConsoleTabWidth);
    DPRINT((L"EFI_WriteConsole: TermBufSize %d\n", TermBufSize));
    TermBuf = malloc( (size_t)(TermBufSize * sizeof(CHAR16)) );
    if ( !TermBuf ) {
        DPRINT((L"EFI_WriteConsole: calloc TermBufSize FAILED\n"));
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    //
    //  Copy the input buffer into the output buffer, adding carriage
    //  returns after each linefeed, expanding tabs, and terminating
    //  with a NUL
    //
    for ( i=0, j=0; i < BufLen; i++ ) {
        if ( ((CHAR16*)Buffer)[i] == L'\n' ) {
            TermBuf[j++] = L'\r';
            TermBuf[j++] = L'\n';
        	ColumnIndex = 0;
        } else if ( ((CHAR16*)Buffer)[i] == L'\t' ) {
        	UINTN   Fill = _ConsoleTabWidth - (ColumnIndex % _ConsoleTabWidth);
            while ( Fill-- ) {
            	TermBuf[j++] = L' ';
        		ColumnIndex++;
            }
        } else {
            TermBuf[j++] = ((CHAR16*)Buffer)[i];
        	ColumnIndex++;
        }
    }

    TermBuf[j] = L'\0';

    DPRINT((L"EFI_WriteConsole: call OutputString, ConsoleInterface %X\n",
            ConsoleInterface));
    DPRINT((L"EFI_WriteConsole: call OutputString, OutputString %X\n",
            ConsoleInterface->OutputString ));
    assert( ConsoleInterface != NULL );
    assert( ConsoleInterface->OutputString != NULL );
    Status = ConsoleInterface->OutputString( ConsoleInterface, TermBuf );

    free(TermBuf);

Done:
    return Status;
}

static EFI_STATUS
EFI_WriteConsoleStdin( 
    IN     VOID     *Buffer,
    IN OUT UINTN    *BufSize,
    IN     VOID     *DevSpecific
    )
/*++
Name:
    EFI_WriteConsoleStdin

Description:
    Stub routine to return error if trying to write to stdin

Arguments:
    BufSize:     ignored
    Buffer:      ignored
    DevSpecific: ignored

Returns:
    EFI_STATUS
--*/
{
    return EFI_WRITE_PROTECTED;
}

static EFI_STATUS
EFI_WriteConsoleStdout( 
    IN     VOID     *Buffer,
    IN OUT UINTN    *BufSize,
    IN     VOID     *DevSpecific
    )
/*++
Name:
    EFI_WriteConsoleStdout

Description:
    Write the console using EFI SIMPLE_TEXT_OUTPUT_INTERFACE Protocol

Arguments:
    BufSize:     on input, size of Buffer; on output, number bytes written
    Buffer:      pointer to data to be written
    DevSpecific: unused for console

Returns:
    EFI_STATUS
--*/
{
    return EFI_WriteConsole( ConsoleOut, Buffer, BufSize );
}

static EFI_STATUS
EFI_WriteConsoleStderr( 
    IN     VOID     *Buffer,
    IN OUT UINTN    *BufSize,
    IN     VOID     *DevSpecific
    )
/*++
Name:
    EFI_WriteConsoleStderr

Description:
    Write the console using EFI SIMPLE_TEXT_OUTPUT_INTERFACE Protocol

Arguments:
    BufSize:     on input, size of Buffer; on output, number bytes written
    Buffer:      pointer to data to be written
    DevSpecific: unused for console

Returns:
    EFI_STATUS
--*/
{
    return EFI_WriteConsole( ConsoleErr, Buffer, BufSize );
}

static EFI_STATUS
EFI_FstatConsole( 
    OUT struct stat *sb,
    IN  VOID        *DevSpecific
    )
/*++
Name:
    EFI_FstatConsole

Description:
    Get stat(2) information about the console interface necessary for
    libc support.

Arguments:
    BufSize:      on input, size of Buffer; on output, number bytes written
    Buffer:       pointer to data to be written
    DevSpecific:  unused for console

Returns:
    EFI_STATUS
--*/
{
    EFI_STATUS Status = EFI_SUCCESS;

    sb->st_dev = 0;
    sb->st_ino = 0;
    sb->st_mode = S_IFCHR;
    sb->st_nlink = 1;
    sb->st_uid = 0;
    sb->st_gid = 0;
    sb->st_rdev = 0;
    sb->st_atimespec.tv_sec = 0;
    sb->st_atimespec.tv_nsec = 0;
    sb->st_mtimespec.tv_sec = 0;
    sb->st_mtimespec.tv_nsec = 0;
    sb->st_ctimespec.tv_sec = 0;
    sb->st_ctimespec.tv_nsec = 0;
    sb->st_size = 0;
    sb->st_blocks = 0;
    sb->st_blksize = 65536;
    sb->st_flags = 0;
    sb->st_gen = 0;

    return Status;
}

static EFI_STATUS
EFI_SeekConsole( 
    IN  UINT64           *Position,
    IN  UINT32           whence,
    IN  VOID             *DevSpecific
    )
/*++
Name:
    EFI_SeekConsole

Description:
    Stub for seek function on the console

Arguments:
    Position:     offset to seek to
    whence:       how to interpret Position arg
    DevSpecific:  unused for console

Returns:
    EFI_STATUS
--*/
{
    return EFI_UNSUPPORTED;
}

static EFI_STATUS
EFI_IoctlConsole( 
    IN VOID     *DevSpecific,
    IN UINT32   Request,
    IN va_list  ArgList
    )
/*++
Name:
    EFI_IoctlConsole

Description:
    Stub for ioctl function on the console

Arguments:
    FileHandle:  unused for console
    Request:     device-specific request

Returns:
    EFI_STATUS
--*/
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN Cols, Rows;
    INT32 Mode;
    struct winsize   *ws;

    if ( !ConsoleOut ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    switch ( Request ) {
    case TIOCGWINSZ:  /* get the window size */
        Status = ConsoleOut->QueryMode( ConsoleOut,
                                        ConsoleOut->Mode->Mode,
                                        &Cols,
                                        &Rows
                                      );
    	ws = va_arg(ArgList, struct winsize*);
        ws->ws_row = (unsigned short)Rows;
        ws->ws_col = (unsigned short)Cols;
        ws->ws_xpixel = 0;
        ws->ws_ypixel = 0;
        break;

    case TIOCSWINSZ:  /* set the window size */
    	ws = va_arg(ArgList, struct winsize*);
    	for (Mode = 0; Mode < ConsoleOut->Mode->MaxMode; Mode++) {
    		Status = ConsoleOut->QueryMode(ConsoleOut, Mode, &Cols, &Rows);
    		if (EFI_ERROR(Status)) {
				goto Done;
    		}

    		if ( Rows == ws->ws_row && Cols == ws->ws_col ) {
        		return ( ConsoleOut->SetMode( ConsoleOut, Mode ) );
    		}
    	}

    	/* fall through is requested window size is not supported */
    default:
        Status = EFI_UNSUPPORTED;
    }
    
Done:
    return Status;
}

static EFI_STATUS
EFI_PollConsole( 
    IN  UINT32           Mask,
    IN  VOID             *DevSpecific
    )
/*++
Name:
    EFI_PollConsole

Description:
    Stub for polling the console

Arguments:
    Mask:         Mask of events to poll for
    DevSpecific:  Pointer to our context

Returns:
    EFI_STATUS
--*/
{
    EFI_STATUS         Status;
    UINTN              Index;
    EFI_BOOT_SERVICES  *BootServ;
    EFI_EVENT          Event[2];
    EFILIBC_CONINDEV_T *TermBuf = DevSpecific;

    //
    //  We only support POLLIN and POLLRDNORM
    //
    if ( Mask & ~(POLLIN | POLLRDNORM) )
	    return EFI_UNSUPPORTED;

    BootServ = _GetBootServices();

    //
    //  If there is a pending keystroke, we don't want to read it.
    //  Create a signaled event and use WaitForEvent.  This MUST be
    //  the second event in the array to give the WaitForKey event
    //  priority.
    //
	assert( ConsoleIn != NULL );
	Event[0] = ConsoleIn->WaitForKey;

    Status = BootServ->CreateEvent( 0, 0, NULL, NULL, &Event[1] );
    if ( EFI_ERROR(Status) ){
    	goto cleanup;
    }
    BootServ->SignalEvent( Event[1] );

	//
	//  Now poll the keyboard.  If Index is 0, we have a key pending
	//
	Status = BootServ->WaitForEvent( 2, Event, &Index );
    if ( EFI_ERROR(Status) ){
    	goto cleanup;
    }

	//
	//  Index will equal 1 if nothing there
	//
    if ( Index == 1 )
    	Status = EFI_NOT_READY;
	//
	//  It better equal 0 if not 1
	//
    else
    	assert( Index == 0 );

cleanup:
	BootServ->CloseEvent( Event[1] );
   	return (Status);
}

//
//  Name:
//      EFI_OpenConsoleIn
//
//  Description:
//		Open the console for use with EFI_SIMPLE_INPUT_INTERFACE
//
//  Arguments:
//		DevSpecific:	buffer structure
//
//  Returns:
//		EFI_STATUS
//
static EFI_STATUS
EFI_OpenConsoleIn( 
    VOID ** DevSpecific
    )
{
    EFILIBC_CONINDEV_T  *TermBuf;
    EFI_STATUS          Status;

    ConsoleIn = _GetConsoleIn();
    if (!ConsoleIn ) {
        return EFI_NOT_FOUND;
    }

    Status = EFI_OUT_OF_RESOURCES;	// assume the worst;

    TermBuf = calloc( 1, sizeof(EFILIBC_CONINDEV_T) );
    if ( TermBuf ) {
    	TermBuf->buf = calloc( _POSIX_MAX_INPUT, sizeof(wchar_t) );
    	if ( TermBuf->buf ) {
    		TermBuf->nwchars = 0;
      		Status = EFI_SUCCESS;
       	} else {
       		free( TermBuf );
       		TermBuf = NULL;
    	}
    }

    *DevSpecific = TermBuf;
    return Status;
}

//
//  Name:
//      OpenConsoleIn
//
//  Description:
//		Opens the console input device.
//
//  Arguments:
//		fd:			Pointer to file descriptor returned on success
//		flags:		Flags
//
//  Returns:
//		EFI_STATUS
//
static EFI_STATUS
OpenConsoleIn(
	INT32 *fd,
	int   flags
	)
{
    EFI_STATUS          Status;
    VOID				*DevSpecific;

    DPRINT(( L"OpenConsoleIn: Entry" ));

	Status = EFI_OpenConsoleIn( &DevSpecific );

	if ( ! EFI_ERROR( Status ) ) {
        Status = _LIBC_AllocateNewFileDescriptor( 
                     L"consolein:",           // FileName
                     flags,                	  // Flags
                     0,                       // Mode
                     TRUE,                    // IsATTy
                     DevSpecific,             // DevSpecific, use for buffer
                     EFI_ReadConsole,         // read
                     EFI_WriteConsoleStdout,  // write
                     EFI_CloseConsoleIn,      // close
                     EFI_SeekConsole,         // lseek
                     EFI_FstatConsole,        // fstat
                     EFI_IoctlConsole,        // ioctl
                     EFI_PollConsole,         // poll
                     fd                       // New FileDescriptor
                     );
	}

	if ( EFI_ERROR( Status ) ) {
		(void )EFI_CloseConsoleIn( DevSpecific );
	}

    DPRINT(( L"OpenConsoleIn: Exit\n" ));
    return Status;
}

//
//  Name:
//      OpenConsoleOut
//
//  Description:
//		Opens the console output device.
//
//  Arguments:
//		fd:			Pointer to file descriptor returned on success
//		flags:		Flags
//
//  Returns:
//		EFI_STATUS
//
static EFI_STATUS
OpenConsoleOut(
	INT32 *fd,
	int   flags
	)
{
    EFI_STATUS Status;

    DPRINT(( L"OpenConsoleOut: Entry" ));

    ConsoleOut = _GetConsoleOut();
    if ( !ConsoleOut ) {
        return  EFI_NOT_FOUND;
    }

    Status = _LIBC_AllocateNewFileDescriptor( 
                 L"consoleout:",          // FileName
                 flags,                   // Flags
                 0,                       // Mode
                 TRUE,                    // IsATTy
                 NULL,                    // DevSpecific
                 EFI_ReadConsole,         // read
                 EFI_WriteConsoleStdout,  // write
                 EFI_CloseConsoleOut,     // close
                 EFI_SeekConsole,         // lseek
                 EFI_FstatConsole,        // fstat
                 EFI_IoctlConsole,        // ioctl
                 EFI_PollConsole,         // poll
                 fd                       // New FileDescriptor
                 );

    if ( EFI_ERROR( Status ) ) {
    	EFI_CloseConsoleOut( NULL );
    }

    DPRINT(( L"OpenConsoleOut: Exit\n" ));
    return Status;
}

//
//  Name:
//      OpenConsoleErr
//
//  Description:
//		Opens the console error device.
//
//  Arguments:
//		fd:			Pointer to file descriptor returned on success
//		flags:		Flags
//
//  Returns:
//		EFI_STATUS
//
static EFI_STATUS
OpenConsoleErr(
    int *fd,
    int flags
    )
{
    EFI_STATUS Status;

    DPRINT(( L"OpenConsoleErr: Entry" ));

    ConsoleErr = _GetConsoleErr();
    if ( !ConsoleErr ) {
        Status = EFI_NOT_FOUND;
    }

    Status = _LIBC_AllocateNewFileDescriptor( 
                 L"consoleerr:",          // FileName
                 flags,                   // Flags
                 0,                       // Mode
                 TRUE,                    // IsATTy
                 NULL,                    // DevSpecific
                 EFI_ReadConsole,         // read
                 EFI_WriteConsoleStderr,  // write
                 EFI_CloseConsoleErr,     // close
                 EFI_SeekConsole,         // lseek
                 EFI_FstatConsole,        // fstat
                 EFI_IoctlConsole,        // ioctl
                 EFI_PollConsole,         // poll
                 fd                       // New FileDescriptor
                 );

	if ( EFI_ERROR( Status ) ) {
		EFI_CloseConsoleErr( NULL );
    }

    DPRINT(( L"OpenConsoleErr: Exit\n" ));
    return Status;
}

//
//  Name:
//      _OpenConsole
//
//  Description:
//		Opens a console device.  This function is the main dispatcher for
//		opening any of the console devices.  The function is registered in
//		the Map Protocol Table for both the SIMPLE_TEXT_INPUT_PROTOCOL and
//		the SIMPLE_TEXT_OUTPUT_PROTOCOL.
//
//      We must use a common entry for stderr and stdout because they map
//      to the same EFI protocol.  We also do stdin just to keep things
//      consistent.
//
//  Arguments:
//		FilePath:	Path of file to open
//		DevName:	Device name to use for mapping
//		Flags:		Flags
//		Mode:		Mode
//		DevPath:	Device path
//		Guid:		Guid
//		fd:			Pointer to file descriptor returned on success
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
_OpenConsole(
	char			*FilePath,
	char			*DevName,
	int				Flags,
	mode_t			Mode,
	EFI_DEVICE_PATH	*DevPath,
	EFI_GUID		*Guid,
	INT32			*fd
	)
{
    EFI_STATUS		Status;

    if ( strcmp( "consolein:", DevName ) == 0 ) {
        Status = OpenConsoleIn( fd, Flags );
    } else if ( strcmp( "consoleout:", DevName ) == 0 ) {
        Status = OpenConsoleOut( fd, Flags );
    } else if ( strcmp( "consoleerr:", DevName ) == 0 ) {
        Status = OpenConsoleErr( fd, Flags );
	} else {
        Status = EFI_INVALID_PARAMETER;
	}

	if ( EFI_ERROR( Status ) ) {
		*fd = -1;
	}

	return Status ;
}
