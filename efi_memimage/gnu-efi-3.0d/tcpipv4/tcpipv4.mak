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
# Set the base output name and type for this makefile
#

BASE_NAME = tcpipv4

#
# Set entry point
#

IMAGE_ENTRY_POINT = EfiNetEntry

#
# Globals needed by master.mak
#

TARGET_BS_DRIVER = $(BASE_NAME)
SOURCE_DIR       = $(SDK_INSTALL_DIR)\protocols\$(BASE_NAME)
BUILD_DIR        = $(SDK_BUILD_DIR)\protocols\$(BASE_NAME)

PPP_SUPPORT = YES

#
# Additional compile flags
#

C_FLAGS = /D __STDC__ /D KERNEL /D__inline__=__inline $(C_FLAGS)

!IF "$(PPP_SUPPORT)" == "YES"

C_FLAGS = /D PPP_SUPPORT $(C_FLAGS)

!ENDIF

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include makefile.hdr
INC = -I . $(INC)

#
# Libraries
#

LIBS = $(LIBS) $(SDK_BUILD_DIR)\lib\libc\libc.lib

#
# Main targets
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\efi_init.obj \
    $(BUILD_DIR)\efi_interface.obj \
    $(BUILD_DIR)\efi_kern_support.obj \
    $(BUILD_DIR)\efi_netiface.obj \
    $(BUILD_DIR)\if.obj \
    $(BUILD_DIR)\if_ether.obj \
    $(BUILD_DIR)\if_ethersubr.obj \
    $(BUILD_DIR)\if_loop.obj \
    $(BUILD_DIR)\igmp.obj \
    $(BUILD_DIR)\in.obj \
    $(BUILD_DIR)\in_cksum.obj \
    $(BUILD_DIR)\in_pcb.obj \
    $(BUILD_DIR)\in_proto.obj \
    $(BUILD_DIR)\in_rmx.obj \
    $(BUILD_DIR)\ip_flow.obj \
    $(BUILD_DIR)\ip_icmp.obj \
    $(BUILD_DIR)\ip_input.obj \
    $(BUILD_DIR)\ip_mroute.obj \
    $(BUILD_DIR)\ip_output.obj \
    $(BUILD_DIR)\kern_timeout.obj \
    $(BUILD_DIR)\param.obj \
    $(BUILD_DIR)\radix.obj \
    $(BUILD_DIR)\random.obj \
    $(BUILD_DIR)\raw_cb.obj \
    $(BUILD_DIR)\raw_ip.obj \
    $(BUILD_DIR)\raw_usrreq.obj \
    $(BUILD_DIR)\route.obj \
    $(BUILD_DIR)\rtsock.obj \
    $(BUILD_DIR)\tcp_debug.obj \
    $(BUILD_DIR)\tcp_input.obj \
    $(BUILD_DIR)\tcp_output.obj \
    $(BUILD_DIR)\tcp_subr.obj \
    $(BUILD_DIR)\tcp_timer.obj \
    $(BUILD_DIR)\tcp_usrreq.obj \
    $(BUILD_DIR)\to_resolve.obj \
    $(BUILD_DIR)\udp_usrreq.obj \
    $(BUILD_DIR)\uipc_domain.obj \
    $(BUILD_DIR)\uipc_mbuf.obj \
    $(BUILD_DIR)\uipc_socket.obj \
    $(BUILD_DIR)\uipc_socket2.obj \
    $(BUILD_DIR)\vm_zone.obj \
!IF "$(PPP_SUPPORT)" == "YES"
    $(BUILD_DIR)\if_ppp.obj \
    $(BUILD_DIR)\slcompress.obj \
    $(BUILD_DIR)\ppp_tty.obj \
    $(BUILD_DIR)\tty_subr.obj \
!ENDIF

#
# Source file dependencies
#

$(BUILD_DIR)\efi_init.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efi_interface.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efi_kern_support.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efi_netiface.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\if.obj               : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\if_ether.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\if_ethersubr.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\if_loop.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\igmp.obj             : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\in.obj               : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\in_cksum.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\in_pcb.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\in_proto.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\in_rmx.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ip_flow.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ip_icmp.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ip_input.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ip_mroute.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ip_output.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\kern_timeout.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\param.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\radix.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\random.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\raw_cb.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\raw_ip.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\raw_usrreq.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\route.obj            : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\rtsock.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_debug.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_input.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_output.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_subr.obj         : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_timer.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tcp_usrreq.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\to_resolve.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\udp_usrreq.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\uipc_domain.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\uipc_mbuf.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\uipc_socket.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\uipc_socket2.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\vm_zone.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\if_ppp.obj           : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\slcompress.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ppp_tty.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\tty_subr.obj         : $(*B).c $(INC_DEPS)

#
# Handoff to Master.Mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
