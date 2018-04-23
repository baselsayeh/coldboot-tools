#
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

#
#  Set the base output name
#

BASE_NAME = libc

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\lib\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\lib\$(BASE_NAME)

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ /D XPG4 $(C_FLAGS)

#
#  If blkxx: devices are to be mapped along with fsxx:, uncomment this line
#
C_FLAGS = /D MAP_BLOCKIO_DEVICES $(C_FLAGS)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\efishell\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\efishell \
      -I $(SDK_INSTALL_DIR)\include\efishell\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include include\makefile.hdr
INC = -I include \
      -I include\$(PROCESSOR) $(INC)

!include makefile.hdr
INC = -I . \
      -I locale $(INC)

#
# Default target
#

all : sub_dirs $(OBJECTS)

#
# Local sub directories
#

sub_dirs : $(BUILD_DIR)\efi \
           $(BUILD_DIR)\locale \
           $(BUILD_DIR)\i386\gen \
           $(BUILD_DIR)\i386\math \
           $(BUILD_DIR)\string \
	       $(BUILD_DIR)\stdlib \
	       $(BUILD_DIR)\stdio \
	       $(BUILD_DIR)\regex \
	       $(BUILD_DIR)\wchar \
           $(BUILD_DIR)\sys \
           $(BUILD_DIR)\gen \
           $(BUILD_DIR)\ia64\gen \
           $(BUILD_DIR)\ia64\math \
           $(BUILD_DIR)\stdtime \
           $(BUILD_DIR)\nls \
           $(BUILD_DIR)\em64t\gen \
           $(BUILD_DIR)\em64t\math \


#
# Sub-directory targets
#

$(BUILD_DIR)\efi      : ; - md $(BUILD_DIR)\efi
$(BUILD_DIR)\locale   : ; - md $(BUILD_DIR)\locale
$(BUILD_DIR)\i386\gen : ; - md $(BUILD_DIR)\i386\gen
$(BUILD_DIR)\i386\math : ; - md $(BUILD_DIR)\i386\math
$(BUILD_DIR)\string   : ; - md $(BUILD_DIR)\string
$(BUILD_DIR)\stdlib   : ; - md $(BUILD_DIR)\stdlib
$(BUILD_DIR)\stdio    : ; - md $(BUILD_DIR)\stdio
$(BUILD_DIR)\regex    : ; - md $(BUILD_DIR)\regex
$(BUILD_DIR)\wchar    : ; - md $(BUILD_DIR)\wchar
$(BUILD_DIR)\sys      : ; - md $(BUILD_DIR)\sys
$(BUILD_DIR)\gen      : ; - md $(BUILD_DIR)\gen
$(BUILD_DIR)\ia64\gen : ; - md $(BUILD_DIR)\ia64\gen
$(BUILD_DIR)\ia64\math : ; - md $(BUILD_DIR)\ia64\math
$(BUILD_DIR)\stdtime  : ; - md $(BUILD_DIR)\stdtime
$(BUILD_DIR)\nls      : ; - md $(BUILD_DIR)\nls
$(BUILD_DIR)\em64t\gen : ; - md $(BUILD_DIR)\em64t\gen
$(BUILD_DIR)\em64t\math : ; - md $(BUILD_DIR)\em64t\math


#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\efi\init.obj \
    $(BUILD_DIR)\efi\efi_interface.obj \
    $(BUILD_DIR)\efi\memory.obj \
    $(BUILD_DIR)\efi\consoleio.obj \
    $(BUILD_DIR)\efi\fileio.obj \
    $(BUILD_DIR)\efi\blockio.obj \
    $(BUILD_DIR)\efi\getpagesize.obj \
    $(BUILD_DIR)\efi\env.obj \
    $(BUILD_DIR)\efi\efi_time.obj \
    $(BUILD_DIR)\efi\is_valid_addr.obj \
    $(BUILD_DIR)\efi\LoadImage.obj \
    $(BUILD_DIR)\efi\StartImage.obj \
    $(BUILD_DIR)\efi\UnloadImage.obj \
    $(BUILD_DIR)\efi\GetFileDevicePath.obj \
