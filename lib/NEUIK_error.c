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
#include <stdio.h>
#include "NEUIK_error.h"


int          inGUIBacktrace      = 0; /* set to 1 only during a GUI backtrace */
int          guiBacktraceFail    = 0;
int          errorsOmitted       = 0;
const int    maxErrors           = 10;
const char * errorList[11]       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const char * funcNameList[11]    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int          errorDuplicates[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int NEUIK_HasErrors()
{
    int rvErr = 0;
    if (!inGUIBacktrace && errorList[0] != NULL) 
    {
        rvErr = 1;
    }
    else if (inGUIBacktrace && guiBacktraceFail)
    {
        rvErr = 1;
    }

    return rvErr;
}


void NEUIK_ClearErrors()
{
    int ctr;

    for (ctr = 0; ctr <= maxErrors; ctr++)
    {
        errorList[ctr] = 0;     
    }
    errorsOmitted = 0;
}


void NEUIK_RaiseError(
    const char * funcName,
    const char * err)
{
    int ctr;
    int notSet = 1;

    if (inGUIBacktrace) 
    {
        guiBacktraceFail = 1;
        return; /* ignore new errors during a backtrace */
    }

    for (ctr = 0; ctr <= maxErrors; ctr++)
    {
        if (errorList[ctr] == NULL)
        {
            if (ctr > 0)
            {
                if (errorList[ctr-1] == err)
                {
                    /* A repeated error message */
                    errorDuplicates[ctr-1]++;
                    notSet = 0;
                    break;
                }
            }
            errorList[ctr]    = err;
            funcNameList[ctr] = funcName;
            notSet = 0;
            break;
        }
    }
    if (notSet)
    {
        errorsOmitted += 1;
    }
}

