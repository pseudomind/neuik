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
#include <SDL_ttf.h>
#include <stdlib.h>

#include "NEUIK_FontSet.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"

extern float neuik__HighDPI_Scaling;

/*----------------------------------------------------------------------------*/
/* The following is the list of default fonts that NEUIK will check for on    */
/* the system.  Fonts higher up in the list are preferred to those lower in   */
/* the list.                                                                  */
/*----------------------------------------------------------------------------*/
static char * defaultFontBaseNames[] = {
    "verdana",        /* Windows */
    "Verdana",        /* OSX     */
    "Helvetica",      /* OSX     */
    "Tahoma",         /* OSX     */
    "Arial",          /* OSX     */
    "DejaVuSans",     /* Linux   */
    "LiberationSans", /* Linux   */
    "Ubuntu",         /* Linux   */
    NULL,
};

/*----------------------------------------------------------------------------*/
/* The following is the list of default monospace fonts that NEUIK will check */
/* for on the system.  Fonts higher up in the list are preferred to those     */
/* lower in the list.                                                         */
/*----------------------------------------------------------------------------*/
static char * defaultMSFontBaseNames[] = {
    "Hack",           /* Linux   */
    "LiberationMono", /* Linux   */
    NULL,
};

/*******************************************************************************
 *
 *  Name:          NEUIK_NewFontSet
 *
 *  Description:   Allocate memory for a fontset and perform initialization.
 *
 *  Returns:       NULL if ther is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
NEUIK_FontSet * NEUIK_NewFontSet(
    const char * fNameBase,        /* base font family name */
    const char * fNameStd,         /* filename of standard `.ttf` file */
    const char * fNameBold,        /* filename of bold `.ttf` file */
    const char * fNameItalic,      /* filename of italic `.ttf` file */
    const char * fNameBoldItalic)  /* filename of bold-italic `.ttf` file */
{
    int             err = 0;
    NEUIK_FontSet * fs  = NULL;

    fs = (NEUIK_FontSet *)malloc(sizeof(NEUIK_FontSet));
    if (fs == NULL) return fs;

    fs->BaseFontName         = NULL;

    fs->Standard.FontName    = NULL;
    fs->Standard.Available   = 0;
    fs->Standard.MaxSize     = 0;
    fs->Standard.NRef        = NULL;
    fs->Standard.Fonts       = NULL;

    fs->Bold.FontName        = NULL;
    fs->Bold.Available       = 0;
    fs->Bold.MaxSize         = 0;
    fs->Bold.NRef            = NULL;
    fs->Bold.Fonts           = NULL;

    fs->Italic.FontName      = NULL;
    fs->Italic.Available     = 0;
    fs->Italic.MaxSize       = 0;
    fs->Italic.NRef          = NULL;
    fs->Italic.Fonts         = NULL;

    fs->BoldItalic.FontName  = NULL;
    fs->BoldItalic.Available = 0;
    fs->BoldItalic.MaxSize   = 0;
    fs->BoldItalic.NRef      = NULL;
    fs->BoldItalic.Fonts     = NULL;

    String_Duplicate(&(fs->BaseFontName), fNameBase);
    if (fs->BaseFontName == NULL)
    {
        err = 1;
        goto out;
    }
    String_Duplicate(&(fs->Standard.FontName), fNameStd);
    if (fs->Standard.FontName == NULL)
    {
        err = 1;
        goto out;
    }
    String_Duplicate(&(fs->Bold.FontName), fNameBold);
    if (fs->Bold.FontName == NULL)
    {
        err = 1;
        goto out;
    }
    String_Duplicate(&(fs->Italic.FontName), fNameItalic);
    if (fs->Italic.FontName == NULL)
    {
        err = 1;
        goto out;
    }
    String_Duplicate(&(fs->BoldItalic.FontName), fNameBoldItalic);
    if (fs->BoldItalic.FontName == NULL)
    {
        err = 1;
        goto out;
    }
out:
    if (err) 
    {
        /*--------------------------------------------------------------------*/
        /* If for some reason the font set was not able to be loaded, free    */
        /* any partially allocated memory.                                    */
        /*--------------------------------------------------------------------*/
        if (fs != NULL) 
        {
            if (fs->BaseFontName        != NULL) free(fs->BaseFontName);
            if (fs->Standard.FontName   != NULL) free(fs->Standard.FontName);
            if (fs->Bold.FontName       != NULL) free(fs->Bold.FontName);
            if (fs->Italic.FontName     != NULL) free(fs->Italic.FontName);
            if (fs->BoldItalic.FontName != NULL) free(fs->BoldItalic.FontName);
            free(fs);
        }
        fs = NULL;
    }

    return fs;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_GetDefaultFontSet
 *
 *  Description:   Returns a pointer to the first supported system default 
 *                 FontSet.  If it is the first time being called for a 
 *                 particular FontSet, a FontSet will be created.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
NEUIK_FontSet * NEUIK_GetDefaultFontSet(
    char ** baseName)      
{
    int                     len                 = 0;
    int                     ctr                 = 0;
    int                     eNum                = 0; /* which error to report (if any) */
    static int              isInitialized       = 0;
    static char           * dFontName           = NULL;
    static char           * dFontBoldName       = NULL;
    static char           * dFontItalicName     = NULL;
    static char           * dFontBoldItalicName = NULL;
    static NEUIK_FontSet ** fontSets            = NULL; /* array of all font sets */
    NEUIK_FontSet         * fs                  = NULL;
    static char             funcName[]          = "NEUIK_GetDefaultFontSet";
    static char           * errMsgs[] = {"", // [0] no error
        "Failed to allocate memory.",                 // [1]
        "NEUIK_NewFontSet failed.",                   // [2]
        "Failed to reallocate memory.",               // [3]
        "Unable to locate any of the default fonts.", // [4]
        "Failure in GetTTFLocation().",               // [5]
        "Failure in GetBoldTTFLocation().",           // [6]
        "Failure in GetItalicTTFLocation().",         // [7]
        "Failure in GetBoldItalicTTFLocation().",     // [8]
    };


    if (!isInitialized)
    {
        /* Look for the first default font that is supported */
        isInitialized = 1;

        for (ctr = 0 ;; ctr++)
        {
            if (defaultFontBaseNames[ctr] == NULL) break;

            if (NEUIK_GetTTFLocation(
                defaultFontBaseNames[ctr], &dFontName))
            {
                eNum = 5;
                goto out;
            }
            if (NEUIK_GetBoldTTFLocation(
                defaultFontBaseNames[ctr], &dFontBoldName))
            {
                eNum = 6;
                goto out;
            }
            if (NEUIK_GetItalicTTFLocation(
                defaultFontBaseNames[ctr], &dFontItalicName))
            {
                eNum = 7;
                goto out;
            }
            if (NEUIK_GetBoldItalicTTFLocation(
                defaultFontBaseNames[ctr], &dFontBoldItalicName))
            {
                eNum = 8;
                goto out;
            }

            if (dFontName != NULL && dFontBoldName != NULL && 
                dFontItalicName != NULL && dFontBoldItalicName != NULL) 
            {
                break;
            }
            else 
            {
                if (dFontName != NULL)
                {
                    free(dFontName);
                    dFontName = NULL;
                }
                if (dFontBoldName != NULL)
                {
                    free(dFontBoldName);
                    dFontBoldName = NULL;
                }
                if (dFontItalicName != NULL)
                {
                    free(dFontItalicName);
                    dFontItalicName = NULL;
                }
                if (dFontBoldItalicName != NULL)
                {
                    free(dFontBoldItalicName);
                    dFontBoldItalicName = NULL;
                }
            }
        }
    }
    if (dFontName == NULL)
    {
        /* None of the default fonts could be located */
        eNum = 4;
        goto out;
    }

    if (fontSets == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This is the first FontSet to be added, allocate initial memory     */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        fontSets = (NEUIK_FontSet **)malloc(2*sizeof(NEUIK_FontSet *));
        if (fontSets == NULL)
        {
            eNum = 1;
            goto out;
        }

        /* set the font name */
        fs = NEUIK_NewFontSet(defaultFontBaseNames[ctr],
            dFontName, dFontBoldName, dFontItalicName, dFontBoldItalicName);
        if (fs == NULL)
        {
            eNum = 2;
            goto out;
        }
        fontSets[0] = fs;
        fontSets[1] = NULL;
    }
    else
    {
        fs = fontSets[0];
        /*--------------------------------------------------------------------*/
        /* This is subsequent menu item, reallocate memory                    */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length */
        for (ctr = 0;;ctr++)
        {
            if (fontSets[ctr] == NULL)
            {
                len = 2 + ctr;
                break;
            }
        }

        fontSets = (NEUIK_FontSet **)realloc(fontSets, len*sizeof(NEUIK_FontSet *));
        if (fontSets == NULL)
        {
            eNum = 3;
            goto out;
        }

        /* set the font name */
        fs = NEUIK_NewFontSet(defaultFontBaseNames[ctr],
            dFontName, dFontBoldName, dFontItalicName, dFontBoldItalicName);
        if (fs == NULL)
        {
            eNum = 2;
            goto out;
        }
        fontSets[ctr+1] = NULL;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }
    else
    {
        *baseName = dFontName;
    }

    return fs;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_GetDefaultMSFontSet
 *
 *  Description:   Returns a pointer to the first supported system default 
 *                 Monospaced FontSet.  If it is the first time being called for
 *                 a particular FontSet, a FontSet will be created.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
NEUIK_FontSet * NEUIK_GetDefaultMSFontSet(
    char ** baseName)      
{
    int                     len                 = 0;
    int                     ctr                 = 0;
    int                     eNum                = 0; /* which error to report (if any) */
    static int              isInitialized       = 0;
    static char           * dFontName           = NULL;
    static char           * dFontBoldName       = NULL;
    static char           * dFontItalicName     = NULL;
    static char           * dFontBoldItalicName = NULL;
    static NEUIK_FontSet ** fontSets            = NULL; /* array of all font sets */
    NEUIK_FontSet         * fs                  = NULL;
    static char             funcName[]          = "NEUIK_GetDefaultMSFontSet";
    static char           * errMsgs[] = {"", // [0] no error
        "Failed to allocate memory.",                 // [1]
        "NEUIK_NewFontSet failed.",                   // [2]
        "Failed to reallocate memory.",               // [3]
        "Unable to locate any of the default fonts.", // [4]
        "Failure in GetTTFLocation().",               // [5]
        "Failure in GetBoldTTFLocation().",           // [6]
        "Failure in GetItalicTTFLocation().",         // [7]
        "Failure in GetBoldItalicTTFLocation().",     // [8]
    };


    if (!isInitialized)
    {
        /* Look for the first default font that is supported */
        isInitialized = 1;

        for (ctr = 0 ;; ctr++)
        {
            if (defaultMSFontBaseNames[ctr] == NULL) break;

            if (NEUIK_GetTTFLocation(
                defaultMSFontBaseNames[ctr], &dFontName))
            {
                eNum = 5;
                goto out;
            }
            if (NEUIK_GetBoldTTFLocation(
                defaultMSFontBaseNames[ctr], &dFontBoldName))
            {
                eNum = 6;
                goto out;
            }
            if (NEUIK_GetItalicTTFLocation(
                defaultMSFontBaseNames[ctr], &dFontItalicName))
            {
                eNum = 7;
                goto out;
            }
            if (NEUIK_GetBoldItalicTTFLocation(
                defaultMSFontBaseNames[ctr], &dFontBoldItalicName))
            {
                eNum = 8;
                goto out;
            }

            if (dFontName != NULL && dFontBoldName != NULL && 
                dFontItalicName != NULL && dFontBoldItalicName != NULL) 
            {
                break;
            }
            else 
            {
                if (dFontName != NULL)
                {
                    free(dFontName);
                    dFontName = NULL;
                }
                if (dFontBoldName != NULL)
                {
                    free(dFontBoldName);
                    dFontBoldName = NULL;
                }
                if (dFontItalicName != NULL)
                {
                    free(dFontItalicName);
                    dFontItalicName = NULL;
                }
                if (dFontBoldItalicName != NULL)
                {
                    free(dFontBoldItalicName);
                    dFontBoldItalicName = NULL;
                }
            }
        }
    }
    if (dFontName == NULL)
    {
        /* None of the default fonts could be located */
        eNum = 4;
        goto out;
    }

    if (fontSets == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This is the first FontSet to be added, allocate initial memory     */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        fontSets = (NEUIK_FontSet **)malloc(2*sizeof(NEUIK_FontSet *));
        if (fontSets == NULL)
        {
            eNum = 1;
            goto out;
        }

        /* set the font name */
        fs = NEUIK_NewFontSet(defaultMSFontBaseNames[ctr],
            dFontName, dFontBoldName, dFontItalicName, dFontBoldItalicName);
        if (fs == NULL)
        {
            eNum = 2;
            goto out;
        }
        fontSets[0] = fs;
        fontSets[1] = NULL;
    }
    else
    {
        fs = fontSets[0];
        /*--------------------------------------------------------------------*/
        /* This is subsequent menu item, reallocate memory                    */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length */
        for (ctr = 0;;ctr++)
        {
            if (fontSets[ctr] == NULL)
            {
                len = 2 + ctr;
                break;
            }
        }

        fontSets = (NEUIK_FontSet **)realloc(fontSets, 
            len*sizeof(NEUIK_FontSet *));
        if (fontSets == NULL)
        {
            eNum = 3;
            goto out;
        }

        /* set the font name */
        fs = NEUIK_NewFontSet(defaultMSFontBaseNames[ctr],
            dFontName, dFontBoldName, dFontItalicName, dFontBoldItalicName);
        if (fs == NULL)
        {
            eNum = 2;
            goto out;
        }
        fontSets[ctr+1] = NULL;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }
    else
    {
        *baseName = dFontName;
    }

    return fs;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_FontSet_GetFont
 *
 *  Description:   Returns a pointer to a FontSet.  If it is the first time 
 *                 being called for a particular FontSet, a FontSet will be 
 *                 created.
 *
 *  Returns:       NULL if ther is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
neuik_ptrTo_TTF_Font NEUIK_FontSet_GetFont(
    NEUIK_FontSet * fs,        /* the fontset to get the font from */
    unsigned int    fSize,     /* the fontsize of the desired font */
    int             useBold,   /* (bool) whether font should be bold */
    int             useItalic) /* (bool) whether font should be italic */
{
    unsigned int        ctr        = 0;
    unsigned int        fSizeSc    = 0; /* HighDPI scaled font size */
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_FontFileSet * ffs        = NULL;
    static char         funcName[] = "NEUIK_FontSet_GetFont";
    static char       * errMsgs[]  = {"", // [0] no error
        "FontSet pointer is NULL.",              // [1]
        "An invalid fontSize of zero supplied.", // [2]
        "Failed to allocate memory.",            // [3]
        "Func `TTF_OpenFont` failed.",           // [4]
        "Failed to reallocate memory.",          // [5]
        "Desired font style is unavailable.",    // [6]
    };
    TTF_Font * rvFont = NULL;

    if (fs == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (fSize == 0)
    {
        eNum = 2;
        goto out;
    }
    fSizeSc = (unsigned int)((float)(fSize)*neuik__HighDPI_Scaling);

    /*------------------------------------------------------------------------*/
    /* Set a pointer to the correct font style (std, bold, italic, bold-ital) */
    /*------------------------------------------------------------------------*/
    if (useBold && useItalic)
    {
        ffs = &(fs->BoldItalic);
    }
    else if (useBold)
    {
        ffs = &(fs->Bold);
    }
    else if (useItalic)
    {
        ffs = &(fs->Italic);
    }
    else
    {
        ffs = &(fs->Standard);
    }

    if (ffs == NULL)
    {
        eNum = 6;
        goto out;
    }

    if (ffs->MaxSize == 0)
    {
        /*--------------------------------------------------------------------*/
        /* This is the first time GetFont is called on this FontSet.          */
        /* Allocate and set initial array memory.                             */
        /*--------------------------------------------------------------------*/
        ffs->NRef = (unsigned int *)malloc((fSizeSc+1)*sizeof(unsigned int));
        if (ffs->NRef == NULL)
        {
            eNum = 3;
            goto out;
        }

        ffs->Fonts = (neuik_ptrTo_TTF_Font *)malloc(
            (fSizeSc+1)*sizeof(TTF_Font*));
        if (ffs->Fonts == NULL)
        {
            eNum = 3;
            goto out;
        }

        /* Zero/NULL out the initial values of each array */
        for (ctr=0; ctr<fSizeSc; ctr++)
        {
            ffs->NRef[ctr]  = 0;
            ffs->Fonts[ctr] = NULL;
        }

        /* load the TTF_Font into the appropriate array index */
        ffs->Fonts[fSizeSc] = TTF_OpenFont(ffs->FontName, fSizeSc);
        if (ffs->Fonts[fSizeSc] == NULL)
        {
            eNum = 4;
            goto out;
        }
        ffs->MaxSize = fSizeSc;
        rvFont = ffs->Fonts[fSizeSc];
    }
    else if (fSizeSc <= ffs->MaxSize)
    {
        /*--------------------------------------------------------------------*/
        /* A subsequent call to GetFont. Check if the font has already been   */
        /* loaded and if so just return the pointer.  Otherwise, load the     */
        /* into the specified array index.                                    */
        /*--------------------------------------------------------------------*/
        if (ffs->Fonts[fSizeSc] == NULL)
        {
            ffs->Fonts[fSizeSc] = TTF_OpenFont(ffs->FontName, fSizeSc);
            if (ffs->Fonts[fSizeSc] == NULL)
            {
                eNum = 4;
                goto out;
            }
        }
        rvFont = ffs->Fonts[fSizeSc];
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* A subsequent call to GetFont. This font size exceeds that of any   */
        /* previously loaded font variants.  The array will need to be        */
        /* reallocated and only the new array values must be initialized.     */
        /*--------------------------------------------------------------------*/
        ffs->NRef = (unsigned int *)realloc(
            ffs->NRef, (fSizeSc+1)*sizeof(unsigned int));
        if (ffs->NRef == NULL)
        {
            eNum = 5;
            goto out;
        }

        ffs->Fonts = (neuik_ptrTo_TTF_Font *)realloc(
            ffs->Fonts, (fSizeSc+1)*sizeof(TTF_Font*));
        if (ffs->Fonts == NULL)
        {
            eNum = 5;
            goto out;
        }

        /* Zero/NULL out the addtional values of each array */
        for (ctr=ffs->MaxSize+1; ctr<fSizeSc; ctr++)
        {
            ffs->NRef[ctr]  = 0;
            ffs->Fonts[ctr] = NULL;
        }

        /* load the TTF_Font into the appropriate array index */
        ffs->Fonts[fSizeSc] = TTF_OpenFont(ffs->FontName, fSizeSc);
        if (ffs->Fonts[fSizeSc] == NULL)
        {
            eNum = 4;
            goto out;
        }
        ffs->MaxSize = fSizeSc;
        rvFont = ffs->Fonts[fSizeSc];
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return rvFont;
}