\
    $(BUILD_DIR)\locale\ansi.obj \
    $(BUILD_DIR)\locale\collate.obj \
    $(BUILD_DIR)\locale\collcmp.obj \
    $(BUILD_DIR)\locale\isctype.obj \
    $(BUILD_DIR)\locale\none.obj \
    $(BUILD_DIR)\locale\runetype.obj \
    $(BUILD_DIR)\locale\table.obj \
    $(BUILD_DIR)\locale\tolower.obj \
    $(BUILD_DIR)\locale\toupper.obj \
    $(BUILD_DIR)\locale\rune.obj \
    $(BUILD_DIR)\locale\setlocale.obj \
    $(BUILD_DIR)\locale\lconv.obj \
    $(BUILD_DIR)\locale\localeconv.obj \
\
    $(BUILD_DIR)\i386\gen\isinf.obj \
\
    $(BUILD_DIR)\string\bcmp.obj \
    $(BUILD_DIR)\string\bcopy.obj \
    $(BUILD_DIR)\string\bzero.obj \
    $(BUILD_DIR)\string\ffs.obj \
    $(BUILD_DIR)\string\index.obj \
    $(BUILD_DIR)\string\memccpy.obj \
    $(BUILD_DIR)\string\memchr.obj \
    $(BUILD_DIR)\string\memcmp.obj \
    $(BUILD_DIR)\string\memcpy.obj \
    $(BUILD_DIR)\string\memmove.obj \
    $(BUILD_DIR)\string\memset.obj \
    $(BUILD_DIR)\string\rindex.obj \
    $(BUILD_DIR)\string\strcasecmp.obj \
    $(BUILD_DIR)\string\strcat.obj \
    $(BUILD_DIR)\string\strchr.obj \
    $(BUILD_DIR)\string\strcmp.obj \
    $(BUILD_DIR)\string\strcoll.obj \
    $(BUILD_DIR)\string\strcpy.obj \
    $(BUILD_DIR)\string\strcspn.obj \
    $(BUILD_DIR)\string\strdup.obj \
    $(BUILD_DIR)\string\strerror.obj \
    $(BUILD_DIR)\string\strlen.obj \
    $(BUILD_DIR)\string\strmode.obj \
    $(BUILD_DIR)\string\strncat.obj \
    $(BUILD_DIR)\string\strncmp.obj \
    $(BUILD_DIR)\string\strncpy.obj \
    $(BUILD_DIR)\string\strpbrk.obj \
    $(BUILD_DIR)\string\strrchr.obj \
    $(BUILD_DIR)\string\strsep.obj \
    $(BUILD_DIR)\string\strspn.obj \
    $(BUILD_DIR)\string\strstr.obj \
    $(BUILD_DIR)\string\strtok.obj \
    $(BUILD_DIR)\string\strxfrm.obj \
    $(BUILD_DIR)\string\swab.obj \
\
    $(BUILD_DIR)\stdlib\abort.obj \
    $(BUILD_DIR)\stdlib\abs.obj \
    $(BUILD_DIR)\stdlib\atexit.obj \
    $(BUILD_DIR)\stdlib\atoi.obj \
    $(BUILD_DIR)\stdlib\atol.obj \
    $(BUILD_DIR)\stdlib\exit.obj \
    $(BUILD_DIR)\stdlib\getopt.obj \
    $(BUILD_DIR)\stdlib\labs.obj \
    $(BUILD_DIR)\stdlib\malloc.obj \
    $(BUILD_DIR)\stdlib\rand.obj \
    $(BUILD_DIR)\stdlib\random.obj \
    $(BUILD_DIR)\stdlib\strtod.obj \
    $(BUILD_DIR)\stdlib\strtol.obj \
    $(BUILD_DIR)\stdlib\strtoul.obj \
    $(BUILD_DIR)\stdlib\strtoq.obj \
    $(BUILD_DIR)\stdlib\strtouq.obj \
    $(BUILD_DIR)\stdlib\qsort.obj \
    $(BUILD_DIR)\stdlib\getenv.obj \
    $(BUILD_DIR)\stdlib\putenv.obj \
    $(BUILD_DIR)\stdlib\setenv.obj \
    $(BUILD_DIR)\stdlib\system.obj \
