/*
 * Copyright (c) 1991, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 *
 * Portions copyright (c) 1999, 2000
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
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * $Id: setlocale.c,v 1.1.1.1 2006/05/30 06:13:45 hhzhou Exp $
 */

//#define LIBC_DEBUG


#ifdef LIBC_RCS
static const char rcsid[] =
    "$Id: setlocale.c,v 1.1.1.1 2006/05/30 06:13:45 hhzhou Exp $";
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setlocale.c 8.1 (Berkeley) 7/4/93";
#endif /* LIBC_SCCS and not lint */



#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <locale.h>
#include <rune.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <efi_interface.h>
#include "collate.h"
#include "setlocale.h"

//tc load locale protocol support
#include <wchar.h>
#include <efi.h>
#include <atk_libc.h>
#include <locale_protocol.h>
#include "../stdtime/timelocal.h"   //ben


#include <libc_debug.h>

/*
 * Category names for getenv()
 */
static char *categories[_LC_LAST] = {
    "LC_ALL",
    "LC_COLLATE",
    "LC_CTYPE",
    "LC_MONETARY",
    "LC_NUMERIC",
    "LC_TIME",
    "LC_MESSAGES",
};

/*
 * Current locales for each category
 */
static char current_categories[_LC_LAST][ENCODING_LEN + 1] = {
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
};

/*
 * The locales we are going to try and load
 */
static char new_categories[_LC_LAST][ENCODING_LEN + 1];
static char saved_categories[_LC_LAST][ENCODING_LEN + 1];

static char current_locale_string[_LC_LAST * (ENCODING_LEN + 1/*"/"*/ + 1)];

static char *currentlocale __P((void));
static char *loadlocale __P((int));
static int  stub_load_locale __P((const char *));

//extern int __time_load_locale __P((const char *)); /* strftime.c */

//#ifdef XPG4
//extern int _xpg4_setrunelocale __P((char *));
//#endif

static char *loadlocalefromprotocol __P((int));
static int read_ctype __P((char *));
static int read_collate __P((char *));
static int read_time __P((char *));
static EFI_LOCALE_INTERFACE *getlocaleprotocol __P((char *));






char *
setlocale(category, locale)
    int category;
    const char *locale;
{
    int i, j, len;
    char *env, *r;

    if (category < LC_ALL || category >= _LC_LAST)
        return (NULL);

    if (!locale)
        return (category != LC_ALL ?
            current_categories[category] : currentlocale());

    /*
     * Default to the current locale for everything.
     */
    for (i = 1; i < _LC_LAST; ++i)
        (void)strcpy(new_categories[i], current_categories[i]);

    /*
     * Now go fill up new_categories from the locale argument
     */
    if (!*locale) {

#ifdef _ORG_FREEBSD_
        env = getenv(categories[category]);

        if (category != LC_ALL && (!env || !*env))
            env = getenv(categories[LC_ALL]);

        if (!env || !*env)
#endif
            env = getenv("LANGUAGE");

        if (!env || !*env)
            env = "C";

        (void) strncpy(new_categories[category], env, ENCODING_LEN);
        new_categories[category][ENCODING_LEN] = '\0';
        if (category == LC_ALL) {
            for (i = 1; i < _LC_LAST; ++i) {
#ifdef _ORG_FREEBSD_
                if (!(env = getenv(categories[i])) || !*env)
#endif              
                    env = new_categories[LC_ALL];
                (void)strncpy(new_categories[i], env, ENCODING_LEN);
                new_categories[i][ENCODING_LEN] = '\0';
            }
        }
    } else if (category != LC_ALL)  {
        (void)strncpy(new_categories[category], locale, ENCODING_LEN);
        new_categories[category][ENCODING_LEN] = '\0';
    } else {
        if ((r = strchr(locale, '/')) == NULL) {
            for (i = 1; i < _LC_LAST; ++i) {
                (void)strncpy(new_categories[i], locale, ENCODING_LEN);
                new_categories[i][ENCODING_LEN] = '\0';
            }
        } else {
            for (i = 1; r[1] == '/'; ++r);
            if (!r[1])
                return (NULL);  /* Hmm, just slashes... */
            do {
                len = (int)(r - locale > ENCODING_LEN ? ENCODING_LEN : r - locale);
                (void)strncpy(new_categories[i], locale, len);
                new_categories[i][len] = '\0';
                i++;
                locale = r;
                while (*locale == '/')
                    ++locale;
                while (*++r && *r != '/');
            } while (*locale);
            while (i < _LC_LAST) {
                (void)strcpy(new_categories[i],
                    new_categories[i-1]);
                i++;
            }
        }
    }

    if (category)
        return (loadlocalefromprotocol(category));

    for (i = 1; i < _LC_LAST; ++i) {
        (void)strcpy(saved_categories[i], current_categories[i]);
        DPRINT((L" Loading %d\n",i));
        if (loadlocalefromprotocol(i) == NULL) {
            for (j = 1; j < i; j++) {
                (void)strcpy(new_categories[j],
                     saved_categories[j]);
                /* XXX can fail too */
                (void)loadlocalefromprotocol(j);
            }
            return (NULL);
        }
    }
    return (currentlocale());
}

