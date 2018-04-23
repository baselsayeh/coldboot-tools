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

    Bin2Hex.c
    
Abstract:
    Give binary file a hexadecimal readable format
    in a new '.txt' file

Author:    

Revision History

--*/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>

#define  NUM_OF_COLUMNS 8

main(int argc, char* argv[])
{
    FILE*   fpSrc;
    FILE*   fpDest;
    char    InBuf;
    char    OutBuf[5];//'0xXX'+'\0'
    char*   pDestName;

    int i;
    long li;

    //
    //Display Help Screen
    //
    if ( argc != 2 
        || ( argc == 2 && !strcmp( argv[1], "?" ))
        || ( argc == 2 && !strcmp( argv[1], "-?" ))
        || ( argc == 2 && !strcmp( argv[1], "/?" ))
        || ( argc == 2 && !strcmp( argv[1], "h" ))
        || ( argc == 2 && !strcmp( argv[1], "-h" ))
        || ( argc == 2 && !strcmp( argv[1], "/h" ))
        )
    {
        printf( "Bin2Hex: Give binary file a "
            "hexadecimal readable format\n" );
        printf( "Input:   A binary file\n" );
        printf( "Output:  Create a new file with the added extension"
            " '.txt'\n" );
        printf( "Usage:\n"
                "         Bin2Hex filename\n" );
        printf( "         ( where 'filename' is the name of the input file )\n" );
        return 0;
    }

    //
    //open the binary file
    //
    fpSrc = fopen( argv[1], "rb" );
    if ( !fpSrc )
    {
        printf( "File: %s open error\n", argv[1] );
        return 0;
    }

    //
    //Create Destination file name
    //
    pDestName = malloc( strlen( argv[1] ) + 5 );
    strcpy( pDestName, argv[1] );
    strcat( pDestName, ".txt");

    //
    //create the destination file, if conflict, prompt and return
    //

    //Is it there already?
    fpDest = fopen( pDestName, "r" );
    if ( fpDest ) // Oh, conflict
    {
        printf( "File: %s already exists! Task not performed\n",
            pDestName );
        fclose( fpDest );
        free( pDestName );
        return 0;
    }

    fpDest = fopen( pDestName, "wt" );

    //
    //Perform reading and writing
    //
    i = 0;
    li = 0;
    while ( fread( &InBuf, 1, 1, fpSrc ) )
    { 
        li ++;
        if ( ((int)(unsigned char)InBuf) < 16 )//for align purpose
        {
            strcpy( OutBuf, "0x0" );
            _itoa( (int)(unsigned char)InBuf, OutBuf + 3, 16 );            
        }
        else
        {
            strcpy( OutBuf, "0x" );
            _itoa( (int)(unsigned char)InBuf, OutBuf + 2, 16 );            
        }

        if (li > 1) //not the first byte, add a comma
        {
            fprintf( fpDest, ", ");
        }

        if ( i == NUM_OF_COLUMNS )
        {
            fprintf( fpDest, "\n" );
            i = 0;
        }

        fprintf( fpDest, "%s", OutBuf );
        i++;
    }

    fprintf( fpDest, "\n//Total: %ld bytes", li );

    if ( ferror(fpSrc) )
    {
        printf( "File: %s operation error!\n" );
    }
    
    //
    //Close both files
    //
    fclose( fpSrc );
    fclose( fpDest );

    free( pDestName );
    return 0;

}