\
    $(BUILD_DIR)\stdio\_flock_stub.obj \
    $(BUILD_DIR)\stdio\asprintf.obj \
    $(BUILD_DIR)\stdio\clrerr.obj \
    $(BUILD_DIR)\stdio\fclose.obj \
    $(BUILD_DIR)\stdio\fdopen.obj \
    $(BUILD_DIR)\stdio\feof.obj \
    $(BUILD_DIR)\stdio\ferror.obj \
    $(BUILD_DIR)\stdio\fflush.obj \
    $(BUILD_DIR)\stdio\fgetc.obj \
    $(BUILD_DIR)\stdio\fgetln.obj \
    $(BUILD_DIR)\stdio\fgetpos.obj \
    $(BUILD_DIR)\stdio\fgets.obj \
    $(BUILD_DIR)\stdio\fileno.obj \
    $(BUILD_DIR)\stdio\findfp.obj \
    $(BUILD_DIR)\stdio\flags.obj \
    $(BUILD_DIR)\stdio\fopen.obj \
    $(BUILD_DIR)\stdio\fprintf.obj \
    $(BUILD_DIR)\stdio\fpurge.obj \
    $(BUILD_DIR)\stdio\fputc.obj \
    $(BUILD_DIR)\stdio\fputs.obj \
    $(BUILD_DIR)\stdio\fread.obj \
    $(BUILD_DIR)\stdio\freopen.obj \
    $(BUILD_DIR)\stdio\fscanf.obj \
    $(BUILD_DIR)\stdio\fseek.obj \
    $(BUILD_DIR)\stdio\fsetpos.obj \
    $(BUILD_DIR)\stdio\ftell.obj \
    $(BUILD_DIR)\stdio\funopen.obj \
    $(BUILD_DIR)\stdio\fvwrite.obj \
    $(BUILD_DIR)\stdio\fwalk.obj \
    $(BUILD_DIR)\stdio\fwrite.obj \
    $(BUILD_DIR)\stdio\gets.obj \
    $(BUILD_DIR)\stdio\getw.obj \
    $(BUILD_DIR)\stdio\makebuf.obj \
    $(BUILD_DIR)\stdio\mktemp.obj \
    $(BUILD_DIR)\stdio\perror.obj \
    $(BUILD_DIR)\stdio\printf.obj \
    $(BUILD_DIR)\stdio\puts.obj \
    $(BUILD_DIR)\stdio\putw.obj \
    $(BUILD_DIR)\stdio\refill.obj \
    $(BUILD_DIR)\stdio\remove.obj \
    $(BUILD_DIR)\stdio\rewind.obj \
    $(BUILD_DIR)\stdio\rget.obj \
    $(BUILD_DIR)\stdio\scanf.obj \
    $(BUILD_DIR)\stdio\setbuf.obj \
    $(BUILD_DIR)\stdio\setbuffer.obj \
    $(BUILD_DIR)\stdio\setvbuf.obj \
    $(BUILD_DIR)\stdio\snprintf.obj \
    $(BUILD_DIR)\stdio\sprintf.obj \
    $(BUILD_DIR)\stdio\sscanf.obj \
    $(BUILD_DIR)\stdio\stdio.obj \
    $(BUILD_DIR)\stdio\tempnam.obj \
    $(BUILD_DIR)\stdio\tmpfile.obj \
    $(BUILD_DIR)\stdio\tmpnam.obj \
    $(BUILD_DIR)\stdio\ungetc.obj \
    $(BUILD_DIR)\stdio\vasprintf.obj \
    $(BUILD_DIR)\stdio\vfprintf.obj \
    $(BUILD_DIR)\stdio\vfscanf.obj \
    $(BUILD_DIR)\stdio\vprintf.obj \
    $(BUILD_DIR)\stdio\vscanf.obj \
    $(BUILD_DIR)\stdio\vsnprintf.obj \
    $(BUILD_DIR)\stdio\vsprintf.obj \
    $(BUILD_DIR)\stdio\vsscanf.obj \
    $(BUILD_DIR)\stdio\wbuf.obj \
    $(BUILD_DIR)\stdio\wsetup.obj \