static char *
currentlocale()
{
    int i;

    (void)strcpy(current_locale_string, current_categories[1]);

    for (i = 2; i < _LC_LAST; ++i)
        if (strcmp(current_categories[1], current_categories[i])) {
            for (i = 2; i < _LC_LAST; ++i) {
                (void) strcat(current_locale_string, "/");
                (void) strcat(current_locale_string, current_categories[i]);
            }
            break;
        }
    return (current_locale_string);
}


static int
stub_load_locale(encoding)
const char *encoding;
{
    //char name[PATH_MAX];
    //struct stat st;

    if (!encoding)
        return(1);
    /*
     * The "C" and "POSIX" locale are always here.
     */
    if (!strcmp(encoding, "C") || !strcmp(encoding, "POSIX"))
        return(0);
    // 
    // if (!_PathLocale)
    //    return(1);
    /* Range checking not needed, encoding has fixed size */
    // strcpy(name, _PathLocale);
    // strcat(name, "/");
    // strcat(name, encoding);
#if 0
    /*
     * Some day we will actually look at this file.
     */
#endif
//  return (stat(name, &st) != 0 || !S_ISDIR(st.st_mode));
    return 0;   

}


//
// load locale protocol support
//
static char *
loadlocalefromprotocol(category)
    int category;
{
    char *ret;
    char *new = new_categories[category];
    char *old = current_categories[category];

    if (strcmp(new, old) == 0)
        return (old);

    if (category == LC_CTYPE) {
        ret = (read_ctype(new) < 0) ? NULL : new;
        if (!ret) {
            (void)read_ctype(old);
        } else
            (void)strcpy(old, new);
        return (ret);
    }

    if (category == LC_COLLATE) {
        ret = (read_collate(new) < 0) ? NULL : new;
        if (!ret) {
            (void)read_collate(old);
        }
        else
            (void)strcpy(old, new);
        return (ret);
    }

    if (category == LC_TIME) {
        ret = (read_time(new) < 0) ? NULL : new;
        if (!ret) {
            (void)read_time(old);
        }           
        else
            (void)strcpy(old, new);
        return (ret);
    }

    if (category == LC_MONETARY ||
        category == LC_MESSAGES ||
        category == LC_NUMERIC) {
        ret = stub_load_locale(new) ? NULL : new;
        if (!ret)
            (void)stub_load_locale(old);
        else
            (void)strcpy(old, new);
        return (ret);
    }

    /* Just in case...*/
    return (NULL);
}



