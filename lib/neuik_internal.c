/*******************************************************************************
 * Copyright (c) 2014-2021, Michael Leimon <mike@leimon.net>
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
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#include "neuik_internal.h"
#include "NEUIK_error.h"

neuik_Set   ** neuik_AllSets    = NULL;
neuik_Class ** neuik_AllClasses = NULL;

neuik_FatalError neuik_Fatal = NEUIK_FATALERROR_NO_ERROR;

#ifndef WIN32
    static sigjmp_buf sigjmp_buffer;
#endif /* WIN32*/

/*******************************************************************************
 *
 *  Name:          neuik_HasFatalError
 *
 *  Description:   Check for the presense of any Fatal Errors.
 *
 *                 A maximum of one fatal error may be tracked. Fatal errors are
 *                 different from standard errors in that they may result from a
 *                 captured SEGFAULT signal, or may result from cases where
 *                 endless function recursion is suspected. Since errors of this
 *                 nature are difficult to trap, Fatal errors are generally
 *                 created an propagate up to higher level functions before
 *                 being captured in as a standard high-level Error. This allows
 *                 for these low-level errors to be reported in a manner that is
 *                 easier for the programmer to understand and fix.
 *
 *  Returns:       1 if there is a fatal error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_HasFatalError()
{
    if (neuik_Fatal != NEUIK_FATALERROR_NO_ERROR)
    {
        return 1;
    }
    return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClassSet
 *
 *  Description:   Register a new class set with the NEUIK library.
 *
 *                 A pointer to the new class set is stored within the `newSet`
 *                 argument.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_RegisterClassSet(
    const char  * setName,        /* (in)  Name of the Class Set */
    const char  * setDescription, /* (in)  Description of the class Set */
    neuik_Set  ** newSet)         /* (out) The unique setID for this class set */
{
    int           ctr;
    size_t        sLen;
    int           eNum       = 0;
    neuik_Set   * thisSet    = NULL;
    static char   funcName[] = "neuik_RegisterClassSet";
    static char * errMsgs[]  = {"", // [0] no error
        "Failed to allocate memory.",                    // [1]
        "Argument `setName` is NULL.",                   // [2]
        "Argument `setName` is invalid (blank).",        // [3]
        "Argument `setDescription` is NULL.",            // [4]
        "Argument `setDescription` is invalid (blank).", // [5]
        "Failed to reallocate memory.",                  // [6]
        "Argument `newSet` is NULL.",                    // [7]
    };

    /*------------------------------------------------------------------------*/
    /* Check for simple problems before continuing.                           */
    /*------------------------------------------------------------------------*/
    if (newSet == NULL)
    {
        eNum = 7;
        goto out;
    }
    (*newSet) = NULL;

    if (setName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (strlen(setName) == 0)
    {
        eNum = 3;
        goto out;
    }
    if (setDescription == NULL)
    {
        eNum = 4;
        goto out;
    }
    else if (strlen(setDescription) == 0)
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate/Reallocate the `AllSets` list.                                */
    /*------------------------------------------------------------------------*/
    if (neuik_AllSets == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This is the first set to be allocated.                             */
        /*--------------------------------------------------------------------*/
        neuik_AllSets = (neuik_Set**) malloc(2*sizeof(neuik_Set*));
        if (neuik_AllSets == NULL)
        {
            eNum = 1;
            goto out;
        }
        neuik_AllSets[1] = NULL; /* nullPtr terminated list */

        neuik_AllSets[0] = (neuik_Set*) malloc(sizeof(neuik_Set));
        if (neuik_AllSets[0] == NULL)
        {
            eNum = 1;
            goto out;
        }
        /* Set successfully allocated */

        ctr = 0;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* A subsequent set, reallocate the array.                            */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length of the array */
        for (ctr = 0;; ctr++)
        {
            if (neuik_AllSets[ctr] == NULL)
            {
                break;
            }
        }

        neuik_AllSets = (neuik_Set**) realloc(neuik_AllSets, 
            (2+ctr)*sizeof(neuik_Set*));
        if (neuik_AllSets == NULL)
        {
            eNum = 6;
            goto out;
        }
        neuik_AllSets[ctr + 1] = NULL; /* nullPtr terminated list */

        neuik_AllSets[ctr] = (neuik_Set*) malloc(sizeof(neuik_Set));
        if (neuik_AllSets[ctr] == NULL)
        {
            eNum = 1;
            goto out;
        }
        /* Set successfully allocated */
    }

    /*------------------------------------------------------------------------*/
    /* Set the values for this new set.                                       */
    /*------------------------------------------------------------------------*/
    thisSet = neuik_AllSets[ctr];

    thisSet->SetID = ctr;
    /* Copy the set name */
    sLen = 1 + strlen(setName);
    thisSet->SetName = (char*) malloc(sLen*(sizeof(char)));
    if (thisSet->SetName == NULL)
    {
        eNum = 1;
        goto out;
    }
    strcpy(thisSet->SetName, setName);

    /* Copy the set description */
    sLen = 1 + strlen(setDescription);
    thisSet->SetDescription = (char*) malloc(sLen*(sizeof(char)));
    if (thisSet->SetDescription == NULL)
    {
        eNum = 1;
        goto out;
    }
    strcpy(thisSet->SetDescription, setDescription);
    (*newSet) = thisSet;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass
 *
 *  Description:   Register a new class with the NEUIK library.
 *
 *                 A pointer to the new class is stored within the `newClass`
 *                 argument.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_RegisterClass(
    const char             * className,        /* (in)  Name of the Class Set */
    const char             * classDescription, /* (in)  Description of the class Set */
    neuik_Set              * classSet,         /* (in)  The class set which this belongs to */
    neuik_Class            * superClass,       /* (in)  Superclass; NULL if there is none */
    neuik_Class_BaseFuncs  * baseFuncs,        /* (in)  Base function table for class support */
    void                   * classFuncs,       /* (in)  Class-specific function table */
    neuik_Class           ** newClass)         /* (out) Store a pointer to the new class */
{
    int           ctr;
    size_t        sLen;
    int           eNum       = 0;
    neuik_Class * thisClass  = NULL;
    static char   funcName[] = "neuik_RegisterClass";
    static char * errMsgs[]  = {"", // [0] no error
        "Failed to allocate memory.",                      // [1]
        "Argument `className` is NULL.",                   // [2]
        "Argument `className` is invalid (blank).",        // [3]
        "Argument `classDescription` is NULL.",            // [4]
        "Argument `classDescription` is invalid (blank).", // [5]
        "Failed to reallocate memory.",                    // [6]
        "Output Argument `newClass` is NULL.",             // [7]
        "Argument `classSet` is NULL.",                    // [8]
    };

    /*------------------------------------------------------------------------*/
    /* Check for simple problems before continuing.                           */
    /*------------------------------------------------------------------------*/
    if (newClass == NULL)
    {
        eNum = 7;
        goto out;
    }
    (*newClass) = NULL;

    if (classSet == NULL)
    {
        eNum = 8;
        goto out;
    }
    (*newClass) = NULL;

    if (className == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (strlen(className) == 0)
    {
        eNum = 3;
        goto out;
    }
    if (classDescription == NULL)
    {
        eNum = 4;
        goto out;
    }
    else if (strlen(classDescription) == 0)
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate/Reallocate the `AllClasses` list.                             */
    /*------------------------------------------------------------------------*/
    if (neuik_AllClasses == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This is the first set to be allocated.                             */
        /*--------------------------------------------------------------------*/
        neuik_AllClasses = (neuik_Class**) malloc(2*sizeof(neuik_Class*));
        if (neuik_AllClasses == NULL)
        {
            eNum = 1;
            goto out;
        }
        neuik_AllClasses[1] = NULL; /* nullPtr terminated list */

        neuik_AllClasses[0] = (neuik_Class*) malloc(sizeof(neuik_Class));
        if (neuik_AllClasses[0] == NULL)
        {
            eNum = 1;
            goto out;
        }
        /* Set successfully allocated */

        ctr = 0;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* A subsequent set, reallocate the array.                            */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length of the array */
        for (ctr = 0;; ctr++)
        {
            if (neuik_AllClasses[ctr] == NULL)
            {
                break;
            }
        }

        neuik_AllClasses = (neuik_Class**) realloc(neuik_AllClasses, 
            (2+ctr)*sizeof(neuik_Class*));
        if (neuik_AllClasses == NULL)
        {
            eNum = 6;
            goto out;
        }
        neuik_AllClasses[ctr + 1] = NULL; /* nullPtr terminated list */

        neuik_AllClasses[ctr] = (neuik_Class*) malloc(sizeof(neuik_Class));
        if (neuik_AllClasses[ctr] == NULL)
        {
            eNum = 1;
            goto out;
        }
        /* Set successfully allocated */
    }

    /*------------------------------------------------------------------------*/
    /* Set the values for this new class.                                     */
    /*------------------------------------------------------------------------*/
    thisClass = neuik_AllClasses[ctr];

    thisClass->ClassID = ctr;
    /* Copy the class name */
    sLen = 1 + strlen(className);
    thisClass->ClassName = (char*) malloc(sLen*(sizeof(char)));
    if (thisClass->ClassName == NULL)
    {
        eNum = 1;
        goto out;
    }
    strcpy(thisClass->ClassName, className);

    /* Copy the class description */
    sLen = 1 + strlen(classDescription);
    thisClass->ClassDescription = (char*) malloc(sLen*(sizeof(char)));
    if (thisClass->ClassDescription == NULL)
    {
        eNum = 1;
        goto out;
    }
    strcpy(thisClass->ClassDescription, classDescription);

    /* Set the class set pointer */
    thisClass->Set        = classSet;
    thisClass->SuperClass = superClass;
    thisClass->baseFuncs  = baseFuncs;

    (*newClass) = thisClass;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New
 *
 *  Description:   Call the class-specific Object_New function.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_Object_New(
    neuik_Class  * objClass,
    void        ** objPtr)
{
    int            eNum              = 0;
    int         (* newFunc)(void **) = NULL;
    static char    funcName[]        = "neuik_Object_New";
    static char  * errMsgs[]         = {"", // [0] no error
        "Output Argument `objPtr` is NULL.",               // [1]
        "Argument `objClass` is NULL.",                    // [2]
        "Argument `objClass` does not implement `New().`", // [3]
        "Function implemtnation for `New()` failed.",      // [4]
    };

    if (objPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (objClass == NULL)
    {
        eNum = 2;
        goto out;
    }

    newFunc = objClass->baseFuncs->New;
    if (newFunc == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Object_New() impelmentation appears to be supplied; use it.            */
    /*------------------------------------------------------------------------*/
    if ((*newFunc)(objPtr))
    {
        eNum = 4;
        goto out;
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Free
 *
 *  Description:   Call the class-specific Object_Free function.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_Object_Free(
    void * objPtr)
{
    int                  eNum              = 0;
    int               (* freeFunc)(void *) = NULL;
    neuik_Class        * objClass          = NULL;
    neuik_Object_Base  * objBase           = NULL;
    static char          funcName[]        = "neuik_Object_Free";
    static char        * errMsgs[]         = {"", // [0] no error
        "Argument `objPtr` is not a valid NEUIK Object.", // [1]
        "Argument `objClass` is NULL.",                   // [2]
        "Argument `objPtr` does not implement `Free().`", // [3]
        "Function implementation for `Free()` failed.",   // [4]
    };

    if (!neuik_Object_IsNEUIKObject(objPtr))
    {
        eNum = 1;
        goto out;
    }

    objBase = (neuik_Object_Base*)(objPtr);
    objClass = objBase->object.nClass;
    if (objClass == NULL)
    {
        eNum = 2;
        goto out;
    }

    freeFunc = objClass->baseFuncs->Free;
    if (freeFunc == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Object_Free() implementation appears to be supplied; use it.           */
    /*------------------------------------------------------------------------*/
    if ((*freeFunc)(objPtr))
    {
        eNum = 4;
        goto out;
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Object_Free
 *
 *  Description:   Call the class-specific Object_Free function.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int NEUIK_Object_Free(
    void ** objPtr)
{
    return neuik_Object_Free(objPtr);
}


#ifndef WIN32
    /*******************************************************************************
     *
     *  Name:          neuik_Object_IsNEUIKObject_capture_segv
     *
     *  Description:   Captures SIGSEGV signals and returns the program flow back
     *                 to the original function.
     *
    ******************************************************************************/
    void neuik_Object_IsNEUIKObject_capture_segv(
        int sig_num)
    {
        int unused = 0;

        if (sig_num) { unused++; }
        if (unused) { unused++; }

        siglongjmp(sigjmp_buffer, 1);
    }
#endif /* WIN32 */


/*******************************************************************************
 *
 *  Name:          neuik_Object_IsNEUIKObject
 *
 *  Description:   Check a (void*) to see if it is a valid NEUIK object.
 *
 *  Returns:       1 if the pointer is a valid object, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_IsNEUIKObject(
    const void * objPtr)
{
    int                 eNum       = 0;
    int                 isNObj     = 1;
    neuik_Object_Base * objBase    = NULL;
    static char         funcName[] = "neuik_Object_IsNEUIKObject";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `objPtr` is NULL.",                   // [1]
        "Object doesn't appear to be an NEUIK object.", // [2]
    };

    if (objPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    objBase = (neuik_Object_Base*)(objPtr);

#ifndef WIN32
    if (sigsetjmp(sigjmp_buffer, 1))
    {
        /*--------------------------------------------------------------------*/
        /* If code execution returns here, it is because of a siglongjmp and  */
        /* it means that the code has reached an error state.                 */
        /*--------------------------------------------------------------------*/
        neuik_Fatal = NEUIK_FATALERROR_SIGSEGV_CAPTURED;
        goto out2;
    }

    signal(SIGSEGV, neuik_Object_IsNEUIKObject_capture_segv);
#endif /* WIN32 */
    if ( (objBase->object).mustBe_1337  != 1337 ||
         (objBase->object).mustBe_90210 != 90210)
    {
        eNum = 2;
        goto out;
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        isNObj = 0;
    }
#ifndef WIN32
    out2:
    signal(SIGSEGV, NULL);
#endif /* WIN32 */
    return isNObj;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_IsNEUIKObject_NoError
 *
 *  Description:   Check a (void*) to see if it is a valid NEUIK object.
 *
 *                 This function variant does not generate errors when/if a 
 *                 check is failed.
 *
 *  Returns:       1 if the pointer is a valid object, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_IsNEUIKObject_NoError(
    const void * objPtr)
{
    neuik_Object_Base * objBase = NULL;

    if (objPtr == NULL) return 0;
    objBase = (neuik_Object_Base*)(objPtr);


    if ( (objBase->object).mustBe_1337  != 1337 ||
         (objBase->object).mustBe_90210 != 90210)
    {
        return 0;
    }

    return 1;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_ImplementsClass
 *
 *  Description:   Check a (void*) to see if its Class or one of its 
 *                 superclasses implements the specified class.
 *
 *  Returns:       1 if the pointer implements specified class, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_ImplementsClass(
    const void  * objPtr,
    neuik_Class * nClass)
{
    int                 eNum       = 2;
    int                 impClass   = 0;
    neuik_Class       * sClass     = NULL; // superClass of a class
    neuik_Object_Base * objBase    = NULL;
    static char         funcName[] = "neuik_Object_ImplementsClass";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `object` is not a valid NEUIK Object.",       // [1]
        "NEUIK Object does not implement the specified class.", // [2]
    };

    if (!neuik_Object_IsNEUIKObject(objPtr))
    {
        if (neuik_HasFatalError())
        {
            goto out2;
        }

        eNum = 1;
        goto out;
    }

    objBase = (neuik_Object_Base*)(objPtr);
    if ((objBase->object).nClass == nClass)
    {
        impClass = 1;
        eNum     = 0;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check if the object is of a class that has at least one SuperClass     */
    /*------------------------------------------------------------------------*/
    sClass = ((objBase->object).nClass)->SuperClass; // get the SuperClass of this objects class
    if (sClass != NULL)
    {
        for (;;)
        {
            /*----------------------------------------------------------------*/
            /* Iterate over the nested superclasses until NULL is hit         */
            /*----------------------------------------------------------------*/
            if (sClass == nClass)
            {
                impClass = 1;
                eNum     = 0;
                break;
            }

            /*----------------------------------------------------------------*/
            /* change superClass pointer to point at the Super of this Class  */
            /*----------------------------------------------------------------*/
            sClass = sClass->SuperClass;
            if (!sClass) break; // sClass = NULL means class has no SuperClass
        }
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }
out2:
    return impClass;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_ImplementsClass_NoEror
 *
 *  Description:   Check a (void*) to see if its Class or one of its 
 *                 superclasses implements the specified class.
 *
 *                 This function variant does not generate errors when/if a 
 *                 check is failed.
 *
 *  Returns:       1 if the pointer implements specified class, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_ImplementsClass_NoEror(
    const void  * objPtr,
    neuik_Class * nClass)
{
    int                 impClass   = 0;
    neuik_Class       * sClass     = NULL; // superClass of a class
    neuik_Object_Base * objBase    = NULL;

    if (!neuik_Object_IsNEUIKObject_NoError(objPtr))
    {
        impClass = 0;
        goto out;
    }

    objBase = (neuik_Object_Base*)(objPtr);
    if ((objBase->object).nClass == nClass)
    {
        impClass = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check if the object is of a class that has at least one SuperClass     */
    /*------------------------------------------------------------------------*/
    sClass = ((objBase->object).nClass)->SuperClass; // get the SuperClass of this objects class
    if (sClass != NULL)
    {
        for (;;)
        {
            /*----------------------------------------------------------------*/
            /* Iterate over the nested superclasses until NULL is hit         */
            /*----------------------------------------------------------------*/
            if (sClass == nClass) return 1;
            {
                impClass = 1;
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* change superClass pointer to point at the Super of this Class  */
            /*----------------------------------------------------------------*/
            sClass = sClass->SuperClass;
            if (!sClass) break; // sClass = NULL means class has no SuperClass
        }
    }
out:

    return impClass;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_IsClass_NoErr
 *
 *  Description:   Check a (void*) to see if it is a valid NEUIK object of the
 *                 specified class.
 *
 *  Returns:       1 if the pointer is of the specified class, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_IsClass(
    const void  * objPtr,
    neuik_Class * nClass)
{
    int                 eNum    = 0;
    int                 isClass = 1;
    neuik_Object      * nObj    = NULL;
    neuik_Object_Base * objBase = NULL;
    static char         funcName[] = "neuik_Object_IsClass";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `objPtr` is not a valid NEUIK Object.", // [1]
        "NEUIK Object is not the specified Class.",       // [2]
    };

    if (!neuik_Object_IsNEUIKObject(objPtr))
    {
        eNum = 1;
        goto out;
    }

    objBase = (neuik_Object_Base*)(objPtr);
    nObj = &(objBase->object);

    if (nObj->nClass != nClass)
    {
        eNum = 2;
        goto out;
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        isClass = 0;
    }

    return isClass;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_IsClass_NoErr
 *
 *  Description:   Check a (void*) to see if it is a valid NEUIK object of the
 *                 specified class.
 *
 *                 This function variant does not generate errors when/if a 
 *                 check is failed.
 *
 *  Returns:       1 if the pointer is of the specified class, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_IsClass_NoErr(
    const void  * objPtr,
    neuik_Class * nClass)
{
    neuik_Object      * nObj    = NULL;
    neuik_Object_Base * objBase = NULL;

    if (!neuik_Object_IsNEUIKObject_NoError(objPtr)) return 0;

    objBase = (neuik_Object_Base*)(objPtr);
    nObj = &(objBase->object);

    if (nObj->nClass != nClass) return 0;

    return 1;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_GetClassObject
 *
 *  Description:   Get a pointer to the desired class object of an object.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_Object_GetClassObject(
    void         * objPtr,
    neuik_Class  * nClass,
    void        ** classObject)
{
    int                 eNum       = 0;
    neuik_Object_Base * objBase    = NULL;
    static char         funcName[] = "neuik_Object_GetClassObject";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `objPtr` does not implement specified class.",     // [1]
        "Output Argument `classObject` is NULL.",                    // [2]
        "Argument `objPtr` is missing the specified class object.",  // [3]
    };

    if (!neuik_Object_ImplementsClass(objPtr, nClass))
    {
        eNum = 1;
        goto out;
    }
    if (classObject == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check the toplevel object to see if that is of correct class type      */
    /*------------------------------------------------------------------------*/
    objBase = (neuik_Object_Base*)(objPtr);
    if ((objBase->object).nClass == nClass)
    {
        (*classObject) = objPtr;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Iterate through the object SuperClasses till the correct Class Object  */
    /* is found.                                                              */
    /*------------------------------------------------------------------------*/
    for (;;)
    {
        objBase = (neuik_Object_Base*)(objBase->object.superClassObj);
        if (objBase == NULL)
        {
            eNum = 3;
            goto out;
        }
        else if ((objBase->object).nClass == nClass)
        {
            (*classObject) = objBase;
            goto out;
        }
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_GetClassObject_NoError
 *
 *  Description:   Get a pointer to the desired class object of an object.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_Object_GetClassObject_NoError(
    void         * objPtr,
    neuik_Class  * nClass,
    void        ** classObject)
{
    int                 eNum       = 0;
    neuik_Object_Base * objBase    = NULL;

    if (!neuik_Object_ImplementsClass_NoEror(objPtr, nClass))
    {
        eNum = 1;
        goto out;
    }
    if (classObject == NULL)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check the toplevel object to see if that is of correct class type      */
    /*------------------------------------------------------------------------*/
    objBase = (neuik_Object_Base*)(objPtr);
    if ((objBase->object).nClass == nClass)
    {
        (*classObject) = objPtr;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Iterate through the object SuperClasses till the correct Class Object  */
    /* is found.                                                              */
    /*------------------------------------------------------------------------*/
    for (;;)
    {
        objBase = (neuik_Object_Base*)(objBase->object.superClassObj);
        if (objBase == NULL)
        {
            eNum = 1;
            goto out;
        }
        else if ((objBase->object).nClass == nClass)
        {
            (*classObject) = objBase;
            goto out;
        }
    }
out:
    if (eNum != 0)
    {
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_GetObjectBaseOfClass
 *
 *  Description:   Set the ObjectBase values for this object.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_GetObjectBaseOfClass(
    neuik_Set    * objSet,
    neuik_Class  * objClass,
    void         * superClassObj,
    neuik_Object * object)
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_GetObjectBaseOfClass";
    static char  * errMsgs[]  = {"", // [0] no error
        "Argument `object` (type: neuik_Object*) is NULL.",  // [1]
        "Argument `objClass` (type: neuik_Class*) is NULL.", // [2]
        "Argument `objSet` (type: neuik_Set*) is NULL.",     // [3]
    };

    if (object == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (objClass == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (objSet == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Input parameters appear valid; set values for the ObjectBase           */
    /*------------------------------------------------------------------------*/
    object->mustBe_1337   = 1337;
    object->mustBe_90210  = 90210;
    object->nSet          = objSet;
    object->nClass        = objClass;
    object->superClassObj = superClassObj;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Show
 *
 *  Description:   Print out detailed information about an object.
 *
 ******************************************************************************/
void neuik_Object_Show(
    void * objPtr,
    int    verbosity)
{
    void              * superClassObj = NULL;
    neuik_Object_Base * objBase       = NULL;
    neuik_Object_Base * sObjBase      = NULL;

    if (!neuik_Object_IsNEUIKObject_NoError(objPtr))
    {
        printf("Pointer is not to a valid NEUIK_Object.\n");
        return;
    }

    /*------------------------------------------------------------------------*/
    /* Check the toplevel object to see if that is of correct class type      */
    /*------------------------------------------------------------------------*/
    objBase = (neuik_Object_Base*)(objPtr);
    printf("%s [%s]\n", 
        objBase->object.nClass->ClassName,
        objBase->object.nSet->SetName);

    if (verbosity == 0) return;

    superClassObj = objBase->object.superClassObj;
    for (;;)
    {
        if (!neuik_Object_IsNEUIKObject_NoError(superClassObj)) break;

        sObjBase = (neuik_Object_Base*)(superClassObj);
        printf("    subclass of %s [%s]\n", 
            sObjBase->object.nClass->ClassName,
            sObjBase->object.nSet->SetName);
        superClassObj = sObjBase->object.superClassObj;
    }
    if (verbosity == 1) return;
}



/*******************************************************************************
 *
 *  Name:          neuik_VirtualFunc_RegisterImplementation
 *
 *  Description:   Register a class:funcImplementation pair with a vFunc set.
 *
 *  Returns:       1 if there is an error, 0 otherwise. 
 *
 ******************************************************************************/
int neuik_VirtualFunc_RegisterImplementation(
    neuik_VirtualFunc * vFuncPtr,
    neuik_Class       * nClass,
    void              * funcImp)
{
    int                     ctr;
    int                     eNum       = 0;
    neuik_VirtualFunc       vFunc      = NULL;
    neuik_virtualFuncPair * thisPair   = NULL;
    static char             funcName[] = 
        "neuik_VirtualFunc_RegisterImplementation";
    static char           * errMsgs[]  = {"", // [0] no error
        "Output Argument `vFunc` is NULL.",   // [1]
        "Argument `nClass` is NULL.",         // [2]
        "Argument `funcImp` is NULL.",        // [3]
        "Failed to allocate memory.",         // [4]
        "Failed to reallocate memory.",       // [5]
    };

    /*------------------------------------------------------------------------*/
    /* Check for simple problems before continuing.                           */
    /*------------------------------------------------------------------------*/
    if (vFuncPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (nClass == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (funcImp == NULL)
    {
        eNum = 3;
        goto out;
    }

    vFunc = *vFuncPtr;

    /*------------------------------------------------------------------------*/
    /* Allocate/Reallocate the `vFunc` array.                                 */
    /*------------------------------------------------------------------------*/
    if (vFunc == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This first time this is to be allocated.                           */
        /*--------------------------------------------------------------------*/
        (*vFuncPtr) = 
            (neuik_virtualFuncPair **) malloc(2*sizeof(neuik_virtualFuncPair *));
        vFunc = *vFuncPtr;
        if (vFunc == NULL)
        {
            eNum = 4;
            goto out;
        }
        vFunc[1] = NULL; /* nullPtr terminated list */

        vFunc[0] = (neuik_virtualFuncPair *) malloc(sizeof(neuik_virtualFuncPair));
        if (vFunc[0] == NULL)
        {
            eNum = 4;
            goto out;
        }
        /* Set successfully allocated */

        ctr = 0;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* A subsequent set, reallocate the array.                            */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length of the array */
        for (ctr = 0;; ctr++)
        {
            if (vFunc[ctr] == NULL)
            {
                break;
            }
        }

        (*vFuncPtr) = (neuik_virtualFuncPair **) realloc((*vFuncPtr), 
            (2+ctr)*sizeof(neuik_virtualFuncPair *));
        vFunc = *vFuncPtr;
        if (vFunc == NULL)
        {
            eNum = 5;
            goto out;
        }
        vFunc[ctr + 1] = NULL; /* nullPtr terminated list */

        vFunc[ctr] = (neuik_virtualFuncPair *) malloc(sizeof(neuik_virtualFuncPair));
        if (vFunc[ctr] == NULL)
        {
            eNum = 4;
            goto out;
        }
        /* Set successfully allocated */
    }

    /*------------------------------------------------------------------------*/
    /* Store the values of the new virtual function pair                      */
    /*------------------------------------------------------------------------*/
    thisPair          = vFunc[ctr];
    thisPair->nClass  = nClass;
    thisPair->funcImp = funcImp;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_VirtualFunc_GetImplementation
 *
 *  Description:   Get funcImplementation from a vFunc set if it exists.
 *
 *  Returns:       NULL if no vFunc reimplemntation exists for the object, 
 *                 otherwise it returns a pointer to the appropriate function. 
 *
 ******************************************************************************/
void * neuik_VirtualFunc_GetImplementation(
    neuik_VirtualFunc   vFunc,
    void              * object)
{
    int                 ctr;
    void              * funcImp    = NULL;
    neuik_Object_Base * objBase    = NULL;
    neuik_Class       * thisClass  = NULL;

    if (vFunc == NULL) return NULL;
    if (!neuik_Object_IsNEUIKObject_NoError(object)) return NULL;

    /*------------------------------------------------------------------------*/
    /* Check the toplevel object to see if it provides a vFunc implementation */
    /*------------------------------------------------------------------------*/
    objBase   = (neuik_Object_Base*)(object);
    thisClass = (objBase->object).nClass;

    for (ctr = 0;; ctr++)
    {
        if (vFunc[ctr] == NULL) break;
        if (vFunc[ctr]->nClass == thisClass)
        {
            /*----------------------------------------------------------------*/
            /* A class which provides a virtual function for this was located */
            /*----------------------------------------------------------------*/
            funcImp = vFunc[ctr]->funcImp;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Iterate through the object SuperClasses till the correct Class Object  */
    /* is found.                                                              */
    /*------------------------------------------------------------------------*/
    objBase = (neuik_Object_Base*)(objBase->object.superClassObj);
    for (;;)
    {
        if (objBase == NULL) break; /* no more superClasses for this object */

        thisClass = (objBase->object).nClass;
        for (ctr = 0;; ctr++)
        {
            if (vFunc[ctr] == NULL) break;
            if (vFunc[ctr]->nClass == thisClass)
            {
                /*------------------------------------------------------------*/
                /* A class which provides a virtual function for this was     */
                /* located                                                    */
                /*------------------------------------------------------------*/
                funcImp = vFunc[ctr]->funcImp;
                goto out;
            }
        }
        /* point to the superClass of the current objectBase */
        objBase = (neuik_Object_Base*)(objBase->object.superClassObj);
    }
out:
    return funcImp;
}