\
    $(BUILD_DIR)\regex\regcomp.obj \
    $(BUILD_DIR)\regex\regerror.obj \
    $(BUILD_DIR)\regex\regexec.obj \
    $(BUILD_DIR)\regex\regfree.obj \
\
    $(BUILD_DIR)\wchar\wcscat.obj \
    $(BUILD_DIR)\wchar\wcschr.obj \
    $(BUILD_DIR)\wchar\wcscmp.obj \
    $(BUILD_DIR)\wchar\wcscpy.obj \
    $(BUILD_DIR)\wchar\wcscspn.obj \
    $(BUILD_DIR)\wchar\wcslen.obj \
    $(BUILD_DIR)\wchar\wcsncat.obj \
    $(BUILD_DIR)\wchar\wcsncmp.obj \
    $(BUILD_DIR)\wchar\wcsncpy.obj \
    $(BUILD_DIR)\wchar\wcspbrk.obj \
    $(BUILD_DIR)\wchar\wcsrchr.obj \
    $(BUILD_DIR)\wchar\wcsspn.obj \
    $(BUILD_DIR)\wchar\wcssep.obj \
    $(BUILD_DIR)\wchar\wcsstr.obj \
    $(BUILD_DIR)\wchar\wcstok.obj \
    $(BUILD_DIR)\wchar\wgetopt.obj \
    $(BUILD_DIR)\wchar\wmemchr.obj \
    $(BUILD_DIR)\wchar\wmemcmp.obj \
    $(BUILD_DIR)\wchar\wmemcpy.obj \
    $(BUILD_DIR)\wchar\wmemmove.obj \
    $(BUILD_DIR)\wchar\wmemset.obj \
    $(BUILD_DIR)\wchar\iswctype.obj \
    $(BUILD_DIR)\wchar\wctrans.obj \
    $(BUILD_DIR)\wchar\wctype.obj \
    $(BUILD_DIR)\wchar\wwbuf.obj \
    $(BUILD_DIR)\wchar\fputwc.obj \
    $(BUILD_DIR)\wchar\wrefill.obj \
    $(BUILD_DIR)\wchar\rgetw.obj \
    $(BUILD_DIR)\wchar\fgetwc.obj \
    $(BUILD_DIR)\wchar\ungetwc.obj \
    $(BUILD_DIR)\wchar\fputws.obj \
    $(BUILD_DIR)\wchar\fgetws.obj \
    $(BUILD_DIR)\wchar\putws.obj \
    $(BUILD_DIR)\wchar\getws.obj \
    $(BUILD_DIR)\wchar\wopen.obj \
    $(BUILD_DIR)\wchar\wfopen.obj \
    $(BUILD_DIR)\wchar\wstat.obj \
    $(BUILD_DIR)\wchar\wfaststat.obj \
    $(BUILD_DIR)\wchar\wmkdir.obj \
    $(BUILD_DIR)\wchar\wrmdir.obj \
    $(BUILD_DIR)\wchar\vfwprintf.obj \
    $(BUILD_DIR)\wchar\vswprintf.obj \
    $(BUILD_DIR)\wchar\vwprintf.obj \
    $(BUILD_DIR)\wchar\fwprintf.obj \
    $(BUILD_DIR)\wchar\swprintf.obj \
    $(BUILD_DIR)\wchar\wprintf.obj \
    $(BUILD_DIR)\wchar\vfwscanf.obj \
    $(BUILD_DIR)\wchar\vswscanf.obj \
    $(BUILD_DIR)\wchar\vwscanf.obj \
    $(BUILD_DIR)\wchar\fwscanf.obj \
    $(BUILD_DIR)\wchar\swscanf.obj \
    $(BUILD_DIR)\wchar\wscanf.obj \
    $(BUILD_DIR)\wchar\wcstod.obj \
    $(BUILD_DIR)\wchar\wcstol.obj \
    $(BUILD_DIR)\wchar\wcstoul.obj \
    $(BUILD_DIR)\wchar\wcstoq.obj \
    $(BUILD_DIR)\wchar\wcstouq.obj \
    $(BUILD_DIR)\wchar\wcscoll.obj \
    $(BUILD_DIR)\wchar\wcsftime.obj \
    $(BUILD_DIR)\wchar\wcsxfrm.obj \
    $(BUILD_DIR)\wchar\wasctime.obj \
    $(BUILD_DIR)\wchar\wctime.obj \