static int
read_ctype(locale)
    char* locale;
{
    EFI_LOCALE_INTERFACE    *pLocale = NULL;
    UINT8   *ctypedata = NULL;
    int     ctypedatasize = 0;
    static  _RuneLocale *rl = NULL;
    
    void *lastp;    
    _RuneEntry *rr;    
    int x;
            
    if (!locale) {
        return -1;
    }

    if (!strcmp(locale, "C") || !strcmp(locale, "POSIX")) {
        _CurrentRuneLocale = &_DefaultRuneLocale;
        return 0;
    }

    pLocale = getlocaleprotocol(locale);
    
    if (!pLocale) {
        return -1;
    }
    
    // set other global variables regarding ctype

    ctypedata = pLocale->getctypedata();
    
    if (!ctypedata) {
        return -1;
    }
    
    ctypedatasize = pLocale->getctypedatasize();
    
    if (ctypedatasize == 0) {
        return -1;
    }
    
    // TODO: read data from ctypedata buffer the fill in rl and other global variables.
    // Made obsolete : _Read_RuneMagi()
    //-------------------------------------    

    if (ctypedatasize < sizeof(_RuneLocale)){
        return -1;
    }
    
    if ( rl != NULL ) free(rl);
    
    if ((rl = (_RuneLocale *) malloc((int) ctypedatasize )) == NULL)
        return -1;

    memcpy(rl, ctypedata, (size_t) ctypedatasize);
    
    lastp = ctypedata + ctypedatasize;

    rl->variable = rl + 1;

    if (strncmp( rl -> magic, _RUNE_MAGIC_1, sizeof(rl -> magic))){
        return -1;
    }

    rl->invalid_rune = ntohl(rl->invalid_rune);
    rl->variable_len = ntohl(rl->variable_len);
    rl->runetype_ext.nranges = ntohl(rl->runetype_ext.nranges);
    rl->maplower_ext.nranges = ntohl(rl->maplower_ext.nranges);
    rl->mapupper_ext.nranges = ntohl(rl->mapupper_ext.nranges);

    for (x = 0; x < _CACHED_RUNES; ++x) {
        rl->runetype[x] = ntohl(rl->runetype[x]);
        rl->maplower[x] = ntohl(rl->maplower[x]);
        rl->mapupper[x] = ntohl(rl->mapupper[x]);
    }

    rl->runetype_ext.ranges = (_RuneEntry *)rl->variable;
    rl->variable = rl->runetype_ext.ranges + rl->runetype_ext.nranges;
    if (rl->variable > lastp) {
        return -1;
    }


    rl->maplower_ext.ranges = (_RuneEntry *)rl->variable;
    rl->variable = rl->maplower_ext.ranges + rl->maplower_ext.nranges;
    
    if (rl->variable > lastp) {
        return -1;
    }   


    rl->mapupper_ext.ranges = (_RuneEntry *)rl->variable;
    rl->variable = rl->mapupper_ext.ranges + rl->mapupper_ext.nranges;

    if (rl->variable > lastp) {
        return -1;
    }


    for (x = 0; x < rl->runetype_ext.nranges; ++x) {
        rr = rl->runetype_ext.ranges;
        rr[x].min = ntohl(rr[x].min);
        rr[x].max = ntohl(rr[x].max);

        if ((rr[x].map = ntohl(rr[x].map)) == 0) {
            int len = rr[x].max - rr[x].min + 1;
            rr[x].types = rl->variable;
            rl->variable = rr[x].types + len;
            if (rl->variable > lastp) {
                return -1;
            }
            while (len-- > 0) {
                rr[x].types[len] = ntohl(rr[x].types[len]);
            }           
        } else
            rr[x].types = 0;
    }


    for (x = 0; x < rl->maplower_ext.nranges; ++x) {
        rr = rl->maplower_ext.ranges;

        rr[x].min = ntohl(rr[x].min);
        rr[x].max = ntohl(rr[x].max);
        rr[x].map = ntohl(rr[x].map);
    }


    for (x = 0; x < rl->mapupper_ext.nranges; ++x) {
        rr = rl->mapupper_ext.ranges;

        rr[x].min = ntohl(rr[x].min);
        rr[x].max = ntohl(rr[x].max);
        rr[x].map = ntohl(rr[x].map);
    }
    

    if (((char *)rl->variable) + rl->variable_len > (char *)lastp) {
        return -1;
    }

    /*
     * Go out and zero pointers that should be zero.
     */
    if (!rl->variable_len)
        rl->variable = 0;

    if (!rl->runetype_ext.nranges)
        rl->runetype_ext.ranges = 0;

    if (!rl->maplower_ext.nranges)
        rl->maplower_ext.ranges = 0;

    if (!rl->mapupper_ext.nranges)
        rl->mapupper_ext.ranges = 0;

    //----------------------------------------
    
    rl->sgetrune = pLocale->sgetrune;
    rl->sputrune = pLocale->sputrune;
    _CurrentRuneLocale = rl;
    pLocale->setinvalidrune(rl->invalid_rune);
    __mb_cur_max = pLocale->getmbcurmax();;
    _CurrentMap = pLocale->getunicodemap();
    _CurrentMapEntryCount = pLocale->getunicodemapentrycount();


    return 0;
}


