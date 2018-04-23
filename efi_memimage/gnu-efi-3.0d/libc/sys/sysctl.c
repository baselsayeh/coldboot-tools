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

  Name:
      sysctl

  Description:
      A pseudo BSD sysctl interface for libc

  Arguments:
      MIB array
      MIB array length
      Old value pointer
      Old value length
      New value pointer
      New value length

  Returns:
      0 on success, -1 otherwise

--*/

#include <efi_interface.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdarg.h>
#ifndef KENRL
#define KERNEL
#include <errno.h>
#include <sys/sysctl.h>
#undef KERNEL
#else
#include <errno.h>
#include <sys/sysctl.h>
#endif
#include "./filedesc.h"

#include <sys/socket.h>
#include <unistd.h>

#include <libc_debug.h>
#include <assert.h>

/*
 * Transfer function to/from "user" space.
 */
static int
sysctl_old_user(struct sysctl_req *req, const void *p, size_t l)
{
        size_t i = 0;

        if (req->oldptr) {
                i = l;
                if (i > req->oldlen - req->oldidx)
                        i = req->oldlen - req->oldidx;
                if (i > 0)
                        bcopy(p, (char *)req->oldptr + req->oldidx, i);
        }
        req->oldidx += l;
        if (req->oldptr && i < l)
                return (ENOMEM);
        return (0);
}

static int
sysctl_new_user(struct sysctl_req *req, void *p, size_t l)
{
        if (!req->newptr)
                return 0;
        if (req->newlen - req->newidx < l)
                return (EINVAL);
        bcopy((char *)req->newptr + req->newidx, p, l);
        req->newidx += l;
        return (0);
}

int
sysctl(
	int		*name,
	u_int	namelen,
	void	*oldp,
	size_t	*oldlenp,
	void	*newp,
	size_t	newlen
	)
{
	struct pseudo_sysctl	req;
	int						fd, ret;

    //
	//  Use top-level identifier to determine how to handle request
	//

    switch (*name) {

    	//
    	//  Deal with network releated requests
    	//

    	case CTL_NET:
    		fd = socket (AF_INET, SOCK_DGRAM, 0);
    		if (fd == -1) {
    			return (-1);
    		}
    		break;

    	//
    	//  Request is not supported
    	//

    	default:
    		errno = EOPNOTSUPP;
    		return -1;
    }

    //
    //  Format the request
    //

	req.name        = name;
	req.namelen     = namelen;
	req.req.oldptr  = oldp;
	req.req.oldlen  = oldlenp ? *oldlenp : 0;
	req.req.oldidx  = 0;
	req.req.oldfunc = sysctl_old_user;
    req.req.newptr  = newp;
    req.req.newlen  = newlen;
	req.req.newidx  = 0;
	req.req.newfunc = sysctl_new_user;

    //
    //  Make the ioctl call
    //

    ret = ioctl (fd, IOCSYSCTL, &req);
   	close (fd);
   	
   	if ( oldlenp )
   		*oldlenp = req.req.oldidx;

    return (ret);
}