\
    $(BUILD_DIR)\sys\init.obj \
    $(BUILD_DIR)\sys\__error.obj \
    $(BUILD_DIR)\sys\_exit.obj \
    $(BUILD_DIR)\sys\close.obj \
    $(BUILD_DIR)\sys\rename.obj \
    $(BUILD_DIR)\sys\fcntl.obj \
    $(BUILD_DIR)\sys\filedesc.obj \
    $(BUILD_DIR)\sys\getpid.obj \
    $(BUILD_DIR)\sys\gettimeofday.obj \
    $(BUILD_DIR)\sys\ioctl.obj \
    $(BUILD_DIR)\sys\isatty.obj \
    $(BUILD_DIR)\sys\open.obj \
    $(BUILD_DIR)\sys\lseek.obj \
    $(BUILD_DIR)\sys\read.obj \
    $(BUILD_DIR)\sys\stat.obj \
    $(BUILD_DIR)\sys\faststat.obj \
    $(BUILD_DIR)\sys\unlink.obj \
    $(BUILD_DIR)\sys\write.obj \
    $(BUILD_DIR)\sys\writev.obj \
    $(BUILD_DIR)\sys\map.obj \
    $(BUILD_DIR)\sys\getdirentries.obj \
    $(BUILD_DIR)\sys\mkdir.obj \
    $(BUILD_DIR)\sys\rmdir.obj \
    $(BUILD_DIR)\sys\select.obj \
    $(BUILD_DIR)\sys\sysctl.obj \
    $(BUILD_DIR)\sys\dup.obj \
    $(BUILD_DIR)\sys\dup2.obj \
    $(BUILD_DIR)\sys\cwd.obj \
    $(BUILD_DIR)\sys\utimes.obj \
    $(BUILD_DIR)\sys\_start_a.obj \
    $(BUILD_DIR)\sys\_start_u.obj \
    $(BUILD_DIR)\sys\_shell_start_a.obj \
    $(BUILD_DIR)\sys\_shell_start_u.obj \
\
    $(BUILD_DIR)\gen\arc4random.obj \
    $(BUILD_DIR)\gen\assert.obj \
    $(BUILD_DIR)\gen\errlst.obj \
    $(BUILD_DIR)\gen\err.obj \
    $(BUILD_DIR)\gen\opendir.obj \
    $(BUILD_DIR)\gen\readdir.obj \
    $(BUILD_DIR)\gen\seekdir.obj \
    $(BUILD_DIR)\gen\telldir.obj \
    $(BUILD_DIR)\gen\rewinddir.obj \
    $(BUILD_DIR)\gen\closedir.obj \
    $(BUILD_DIR)\gen\scandir.obj \
    $(BUILD_DIR)\gen\sleep.obj \
    $(BUILD_DIR)\gen\time.obj \
    $(BUILD_DIR)\gen\glob.obj \
    $(BUILD_DIR)\gen\stringlist.obj \
    $(BUILD_DIR)\gen\ResolveFileName.obj \
    $(BUILD_DIR)\gen\utime.obj \
