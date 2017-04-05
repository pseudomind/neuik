/*******************************************************************************
 * Copyright (c) 2014-2017, Michael Leimon <leimon@gmail.com>
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
	int             eNum      = 0; /* which error to report (if any) */
	struct stat     statRes;
	static size_t   baseLen   = 0;
	static char     sysDir[]  = "C:\\Windows\\Fonts\\";
	static char     funcName[] = "NEUIK_GetTTFLocation";
	static char   * errMsgs[] = {"",        // [0] no error
		"Base fontName is NULL/empty.", // [1]
		"Failed to allocate memory.",   // [2]
		"Pointer to `loc` is NULL.",    // [3]
	};

	if (baseLen == 0)
	{
		/* baseLen setup: Only happens once */
		baseLen = strlen(sysDir) + strlen(".ttf") + 1;
	}

	if (loc == NULL)
	{
		eNum = 3;
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
	/* Check in the `System` (c:/Windows/Fonts) directory. */
	sprintf((*loc), "C:\\Windows\\Fonts\\%s.ttf", fName);
	if (!stat((*loc), &statRes)) goto out;

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
	int           eNum   = 0; /* which error to report (if any) */
	size_t        fLen   = 0;
	char        * bfName = NULL;
	static char   funcName[] = "NEUIK_GetBoldTTFLocation";
	static char * errMsgs[] = { "",            // [0] no error
		"Base fontName is NULL/empty.",        // [1]
		"Failed to allocate memory",           // [2]
		"Failure in `NEUIK_GetTTFLocation()`", // [3]
	};

	if (fName == NULL)
	{
		eNum = 1;
		goto out;
	}

	fLen = strlen(fName);
	bfName = malloc((fLen + 2)*(sizeof(char)));
	if (bfName == NULL)
	{
		eNum = 2;
		goto out;
	}
	sprintf(bfName, "%sb", fName);

	if (NEUIK_GetTTFLocation(bfName, loc))
	{
		eNum = 3;
		goto out;
	}
out:
	if (eNum != 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (bfName != NULL) free(bfName);
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
	int           eNum = 0; /* which error to report (if any) */
	size_t        fLen = 0;
	char        * ifName = NULL;
	static char   funcName[] = "NEUIK_GetItalicTTFLocation";
	static char * errMsgs[] = { "",            // [0] no error
		"Base fontName is NULL/empty.",        // [1]
		"Failed to allocate memory",           // [2]
		"Failure in `NEUIK_GetTTFLocation()`", // [3]
	};

	if (fName == NULL)
	{
		eNum = 1;
		goto out;
	}

	fLen = strlen(fName);
	ifName = malloc((fLen + 2)*(sizeof(char)));
	if (ifName == NULL)
	{
		eNum = 2;
		goto out;
	}
	sprintf(ifName, "%si", fName);

	if (NEUIK_GetTTFLocation(ifName, loc))
	{
		eNum = 3;
		goto out;
	}
out:
	if (eNum != 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (ifName != NULL) free(ifName);
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
	int           eNum = 0; /* which error to report (if any) */
	size_t        fLen = 0;
	char        * zfName = NULL;
	static char   funcName[] = "NEUIK_GetBoldItalicTTFLocation";
	static char * errMsgs[] = { "",            // [0] no error
		"Base fontName is NULL/empty.",        // [1]
		"Failed to allocate memory",           // [2]
		"Failure in `NEUIK_GetTTFLocation()`", // [3]
	};

	if (fName == NULL)
	{
		eNum = 1;
		goto out;
	}

	fLen = strlen(fName);
	zfName = malloc((fLen + 2)*(sizeof(char)));
	if (zfName == NULL)
	{
		eNum = 2;
		goto out;
	}
	sprintf(zfName, "%sz", fName);

	if (NEUIK_GetTTFLocation(zfName, loc))
	{
		eNum = 3;
		goto out;
	}
out:
	if (eNum != 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (zfName != NULL) free(zfName);
	return eNum;
}
