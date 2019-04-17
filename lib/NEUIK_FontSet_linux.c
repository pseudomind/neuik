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
	int           eNum         = 0; /* which error to report (if any) */
	FILE        * cmdOut       = NULL;
	char        * cmdStr       = NULL;
	char        * strPtr       = NULL;
	char          buffer[2048];
	static char   baseMatch[]  = "fc-match ";
	static char   vMatch[]     = "fc-match -v ";
	static char   grepStr[]    = " | grep file:";
	static char   funcName[]   = "NEUIK_GetTTFLocation";
	static char * errMsgs[]    = {"",              // [0] no error
		"Base fontName is NULL/empty.",            // [1]
		"Failed to allocate memory.",              // [2]
		"Unable to locate font.",                  // [3]
		"Pointer to `loc` is NULL.",               // [4]
		"Fontconfig tool `fc-match` not in path.", // [5]
		"Obtained a NULL path.",                   // [6]
	};

	if (loc == NULL)
	{
		eNum = 4;
		goto out;
	}
	/* if not otherwise set, NULL will indicate that this font was invalid. */
	(*loc) = NULL;

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

	/*------------------------------------------------------------------------*/
	/* First verify that fontconfig is present on the system                  */
	/*------------------------------------------------------------------------*/
	if (system("which fc-match > /dev/null 2>&1"))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Form initial `fc-match` call (malloc for maximum size possibly needed) */
	/*------------------------------------------------------------------------*/
	cmdStr = (char *)malloc(
		(1 + strlen(vMatch) + strlen(fName) + strlen(grepStr))*sizeof(char));
	if (cmdStr == NULL)
	{
		eNum = 2;
		goto out;
	}

	sprintf(cmdStr, "%s%s", baseMatch, fName);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the suggested font filename and verify that it is a TTF file.  */
	/*------------------------------------------------------------------------*/
	if (strchr(buffer, ':'))
	{
		*(strchr(buffer, ':')) = '\0';
	}
	strPtr = strrchr(buffer, '.');
	if (strPtr == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* This would be an invalid file format... skip                       */
		/*--------------------------------------------------------------------*/
		goto out;
	}
	else
	{
		strPtr += 1;
	}
	if (strcmp("ttf", strPtr))
	{
		/* This font is not a TTF type, just return. */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Otherwise this appears to be a TTF-type font.                          */
	/*------------------------------------------------------------------------*/
	sprintf(cmdStr, "%s%s%s", vMatch, fName, grepStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the file path for the font.                                    */
	/*------------------------------------------------------------------------*/
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) { *strPtr = '\0'; }
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) strPtr += 1;

	/*------------------------------------------------------------------------*/
	/* At this point the font name has been isolated, copy it out.            */
	/*------------------------------------------------------------------------*/
	if (strPtr == NULL)
	{
		eNum = 6;
		goto out;
	}
	(*loc) = (char *)malloc((1 + strlen(strPtr))*sizeof(char));
	if ((*loc) == NULL)
	{
		eNum = 2;
		goto out;
	}

	strcpy(*loc, strPtr);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (cmdStr != NULL) free(cmdStr);

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
	int           eNum         = 0; /* which error to report (if any) */
	FILE        * cmdOut       = NULL;
	char        * cmdStr       = NULL;
	char        * strPtr       = NULL;
	char          buffer[2048];
	static char   baseMatch[]  = "fc-match ";
	static char   vMatch[]     = "fc-match -v ";
	static char   optStr[]     = ":bold";
	static char   grepStr[]    = " | grep file:";
	static char   funcName[]   = "NEUIK_GetBoldTTFLocation";
	static char * errMsgs[]    = {"",              // [0] no error
		"Base fontName is NULL/empty.",            // [1]
		"Failed to allocate memory.",              // [2]
		"Unable to locate font.",                  // [3]
		"Pointer to `loc` is NULL.",               // [4]
		"Fontconfig tool `fc-match` not in path.", // [5]
		"Obtained a NULL path.",                   // [6]
	};

	if (loc == NULL)
	{
		eNum = 4;
		goto out;
	}
	/* if not otherwise set, NULL will indicate that this font was invalid. */
	(*loc) = NULL;

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

	/*------------------------------------------------------------------------*/
	/* First verify that fontconfig is present on the system                  */
	/*------------------------------------------------------------------------*/
	if (system("which fc-match > /dev/null 2>&1"))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Form initial `fc-match` call (malloc for maximum size possibly needed) */
	/*------------------------------------------------------------------------*/
	cmdStr = (char *)malloc(
		(1 + 
			strlen(vMatch) + 
			strlen(fName) + 
			strlen(optStr) +
			strlen(grepStr))*sizeof(char));
	if (cmdStr == NULL)
	{
		eNum = 2;
		goto out;
	}

	sprintf(cmdStr, "%s%s%s", baseMatch, fName, optStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the suggested font filename and verify that it is a TTF file.  */
	/*------------------------------------------------------------------------*/
	if (strchr(buffer, ':'))
	{
		*(strchr(buffer, ':')) = '\0';
	}
	strPtr = strrchr(buffer, '.');
	if (strPtr == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* This would be an invalid file format... skip                       */
		/*--------------------------------------------------------------------*/
		goto out;
	}
	else
	{
		strPtr += 1;
	}
	if (strcmp("ttf", strPtr))
	{
		/* This font is not a TTF type, just return. */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Otherwise this appears to be a TTF-type font.                          */
	/*------------------------------------------------------------------------*/
	sprintf(cmdStr, "%s%s%s%s", vMatch, fName, optStr, grepStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the file path for the font.                                    */
	/*------------------------------------------------------------------------*/
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) { *strPtr = '\0'; }
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) strPtr += 1;

	/*------------------------------------------------------------------------*/
	/* At this point the font name has been isolated, copy it out.            */
	/*------------------------------------------------------------------------*/
	if (strPtr == NULL)
	{
		eNum = 6;
		goto out;
	}
	(*loc) = (char *)malloc((1 + strlen(strPtr))*sizeof(char));
	if ((*loc) == NULL)
	{
		eNum = 2;
		goto out;
	}

	strcpy(*loc, strPtr);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (cmdStr != NULL) free(cmdStr);

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
	int           eNum         = 0; /* which error to report (if any) */
	FILE        * cmdOut       = NULL;
	char        * cmdStr       = NULL;
	char        * strPtr       = NULL;
	char          buffer[2048];
	static char   baseMatch[]  = "fc-match ";
	static char   vMatch[]     = "fc-match -v ";
	static char   optStr[]     = ":italic";
	static char   grepStr[]    = " | grep file:";
	static char   funcName[]   = "NEUIK_GetItalicTTFLocation";
	static char * errMsgs[]    = {"",              // [0] no error
		"Base fontName is NULL/empty.",            // [1]
		"Failed to allocate memory.",              // [2]
		"Unable to locate font.",                  // [3]
		"Pointer to `loc` is NULL.",               // [4]
		"Fontconfig tool `fc-match` not in path.", // [5]
		"Obtained a NULL path.",                   // [6]
	};

	if (loc == NULL)
	{
		eNum = 4;
		goto out;
	}
	/* if not otherwise set, NULL will indicate that this font was invalid. */
	(*loc) = NULL;

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

	/*------------------------------------------------------------------------*/
	/* First verify that fontconfig is present on the system                  */
	/*------------------------------------------------------------------------*/
	if (system("which fc-match > /dev/null 2>&1"))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Form initial `fc-match` call (malloc for maximum size possibly needed) */
	/*------------------------------------------------------------------------*/
	cmdStr = (char *)malloc(
		(1 + 
			strlen(vMatch) + 
			strlen(fName) + 
			strlen(optStr) +
			strlen(grepStr))*sizeof(char));
	if (cmdStr == NULL)
	{
		eNum = 2;
		goto out;
	}

	sprintf(cmdStr, "%s%s%s", baseMatch, fName, optStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the suggested font filename and verify that it is a TTF file.  */
	/*------------------------------------------------------------------------*/
	if (strchr(buffer, ':'))
	{
		*(strchr(buffer, ':')) = '\0';
	}
	strPtr = strrchr(buffer, '.');
	if (strPtr == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* This would be an invalid file format... skip                       */
		/*--------------------------------------------------------------------*/
		goto out;
	}
	else
	{
		strPtr += 1;
	}
	if (strcmp("ttf", strPtr))
	{
		/* This font is not a TTF type, just return. */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Otherwise this appears to be a TTF-type font.                          */
	/*------------------------------------------------------------------------*/
	sprintf(cmdStr, "%s%s%s%s", vMatch, fName, optStr, grepStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the file path for the font.                                    */
	/*------------------------------------------------------------------------*/
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) { *strPtr = '\0'; }
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) strPtr += 1;

	/*------------------------------------------------------------------------*/
	/* At this point the font name has been isolated, copy it out.            */
	/*------------------------------------------------------------------------*/
	if (strPtr == NULL)
	{
		eNum = 6;
		goto out;
	}
	(*loc) = (char *)malloc((1 + strlen(strPtr))*sizeof(char));
	if ((*loc) == NULL)
	{
		eNum = 2;
		goto out;
	}

	strcpy(*loc, strPtr);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (cmdStr != NULL) free(cmdStr);

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
	int           eNum         = 0; /* which error to report (if any) */
	FILE        * cmdOut       = NULL;
	char        * cmdStr       = NULL;
	char        * strPtr       = NULL;
	char          buffer[2048];
	static char   baseMatch[]  = "fc-match ";
	static char   vMatch[]     = "fc-match -v ";
	static char   optStr[]     = ":bold:italic";
	static char   grepStr[]    = " | grep file:";
	static char   funcName[]   = "NEUIK_GetBoldItalicTTFLocation";
	static char * errMsgs[]    = {"",              // [0] no error
		"Base fontName is NULL/empty.",            // [1]
		"Failed to allocate memory.",              // [2]
		"Unable to locate font.",                  // [3]
		"Pointer to `loc` is NULL.",               // [4]
		"Fontconfig tool `fc-match` not in path.", // [5]
		"Obtained a NULL path.",                   // [6]
	};

	if (loc == NULL)
	{
		eNum = 4;
		goto out;
	}
	/* if not otherwise set, NULL will indicate that this font was invalid. */
	(*loc) = NULL;

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

	/*------------------------------------------------------------------------*/
	/* First verify that fontconfig is present on the system                  */
	/*------------------------------------------------------------------------*/
	if (system("which fc-match > /dev/null 2>&1"))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Form initial `fc-match` call (malloc for maximum size possibly needed) */
	/*------------------------------------------------------------------------*/
	cmdStr = (char *)malloc(
		(1 + 
			strlen(vMatch) + 
			strlen(fName) + 
			strlen(optStr) +
			strlen(grepStr))*sizeof(char));
	if (cmdStr == NULL)
	{
		eNum = 2;
		goto out;
	}

	sprintf(cmdStr, "%s%s%s", baseMatch, fName, optStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the suggested font filename and verify that it is a TTF file.  */
	/*------------------------------------------------------------------------*/
	if (strchr(buffer, ':'))
	{
		*(strchr(buffer, ':')) = '\0';
	}
	strPtr = strrchr(buffer, '.');
	if (strPtr == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* This would be an invalid file format... skip                       */
		/*--------------------------------------------------------------------*/
		goto out;
	}
	else
	{
		strPtr += 1;
	}
	if (strcmp("ttf", strPtr))
	{
		/* This font is not a TTF type, just return. */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Otherwise this appears to be a TTF-type font.                          */
	/*------------------------------------------------------------------------*/
	sprintf(cmdStr, "%s%s%s%s", vMatch, fName, optStr, grepStr);

	cmdOut = popen(cmdStr, "r");
	if (fgets(buffer, sizeof(buffer) - 1, cmdOut) == NULL)
	{
		/* Either an error, or no data was read, exit */
		goto out;
	}

	buffer[sizeof(buffer)-1] = '\0';
	pclose(cmdOut);

	/*------------------------------------------------------------------------*/
	/* Isolate the file path for the font.                                    */
	/*------------------------------------------------------------------------*/
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) { *strPtr = '\0'; }
	strPtr = strrchr(buffer, '"');
	if (strPtr != NULL) strPtr += 1;

	/*------------------------------------------------------------------------*/
	/* At this point the font name has been isolated, copy it out.            */
	/*------------------------------------------------------------------------*/
	if (strPtr == NULL)
	{
		eNum = 6;
		goto out;
	}
	(*loc) = (char *)malloc((1 + strlen(strPtr))*sizeof(char));
	if ((*loc) == NULL)
	{
		eNum = 2;
		goto out;
	}

	strcpy(*loc, strPtr);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (cmdStr != NULL) free(cmdStr);

	return eNum;
}