\
    $(BUILD_DIR)\stdtime\asctime.obj \
    $(BUILD_DIR)\stdtime\localtime.obj \
    $(BUILD_DIR)\stdtime\difftime.obj \
    $(BUILD_DIR)\stdtime\strftime.obj \
    $(BUILD_DIR)\stdtime\strptime.obj \
    $(BUILD_DIR)\stdtime\timelocal.obj \
\
    $(BUILD_DIR)\nls\catclose.obj \
    $(BUILD_DIR)\nls\catgets.obj \
    $(BUILD_DIR)\nls\catopen.obj \
    $(BUILD_DIR)\nls\msgcat.obj \

#
#  Processor specific objects
#

!IF "$(PROCESSOR)" == "Ia32"

OBJECTS = $(OBJECTS) \
	$(BUILD_DIR)\i386\gen\setjmp.obj \
	$(BUILD_DIR)\i386\math\math.obj \

!ENDIF

!IF "$(PROCESSOR)" == "Ia64"

OBJECTS = $(OBJECTS) \
	$(BUILD_DIR)\ia64\gen\setjmp.obj \
	$(BUILD_DIR)\ia64\math\math.obj \

!ENDIF

!IF "$(PROCESSOR)" == "Em64t"

OBJECTS = $(OBJECTS) \
	$(BUILD_DIR)\em64t\gen\setjmp.obj \
	$(BUILD_DIR)\em64t\math\math.obj \

!ENDIF

#
# Source file dependencies
#

$(BUILD_DIR)\efi\init.obj              : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\efi_interface.obj     : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\memory.obj            : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\consoleio.obj         : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\fileio.obj            : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\getpagesize.obj       : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\env.obj               : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\efi_time.obj          : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\is_valid_addr.obj     : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\LoadImage.obj         : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\StartImage.obj        : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\UnloadImage.obj       : efi\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\efi\GetFileDevicePath.obj : efi\$(*B).c      $(INC_DEPS)

$(BUILD_DIR)\locale\ansi.obj           : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\collate.obj        : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\collcmp.obj        : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\isctype.obj        : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\none.obj           : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\runetype.obj       : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\table.obj          : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\tolower.obj        : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\toupper.obj        : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\rune.obj           : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\setlocale.obj      : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\lconv.obj          : locale\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\locale\localeconv.obj     : locale\$(*B).c   $(INC_DEPS)

$(BUILD_DIR)\i386\gen\isinf.obj        : i386\gen\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\i386\gen\setjmp.obj       : i386\gen\$(*B).c $(INC_DEPS)

$(BUILD_DIR)\string\bcmp.obj           : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\bcopy.obj          : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\bzero.obj          : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\ffs.obj            : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\index.obj          : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memccpy.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memchr.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memcmp.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memcpy.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memmove.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\memset.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\rindex.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcasecmp.obj     : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcat.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strchr.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcmp.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcoll.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcpy.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strcspn.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strdup.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strerror.obj       : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strlen.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strmode.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strncat.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strncmp.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strncpy.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strpbrk.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strrchr.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strsep.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strspn.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strstr.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strtok.obj         : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\strxfrm.obj        : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\string\swab.obj           : string\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\abort.obj          : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\abs.obj            : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\atexit.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\atoi.obj           : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\atol.obj           : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\exit.obj           : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\getopt.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\labs.obj           : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\malloc.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\rand.obj           : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\random.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\strtod.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\strtol.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\strtoul.obj        : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\strtoq.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\strtouq.obj        : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\qsort.obj          : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\getenv.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\putenv.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\setenv.obj         : stdlib\$(*B).c   $(INC_DEPS)
$(BUILD_DIR)\stdlib\system.obj         : stdlib\$(*B).c   $(INC_DEPS)

