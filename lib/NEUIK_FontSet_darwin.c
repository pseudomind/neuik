/*******************************************************************************
 * Copyright (c) 2014-2020, Michael Leimon <leimon@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "NEUIK_error.h"


/*******************************************************************************
 *
 *  Name:          NEUIK_GetTTFLocation
 *
 *  Description:   Determines the location of the desired system font.
 *
 *  Returns:       A non-zero value if there is an error. Not finding the 
 *                 desired font is not considered an error however, in such a 
 *                 case the location argument will be set to NULL.
 *
 ******************************************************************************/
int NEUIK_GetTTFLocation(
    const char  * fName, /* Base font name */
    char       ** loc)   /* Location of the desired font */
{
    int                    eNum       = 0; /* which error to report (if any) */
    struct stat            statRes;
    static unsigned long   baseLen    = 0;
    static char            userDir[]  = "~/Library/Fonts/";
    static char            locDir[]   = "/Library/Fonts/";
    static char            sysDir[]   = "/System/Library/Fonts/";
    static char            funcName[] = "NEUIK_GetTTFLocation";
    static char          * errMsgs[]  = {"", // [0] no error
        "Base fontName is NULL/empty.",      // [1]
        "Failed to allocate memory.",        // [2]
        "Unable to locate font.",            // [3]
        "Pointer to `loc` is NULL.",         // [4]
    };

    if (baseLen == 0)
    {
        /* baseLen setup: Only happens once */
        baseLen = strlen(sysDir) + strlen(".ttf") + 1;
    }

    if (loc == NULL)
    {
        eNum = 4;
        goto out;
    }
    if (fName == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (*fName == 0)
    {
        eNum = 1;
        goto out;

    }

    (*loc) = (char *)malloc((baseLen + strlen(fName))*sizeof(char));
    if ((*loc) == NULL)
    {
        eNum = 2;
        goto out;
    }
    /* Check in the `User` (~/Library/Fonts) directory. */
    sprintf((*loc), "%s%s.ttf", userDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `Local` (/Library/Fonts) directory. */
    sprintf((*loc), "%s%s.ttf", locDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `System` (/System/Library/Fonts) directory. */
    sprintf((*loc), "%s%s.ttf", sysDir, fName);
    if (stat((*loc), &statRes))
    {
        /* If the font wasn't found here, then it wasn't found in any loc. */
        free((*loc));
        (*loc) = NULL;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_GetBoldTTFLocation
 *
 *  Description:   Determines the location of the desired system font.
 *
 *  Returns:       A non-zero value if there is an error. Not finding the 
 *                 desired font is not considered an error however, in such a 
 *                 case the location argument will be set to NULL.
 *
 ******************************************************************************/
int NEUIK_GetBoldTTFLocation(
    const char  * fName, /* Base font name */
    char       ** loc)   /* Location of the desired font */
{
    int                    eNum       = 0; /* which error to report (if any) */
    struct stat            statRes;
    static unsigned long   baseLen    = 0;
    static char            userDir[]  = "~/Library/Fonts/";
    static char            locDir[]   = "/Library/Fonts/";
    static char            sysDir[]   = "/System/Library/Fonts/";
    static char            funcName[] = "NEUIK_GetBoldTTFLocation";
    static char          * errMsgs[]  = {"", // [0] no error
        "Base fontName is NULL/empty.",      // [1]
        "Failed to allocate memory.",        // [2]
        "Unable to locate font.",            // [3]
        "Pointer to `loc` is NULL.",         // [4]
    };

    if (baseLen == 0)
    {
        /* baseLen setup: Only happens once */
        baseLen = strlen(sysDir) + strlen(" Bold.ttf") + 1;
    }

    if (loc == NULL)
    {
        eNum = 4;
        goto out;
    }
    if (fName == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (*fName == 0)
    {
        eNum = 1;
        goto out;

    }

    (*loc) = (char *)malloc((baseLen + strlen(fName))*sizeof(char));
    if ((*loc) == NULL)
    {
        eNum = 2;
        goto out;
    }
    /* Check in the `User` (~/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold.ttf", userDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `Local` (/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold.ttf", locDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `System` (/System/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold.ttf", sysDir, fName);
    if (stat((*loc), &statRes))
    {
        /* If the font wasn't found here, then it wasn't found in any loc. */
        free((*loc));
        (*loc) = NULL;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_GetItalicTTFLocation
 *
 *  Description:   Determines the location of the desired system font.
 *
 *  Returns:       A non-zero value if there is an error. Not finding the 
 *                 desired font is not considered an error however, in such a 
 *                 case the location argument will be set to NULL.
 *
 ******************************************************************************/
int NEUIK_GetItalicTTFLocation(
    const char  * fName, /* Base font name */
    char       ** loc)   /* Location of the desired font */
{
    int                    eNum       = 0; /* which error to report (if any) */
    struct stat            statRes;
    static unsigned long   baseLen    = 0;
    static char            userDir[]  = "~/Library/Fonts/";
    static char            locDir[]   = "/Library/Fonts/";
    static char            sysDir[]   = "/System/Library/Fonts/";
    static char            funcName[] = "NEUIK_GetItalicTTFLocation";
    static char          * errMsgs[]  = {"", // [0] no error
        "Base fontName is NULL/empty.",      // [1]
        "Failed to allocate memory.",        // [2]
        "Unable to locate font.",            // [3]
        "Pointer to `loc` is NULL.",         // [4]
    };

    if (baseLen == 0)
    {
        /* baseLen setup: Only happens once */
        baseLen = strlen(sysDir) + strlen(" Italic.ttf") + 1;
    }

    if (loc == NULL)
    {
        eNum = 4;
        goto out;
    }
    if (fName == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (*fName == 0)
    {
        eNum = 1;
        goto out;

    }

    (*loc) = (char *)malloc((baseLen + strlen(fName))*sizeof(char));
    if ((*loc) == NULL)
    {
        eNum = 2;
        goto out;
    }
    /* Check in the `User` (~/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Italic.ttf", userDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `Local` (/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Italic.ttf", locDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `System` (/System/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Italic.ttf", sysDir, fName);
    if (stat((*loc), &statRes))
    {
        /* If the font wasn't found here, then it wasn't found in any loc. */
        free((*loc));
        (*loc) = NULL;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_GetBoldItalicTTFLocation
 *
 *  Description:   Determines the location of the desired system font.
 *
 *  Returns:       A non-zero value if there is an error. Not finding the 
 *                 desired font is not considered an error however, in such a 
 *                 case the location argument will be set to NULL.
 *
 ******************************************************************************/
int NEUIK_GetBoldItalicTTFLocation(
    const char  * fName, /* Base font name */
    char       ** loc)   /* Location of the desired font */
{
    int                    eNum       = 0; /* which error to report (if any) */
    struct stat            statRes;
    static unsigned long   baseLen    = 0;
    static char            userDir[]  = "~/Library/Fonts/";
    static char            locDir[]   = "/Library/Fonts/";
    static char            sysDir[]   = "/System/Library/Fonts/";
    static char            funcName[] = "NEUIK_GetBoldItalicTTFLocation";
    static char          * errMsgs[]  = {"", // [0] no error
        "Base fontName is NULL/empty.",      // [1]
        "Failed to allocate memory.",        // [2]
        "Unable to locate font.",            // [3]
        "Pointer to `loc` is NULL.",         // [4]
    };

    if (baseLen == 0)
    {
        /* baseLen setup: Only happens once */
        baseLen = strlen(sysDir) + strlen(" Bold Italic.ttf") + 1;
    }

    if (loc == NULL)
    {
        eNum = 4;
        goto out;
    }
    if (fName == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (*fName == 0)
    {
        eNum = 1;
        goto out;

    }

    (*loc) = (char *)malloc((baseLen + strlen(fName))*sizeof(char));
    if ((*loc) == NULL)
    {
        eNum = 2;
        goto out;
    }
    /* Check in the `User` (~/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold Italic.ttf", userDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `Local` (/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold Italic.ttf", locDir, fName);
    if (!stat((*loc), &statRes)) goto out;

    /* Check in the `System` (/System/Library/Fonts) directory. */
    sprintf((*loc), "%s%s Bold Italic.ttf", sysDir, fName);
    if (stat((*loc), &statRes))
    {
        /* If the font wasn't found here, then it wasn't found in any loc. */
        free((*loc));
        (*loc) = NULL;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

