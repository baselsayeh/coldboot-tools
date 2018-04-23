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
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void bcopy(char*, char*, int);

#ifdef _DEBUG
void strcpy(char*, char*);
int  strlen(char*);
#endif

#ifdef notdef /*_EFI_EMULATION_*/
__stdcall
Sleep( long dwMilliseconds );
#endif


void setsoftnet(){}	/* cause soft interrupt to process netisr bits */
                        /* we do that now in splx() */

void panic( char* str)
{
	extern EFI_STATUS StopTimer (void);

	printf("PANIC: %s\n", str);
	(void)StopTimer();
	printf("Hit ^C\n");
	while ( 1 )
#ifdef notdef /*_EFI_EMULATION_*/
		Sleep( 5000 );
#else
		;
#endif
}

int getenv_int(){ return 0; }
#ifdef notdef /*_EFI_EMULATION_*/
int snprintf(char* buf){ strcpy( buf, "undi0" ); return strlen("undi0"); }
#endif

char *
inet_ntoa(int ina)
{
	static char buf[4*sizeof "123"];
	unsigned char *ucp = (unsigned char *)&ina;

	sprintf(buf, "%d.%d.%d.%d",
		ucp[0] & 0xff,
		ucp[1] & 0xff,
		ucp[2] & 0xff,
		ucp[3] & 0xff);
	return buf;
}

int copyin(void *src, void *dst, int size){ bcopy( src, dst, size ); return 0;}
int copyout(void *src, void *dst, int size){ bcopy( src, dst, size ); return 0;}

#ifdef _TIMESTAMP_LOG_ENTRY_
char logbuf[1024];
#endif
void log_(int level, char *fmt, ... )
{
	va_list	pArg;
	va_start( pArg, fmt );
#ifdef _TIMESTAMP_LOG_ENTRY_
	vsprintf( logbuf, fmt, pArg );
	fprintf(stderr, "%03d: %s", time(NULL) % 1000, logbuf);
#else
	vfprintf( stderr, fmt, pArg );
#endif
	va_end( pArg );
}

int   proc0;
