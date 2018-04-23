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

BASE_NAME = libefi

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\lib\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\lib\$(BASE_NAME)

#
# Additional compile flags
#


#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\efishell\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\efishell \
      -I $(SDK_INSTALL_DIR)\include\efishell\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\protocol\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\protocol \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\protocol\$(PROCESSOR) $(INC)

!include .\makefile.hdr
INC = -I . \
      -I .\$(PROCESSOR) $(INC)

#
# Defaulit target
#

all : sub_dirs $(OBJECTS)

#
# Local sub directories
#

sub_dirs : $(BUILD_DIR)\runtime \
           $(BUILD_DIR)\$(PROCESSOR) \
	
#
# Directory targets
#

$(BUILD_DIR)\runtime : ; - md $(BUILD_DIR)\runtime

#
#  Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\boxdraw.obj \
    $(BUILD_DIR)\console.obj \
    $(BUILD_DIR)\crc.obj \
    $(BUILD_DIR)\data.obj \
    $(BUILD_DIR)\debug.obj \
    $(BUILD_DIR)\dpath.obj \
    $(BUILD_DIR)\error.obj \
    $(BUILD_DIR)\event.obj \
    $(BUILD_DIR)\guid.obj \
    $(BUILD_DIR)\hand.obj \
    $(BUILD_DIR)\init.obj \
    $(BUILD_DIR)\lock.obj \
    $(BUILD_DIR)\misc.obj \
    $(BUILD_DIR)\print.obj \
    $(BUILD_DIR)\sread.obj \
    $(BUILD_DIR)\str.obj \
    $(BUILD_DIR)\hw.obj \
    $(BUILD_DIR)\smbios.obj \
    $(BUILD_DIR)\systable.obj\
\
    $(BUILD_DIR)\runtime\efirtlib.obj \
    $(BUILD_DIR)\runtime\lock.obj \
    $(BUILD_DIR)\runtime\rtdata.obj \
    $(BUILD_DIR)\runtime\str.obj \
    $(BUILD_DIR)\runtime\vm.obj \

#
#  Processor dependent targets
#

!IF "$(PROCESSOR)" == "Ia32"

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\$(PROCESSOR)\initplat.obj \
    $(BUILD_DIR)\$(PROCESSOR)\math.obj \

!ENDIF

!IF "$(PROCESSOR)" == "Ia64"

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\$(PROCESSOR)\initplat.obj \
    $(BUILD_DIR)\$(PROCESSOR)\math.obj \
    $(BUILD_DIR)\$(PROCESSOR)\salpal.obj \
    $(BUILD_DIR)\$(PROCESSOR)\palproc.obj \

!ENDIF

!IF "$(PROCESSOR)" == "Em64t"

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\$(PROCESSOR)\initplat.obj \
    $(BUILD_DIR)\$(PROCESSOR)\math.obj \

!ENDIF

#
# Source file dependencies
#

$(BUILD_DIR)\boxdraw.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\console.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\crc.obj              : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\data.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\debug.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\dpath.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\error.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\event.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\guid.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\hand.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\init.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\lock.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\misc.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\print.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sread.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\str.obj              : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\hw.obj               : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\smbios.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\systable.obj         : $(*B).c $(INC_DEPS)

$(BUILD_DIR)\runtime\efirtlib.obj : runtime\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\runtime\lock.obj     : runtime\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\runtime\rtdata.obj   : runtime\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\runtime\str.obj      : runtime\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\runtime\vm.obj       : runtime\$(*B).c $(INC_DEPS)

#
#  Processor dependent targets
#

$(BUILD_DIR)\$(PROCESSOR)\initplat.obj : $(PROCESSOR)\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\$(PROCESSOR)\math.obj     : $(PROCESSOR)\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\$(PROCESSOR)\salpal.obj   : $(PROCESSOR)\$(*B).c $(INC_DEPS)
$(BUILD_DIR)\$(PROCESSOR)\palproc.obj  : $(PROCESSOR)\$(*B).s $(INC_DEPS)

#
# Because libefi compiles out of sub-directories, we need some of our own
# inference rules.  $(CC_LINE) is defined in master.mak
#

{runtime}.c{$(BUILD_DIR)\runtime}.obj: ; $(CC_LINE)

#
# Handoff to master.mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
