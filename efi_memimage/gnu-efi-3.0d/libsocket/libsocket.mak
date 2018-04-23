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
# Set the base output name
#

BASE_NAME = libsocket

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)\lib\$(BASE_NAME)
BUILD_DIR  = $(SDK_BUILD_DIR)\lib\$(BASE_NAME)

#
# Additional compiler flags
#

C_FLAGS = /D __STDC__ $(C_FLAGS)

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

!include $(SOURCE_DIR)\makefile.hdr
INC = -I . $(INC)


#
# Default targets
#

all : dirs $(OBJECTS)

#
# Library object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\efisocketinit.obj \
    $(BUILD_DIR)\efisocketio.obj \
    $(BUILD_DIR)\accept.obj \
    $(BUILD_DIR)\addr2ascii.obj \
    $(BUILD_DIR)\ascii2addr.obj \
    $(BUILD_DIR)\base64.obj \
    $(BUILD_DIR)\bind.obj \
    $(BUILD_DIR)\connect.obj \
    $(BUILD_DIR)\ether_addr.obj \
    $(BUILD_DIR)\gethostbydns.obj \
    $(BUILD_DIR)\gethostbyht.obj \
    $(BUILD_DIR)\gethostbynis.obj \
    $(BUILD_DIR)\gethostnamadr.obj \
    $(BUILD_DIR)\gethostname.obj \
    $(BUILD_DIR)\getnetbydns.obj \
    $(BUILD_DIR)\getnetbyht.obj \
    $(BUILD_DIR)\getnetbynis.obj \
    $(BUILD_DIR)\getnetnamadr.obj \
    $(BUILD_DIR)\getpeername.obj \
    $(BUILD_DIR)\getproto.obj \
    $(BUILD_DIR)\getprotoent.obj \
    $(BUILD_DIR)\getprotoname.obj \
    $(BUILD_DIR)\getservbyname.obj \
    $(BUILD_DIR)\getservbyport.obj \
    $(BUILD_DIR)\getservent.obj \
    $(BUILD_DIR)\getsockname.obj \
    $(BUILD_DIR)\getsockopt.obj \
    $(BUILD_DIR)\herror.obj \
    $(BUILD_DIR)\inet_addr.obj \
    $(BUILD_DIR)\inet_lnaof.obj \
    $(BUILD_DIR)\inet_makeaddr.obj \
    $(BUILD_DIR)\inet_net_ntop.obj \
    $(BUILD_DIR)\inet_net_pton.obj \
    $(BUILD_DIR)\inet_neta.obj \
    $(BUILD_DIR)\inet_netof.obj \
    $(BUILD_DIR)\inet_network.obj \
    $(BUILD_DIR)\inet_ntoa.obj \
    $(BUILD_DIR)\inet_ntop.obj \
    $(BUILD_DIR)\inet_pton.obj \
    $(BUILD_DIR)\linkaddr.obj \
    $(BUILD_DIR)\listen.obj \
    $(BUILD_DIR)\map_v4v6.obj \
    $(BUILD_DIR)\ns_addr.obj \
    $(BUILD_DIR)\ns_name.obj \
    $(BUILD_DIR)\ns_netint.obj \
    $(BUILD_DIR)\ns_ntoa.obj \
    $(BUILD_DIR)\ns_parse.obj \
    $(BUILD_DIR)\ns_print.obj \
    $(BUILD_DIR)\ns_ttl.obj \
    $(BUILD_DIR)\nsap_addr.obj \
    $(BUILD_DIR)\pollsocket.obj \
    $(BUILD_DIR)\recv.obj \
    $(BUILD_DIR)\recvfrom.obj \
    $(BUILD_DIR)\res_comp.obj \
    $(BUILD_DIR)\res_data.obj \
    $(BUILD_DIR)\res_debug.obj \
    $(BUILD_DIR)\res_init.obj \
    $(BUILD_DIR)\res_mkquery.obj \
    $(BUILD_DIR)\res_mkupdate.obj \
    $(BUILD_DIR)\res_query.obj \
    $(BUILD_DIR)\res_send.obj \
    $(BUILD_DIR)\res_update.obj \
    $(BUILD_DIR)\send.obj \
    $(BUILD_DIR)\sendto.obj \
    $(BUILD_DIR)\sethostname.obj \
    $(BUILD_DIR)\setsockopt.obj \
    $(BUILD_DIR)\shutdown.obj \
    $(BUILD_DIR)\socket.obj \

#
# Source file dependencies
#

$(BUILD_DIR)\efisocketinit.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\efisocketio.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\accept.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\addr2ascii.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ascii2addr.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\base64.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\bind.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\connect.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ether_addr.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\gethostbydns.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\gethostbyht.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\gethostbynis.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\gethostnamadr.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\gethostname.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getnetbydns.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getnetbyht.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getnetbynis.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getnetnamadr.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getpeername.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getproto.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getprotoent.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getprotoname.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getservbyname.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getservbyport.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getservent.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getsockname.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\getsockopt.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\herror.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_addr.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_lnaof.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_makeaddr.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_net_ntop.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_net_pton.obj : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_neta.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_netof.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_network.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_ntoa.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_ntop.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\inet_pton.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\linkaddr.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\listen.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\map_v4v6.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_addr.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_name.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_netint.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_ntoa.obj       : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_parse.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_print.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\ns_ttl.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\nsap_addr.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\pollsocket.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\recv.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\recvfrom.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_comp.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_data.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_debug.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_init.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_mkquery.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_mkupdate.obj  : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_query.obj     : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_send.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\res_update.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\send.obj          : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sendto.obj        : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\sethostname.obj   : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\setsockopt.obj    : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\shutdown.obj      : $(*B).c $(INC_DEPS)
$(BUILD_DIR)\socket.obj        : $(*B).c $(INC_DEPS)

#
# Handoff to Master.Mak
#

!include $(SDK_INSTALL_DIR)\build\master.mak