static int
read_collate(locale)
    char* locale;
{
    EFI_LOCALE_INTERFACE    *pLocale = NULL;
    UINT8   *collatedata = NULL;
    UINT8   *p;
    int     collatedatasize = 0;
    
    
    int save_load_error;
    
    save_load_error = __collate_load_error;
    __collate_load_error = 1;
    if (!locale) {
        __collate_load_error = save_load_error;
        return -1;
    }
    if (!strcmp(locale, "C") || !strcmp(locale, "POSIX"))
        return 0;

    pLocale = getlocaleprotocol(locale);
    
    if (!pLocale) {
        return -1;
    }
    
    // set other global variables regarding collate

    collatedata = pLocale->getcollatedata();
    p = collatedata;
    
    if (!collatedata)   {
        return -1;
    }
    
    collatedatasize = pLocale->getcollatedatasize();
    
    if (collatedatasize == 0)   {
        return -1;
    }

    // TODO: read data from collatedata buffer then fill in global variables
    // Made obsolete : __collate_load_tables()    
    // ----------------------------------------------
    
    strncpy( __collate_version, p, sizeof(__collate_version));    
    p = p + sizeof(__collate_version);
    
    if (strcmp(__collate_version, COLLATE_VERSION) != 0) {
        return -1;
    }
    
    strncpy( (char*)__collate_substitute_table, p, sizeof(__collate_substitute_table));
    p = p + sizeof(__collate_substitute_table);
          
    strncpy( (char*)__collate_char_pri_table, p, sizeof(__collate_char_pri_table));
    p = p + sizeof(__collate_char_pri_table);
    
    strncpy( (char*)__collate_chain_pri_table, p, sizeof(__collate_chain_pri_table));
    p = p + sizeof(__collate_chain_pri_table);
    
    __collate_load_error = 0;
    
    //----------------------------------------------

    return 0;
}


static int
read_time(locale)
    char* locale;
{
    EFI_LOCALE_INTERFACE    *pLocale = NULL;
    UINT8   *timedata = NULL;
    int     timedatasize = 0;
    
    static char *       locale_buf;
    static char     locale_buf_C[] = "C";

    char *          lbuf;
    char *          p;
    const char **       ap;
    const char *        plim;
        
    size_t          namesize;
    size_t          bufsize;
    int                     save_using_locale;

    save_using_locale = _time_using_locale;
    _time_using_locale = 0;

    if (locale == NULL)
        goto no_locale;

    if (!strcmp(locale, "C") || !strcmp(locale, "POSIX"))
        return 0;

    pLocale = getlocaleprotocol(locale);
    
    if (!pLocale) {
        return -1;
    }
    
    // set other global variables regarding time

    timedata = pLocale->gettimedata();
    
    if (!timedata)  {
        return -1;
    }
    
    timedatasize = pLocale->gettimedatasize();
    
    if (timedatasize == 0)  {
        return -1;
    }

    // TODO: read data from timedata buffer the fill in global variables.
    // Made obsolete: __time_load_locale()
    //----------------------------------------
    
    
    /*
    ** If the locale name is the same as our cache, use the cache.
    */
    lbuf = locale_buf;
    if (lbuf != NULL && strcmp(locale, lbuf) == 0) {
        p = lbuf;
        for (ap = (const char **) &_time_localebuf;
            ap < (const char **) (&_time_localebuf + 1);
                ++ap)
                    *ap = p += strlen(p) + 1;
        _time_using_locale = 1;
        return 0;
    }
    /*
    ** Slurp the locale file into the cache.
    */
    namesize = strlen(locale) + 1;

    if (timedatasize <= 0)
        goto bad_locale;
        
    bufsize = namesize + (size_t)timedatasize;
    locale_buf = NULL;
    lbuf = (lbuf == NULL || lbuf == locale_buf_C) ?
        malloc(bufsize) : reallocf(lbuf, bufsize);
    if (lbuf == NULL)
        goto bad_locale;
    (void) strcpy(lbuf, locale);
    p = lbuf + namesize;
    plim = p + timedatasize;
    
    memcpy(p, timedata, (size_t) timedatasize);
        
    /*
    ** Parse the locale file into localebuf.
    */
    if (plim[-1] != '\n')
        goto bad_lbuf;
    for (ap = (const char **) &_time_localebuf;
        ap < (const char **) (&_time_localebuf + 1);
            ++ap) {
                if (p == plim)
                    goto reset_locale;
                *ap = p;
                while (*p != '\n')
                    ++p;
                *p++ = '\0';
    }
    /*
    ** Record the successful parse in the cache.
    */
    locale_buf = lbuf;

    _time_using_locale = 1;
    return 0;

reset_locale:
    /*
     * XXX - This may not be the correct thing to do in this case.
     * setlocale() assumes that we left the old locale alone.
     */
    locale_buf = locale_buf_C;
    _time_localebuf = _C_time_locale;
    save_using_locale = 0;
bad_lbuf:
    free(lbuf);
bad_locale:
    //(void) close(fd);
no_locale:
    _time_using_locale = save_using_locale;
    return -1;

    //-------------------------------------------
        
    //return 0;
}