$(BUILD_DIR)\stdio\_flock_stub.obj     : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\asprintf.obj        : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\clrerr.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fclose.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fdopen.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\feof.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\ferror.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fflush.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fgetc.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fgetln.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fgetpos.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fgets.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fileno.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\findfp.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\flags.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fopen.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fprintf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fpurge.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fputc.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fputs.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fread.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\freopen.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fscanf.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fseek.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fsetpos.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\ftell.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\funopen.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fvwrite.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fwalk.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\fwrite.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\gets.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\getw.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\makebuf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\mktemp.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\perror.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\printf.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\puts.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\putw.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\refill.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\remove.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\rewind.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\rget.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\scanf.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\setbuf.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\setbuffer.obj       : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\setvbuf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\snprintf.obj        : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\sprintf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\sscanf.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\stdio.obj           : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\tempnam.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\tmpfile.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\tmpnam.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\ungetc.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vasprintf.obj       : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vfprintf.obj        : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vfscanf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vprintf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vscanf.obj          : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vsnprintf.obj       : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vsprintf.obj        : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\vsscanf.obj         : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\wbuf.obj            : stdio\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\stdio\wsetup.obj          : stdio\$(*B).c    $(INC_DEPS)

$(BUILD_DIR)\regex\regcomp.obj         : regex\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\regex\regerror.obj        : regex\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\regex\regexec.obj         : regex\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\regex\regfree.obj         : regex\$(*B).c    $(INC_DEPS)

$(BUILD_DIR)\wchar\wcscat.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcschr.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcscmp.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcscpy.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcscspn.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcslen.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsncat.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsncmp.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsncpy.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcspbrk.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsrchr.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsspn.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcssep.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsstr.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstok.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wgetopt.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmemchr.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmemcmp.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmemcpy.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmemmove.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmemset.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\iswctype.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wctrans.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wctype.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wwbuf.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fputwc.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wrefill.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\rgetw.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fgetwc.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\ungetwc.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fputws.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fgetws.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\putws.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\getws.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fwide.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wopen.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wfopen.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wstat.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wfaststat.obj       : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wmkdir.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wrmdir.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vfwprintf.obj       : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vswprintf.obj       : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vwprintf.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fwprintf.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\swprintf.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wprintf.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vfwscanf.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vswscanf.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\vwscanf.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\fwscanf.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\swscanf.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wscanf.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstod.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstol.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstoul.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstoq.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcstouq.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcscoll.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsftime.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcsxfrm.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wasctime.obj        : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wctime.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\btowc.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wctob.obj           : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\mbsinit.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\mbrlen.obj          : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\mbrtowc.obj         : wchar\$(*B).c    $(INC_DEPS)
$(BUILD_DIR)\wchar\wcrtomb.obj         : wchar\$(*B).c    $(INC_DEPS)

$(BUILD_DIR)\sys\init.obj              : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\__error.obj           : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\_exit.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\close.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\rename.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\fcntl.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\filedesc.obj          : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\getpid.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\gettimeofday.obj      : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\ioctl.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\isatty.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\open.obj              : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\lseek.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\read.obj              : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\stat.obj              : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\faststat.obj          : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\unlink.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\write.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\writev.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\map.obj               : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\getdirentries.obj     : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\mkdir.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\rmdir.obj             : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\select.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\sysctl.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\dup.obj               : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\dup2.obj              : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\cwd.obj               : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\utimes.obj            : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\_start_a.obj          : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\_start_u.obj          : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\_shell_start_a.obj    : sys\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\sys\_shell_start_u.obj    : sys\$(*B).c      $(INC_DEPS)