static EFI_LOCALE_INTERFACE *
getlocaleprotocol(locale)
    char* locale;
{
    EFI_LOCALE_INTERFACE    *pLocale = NULL;
    EFI_GUID LocaleProtocol = EFI_LOCALE_PROTOCOL;

    EFI_HANDLE  LocaleImageHandle = 0;
    EFI_HANDLE  Handles[5];
    BOOLEAN     LoadAttempted = FALSE;
    BOOLEAN     FreeMem = FALSE;
    UINTN       BufSize = sizeof(Handles);
    EFI_HANDLE  *LocaleHandles = Handles;
    BOOLEAN     LocaleProtocolFound = FALSE;
    EFI_STATUS  Status         = EFI_SUCCESS;
    INTN        NumHandles     = 0;
    int         i;
    char        *FileName = NULL;
    wchar_t     *wFileName = NULL;
    
    if (!locale || strlen(locale) > ENCODING_LEN) {
        goto Exit;
    }

    FileName = malloc(FILENAME_MAX);

    if (FileName == NULL) {
        goto Exit;
    }

    wFileName = malloc(FILENAME_MAX * sizeof(wchar_t));
    if (wFileName == NULL) {
        goto Exit;
    }

retry:
    // locate locale protocols
    Status = _GetBootServices()->LocateHandle( 
                     ByProtocol, 
                    &LocaleProtocol, 
                     NULL, 
                    &BufSize, 
                     LocaleHandles );

    if ( Status == EFI_NOT_FOUND ) {
        // none found so load the one we're looking for
retry2: 
        // if we've already attempted an explicit load, fail
        if ( LoadAttempted ) {
            goto Exit;
        }
    
        // construct the filename and attempt to load it
        strcpy( FileName, locale );
        strcat( FileName, ".efi" );

        mbstowcs(wFileName, FileName, FILENAME_MAX);
            
        Status = LoadImage(wFileName, &LocaleImageHandle);

        if (EFI_ERROR(Status)) {
            goto Exit;
        }

        // start locale protocol
        Status = StartImage( LocaleImageHandle,
                             0,
                             NULL,
                             NULL,
                             0,
                             0 );

        if (EFI_ERROR(Status)) {
            goto Exit;
        }

        // let's try this again
        LoadAttempted = TRUE;
        goto retry;

    } else if ( Status == EFI_BUFFER_TOO_SMALL ) {
        // too many handles, allocate enough room.
        // note: we as for one more than needed incase we
        //       attempt another load from below.
        LocaleHandles = malloc(BufSize + sizeof(EFI_HANDLE));
    
        if (LocaleHandles == NULL) {
            goto Exit;
        }
            
        // let's try this again
        FreeMem = TRUE;
        goto retry;
    
    } else if (EFI_ERROR(Status)) {
        // some other error
        goto Exit;
    }

    // go through all returned handes looking for the requested locale
    NumHandles = BufSize / sizeof(EFI_HANDLE);

    for (i = 0; i < NumHandles; i++) {
        Status = _GetBootServices()->HandleProtocol( 
                         LocaleHandles[i], 
                        &LocaleProtocol, 
                        (VOID**)&pLocale 
                         );

        if ( EFI_ERROR( Status ) ) {
            goto Exit;
        }
        
        if ( strcmp( pLocale->getlocale(), locale ) == 0 ) {
            // Got it!
            LocaleProtocolFound = TRUE; // everything is well
    
            goto Exit;
        }
    }

    // No match, try to explicitly load the protocol
    goto retry2;
    
Exit:
    // free memory if needed    

    if (FileName) {
        free(FileName);
        FileName = NULL;
    }

    if (wFileName) {
        free(wFileName);
        wFileName = NULL;
    }

    if ( FreeMem ) {
        free(LocaleHandles);
        LocaleHandles = NULL;
    }

    if (!pLocale) {
        return NULL;
    }

    if (LocaleProtocolFound) {
        return pLocale;
    }
    
    return NULL;
}