$(BUILD_DIR)\gen\arc4random.obj        : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\assert.obj            : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\errlst.obj            : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\err.obj               : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\opendir.obj           : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\readdir.obj           : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\seekdir.obj           : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\telldir.obj           : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\rewinddir.obj         : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\closedir.obj          : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\scandir.obj           : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\sleep.obj             : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\time.obj              : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\glob.obj              : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\stringlist.obj        : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\ResolveFileName.obj   : gen\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\gen\utime.obj             : gen\$(*B).c      $(INC_DEPS)

$(BUILD_DIR)\stdtime\asctime.obj       : stdtime\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\stdtime\localtime.obj     : stdtime\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\stdtime\difftime.obj      : stdtime\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\stdtime\strftime.obj      : stdtime\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\stdtime\strptime.obj      : stdtime\$(*B).c  $(INC_DEPS)
$(BUILD_DIR)\stdtime\timelocal.obj     : stdtime\$(*B).c  $(INC_DEPS)

$(BUILD_DIR)\nls\catclose.obj          : nls\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\nls\catgets.obj           : nls\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\nls\catopen.obj           : nls\$(*B).c      $(INC_DEPS)
$(BUILD_DIR)\nls\msgcat.obj            : nls\$(*B).c      $(INC_DEPS)

#
#  Assembly language dependencies
#

$(BUILD_DIR)\ia64\gen\setjmp.obj       : ia64\gen\$(*B).s $(INC_DEPS)
    $(CC) $(CFLAGS_P) /D IA64_ASSEMBLER ia64\gen\$(*B).s > $(@R).pro
    $(ASM) $(AFLAGS) $(MODULE_AFLAGS) $(@R).pro
    del $(@R).pro

$(BUILD_DIR)\em64t\gen\setjmp.obj       : em64t\gen\$(*B).s $(INC_DEPS)
    $(ASM) $(AFLAGS) $(MODULE_AFLAGS) /Fo$@ em64t\gen\$(*B).s


#
# Because libc compiles out of sub-directories, we need some of our own
# inference rules.  $(CC_LINE) is defined in master.mak
#

{efi}.c{$(BUILD_DIR)\efi}.obj:           ; $(CC_LINE)
{locale}.c{$(BUILD_DIR)\locale}.obj:     ; $(CC_LINE)
{string}.c{$(BUILD_DIR)\string}.obj:     ; $(CC_LINE)
{stdlib}.c{$(BUILD_DIR)\stdlib}.obj:     ; $(CC_LINE)
{stdio}.c{$(BUILD_DIR)\stdio}.obj:       ; $(CC_LINE)
{regex}.c{$(BUILD_DIR)\regex}.obj:       ; $(CC_LINE)
{wchar}.c{$(BUILD_DIR)\wchar}.obj:       ; $(CC_LINE)
{sys}.c{$(BUILD_DIR)\sys}.obj:           ; $(CC_LINE)
{gen}.c{$(BUILD_DIR)\gen}.obj:           ; $(CC_LINE)
{stdtime}.c{$(BUILD_DIR)\stdtime}.obj:   ; $(CC_LINE)
{nls}.c{$(BUILD_DIR)\nls}.obj:           ; $(CC_LINE)
{i386\gen}.c{$(BUILD_DIR)\i386\gen}.obj: ; $(CC_LINE)
{ia64\gen}.c{$(BUILD_DIR)\ia64\gen}.obj: ; $(CC_LINE)
{em64t\gen}.c{$(BUILD_DIR)\em64t\gen}.obj: ; $(CC_LINE)
{i386\math}.c{$(BUILD_DIR)\i386\math}.obj: ; $(CC_LINE)
{ia64\math}.c{$(BUILD_DIR)\ia64\math}.obj: ; $(CC_LINE)
{em64t\math}.c{$(BUILD_DIR)\em64t\math}.obj: ; $(CC_LINE)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak

