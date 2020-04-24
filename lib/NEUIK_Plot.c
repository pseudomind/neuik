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
#include <SDL.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "NEUIK_Plot.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Container.h"
#include "NEUIK_Event.h"
#include "NEUIK_Window.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Label.h"
#include "NEUIK_Frame.h"
#include "NEUIK_Transformer.h"
#include "NEUIK_CelGroup.h"
#include "NEUIK_HGroup.h"
#include "NEUIK_VGroup.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Plot(void ** contPtr);
int neuik_Object_Free__Plot(void * contPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
neuik_EventState neuik_Element_CaptureEvent__Plot(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_IsShown__Plot(NEUIK_Element);
int neuik_Element_SetWindowPointer__Plot(NEUIK_Element, void*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Plot_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__Plot,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__Plot,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Plot
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Plot()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_Plot";
    static char * errMsgs[]  = {"",                                        // [0] no error
        "NEUIK library must be initialized first.",                        // [1]
        "Failed to register `Plot` object class.",                         // [2]
        "Failed to register `Element_SetWindowPointer` virtual function.", // [3]
    };

    if (!neuik__isInitialized)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Otherwise, register the object                                         */
    /*------------------------------------------------------------------------*/
    if (neuik_RegisterClass(
        "NEUIK_Plot",                              // className
        "This Element contains a plot of values.", // classDescription
        neuik__Set_NEUIK,                          // classSet
        neuik__Class_Element,                      // superClass
        &neuik_Plot_BaseFuncs,                     // baseFuncs
        NULL,                                      // classFuncs XXXXX
        &neuik__Class_Plot))                       // newClass
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Register virtual function implementations                              */
    /*------------------------------------------------------------------------*/
    if (neuik_VirtualFunc_RegisterImplementation(
        &neuik_Element_vfunc_SetWindowPointer,
        neuik__Class_Plot,
        neuik_Element_SetWindowPointer__Plot))
    {
        eNum = 3;
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
 *  Name:          neuik_Object_New__Plot
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Plot(
    void ** plotPtr)
{
    int             eNum       = 0;
    NEUIK_Plot    * plot       = NULL;
    NEUIK_Element * sClassPtr  = NULL;
    static char     funcName[] = "neuik_Object_New__Plot";
    static char   * errMsgs[]  = {"",                  // [0] no error
        "Output Argument `plotPtr` is NULL.",          // [1]
        "Failure to allocate memory.",                 // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",    // [3]
        "Failure in function `neuik.NewElement`.",     // [4]
        "Failure in `NEUIK_NewVGroup()`.",             // [5]
        "Failure in `NEUIK_MakeLabel()`.",             // [6]
        "Failure in `NEUIK_NewFrame()`.",              // [7]
        "Failure in `NEUIK_Container_AddElements()`.", // [8]
        "Failure in `NEUIK_NewHGroup()`.",             // [9]
        "Failure in `NEUIK_NewTransformer()`.",        // [10]
        "Failure in `NEUIK_Container_SetElement()`.",  // [11]
        "Failure in `NEUIK_Transformer_Configure()`.", // [12]
        "Failure in `NEUIK_NewCelGroup()`.",           // [13]
        "Failure in `NEUIK_Element_Configure()`.",     // [13]
        "Failure in `NEUIK_VGroup_SetVSpacing()`.",    // [14]
    };

    if (plotPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*plotPtr) = (NEUIK_Plot*) malloc(sizeof(NEUIK_Plot));
    plot = *plotPtr;
    if (plot == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_Plot, 
            NULL,
            &(plot->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate the child objects                                             */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->visual))
    {
        eNum = 5;
        goto out;
    }
    if (NEUIK_Element_Configure(plot->visual, "FillAll", NULL))
    {
        eNum = 14;
        goto out;
    }

    if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->title))
    {
        eNum = 5;
        goto out;
    }
    if (NEUIK_VGroup_SetVSpacing((NEUIK_VGroup*)(plot->title), 0))
    {
        eNum = 14;
        goto out;
    }

    if (NEUIK_NewHGroup((NEUIK_HGroup**)&plot->hg_data))
    {
        eNum = 9;
        goto out;
    }
    if (NEUIK_Element_Configure(plot->hg_data, "FillAll", NULL))
    {
        eNum = 14;
        goto out;
    }

    if (NEUIK_NewTransformer((NEUIK_Transformer**)&plot->y_label_trans))
    {
        eNum = 10;
        goto out;
    }
    if (NEUIK_Transformer_Configure((NEUIK_Transformer*)plot->y_label_trans, 
        "Rotation=270.0", NULL))
    {
        eNum = 12;
        goto out;
    }
    if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->y_label))
    {
        eNum = 5;
        goto out;
    }
    if (NEUIK_VGroup_SetVSpacing((NEUIK_VGroup*)(plot->y_label), 0))
    {
        eNum = 14;
        goto out;
    }
    if (NEUIK_Container_SetElement(plot->y_label_trans, plot->y_label))
    {
        eNum = 11;
        goto out;
    }

    if (NEUIK_NewCelGroup((NEUIK_CelGroup**)&plot->drawing))
    {
        eNum = 13;
        goto out;
    }
    if (NEUIK_Element_Configure(plot->drawing, "FillAll", NULL))
    {
        eNum = 14;
        goto out;
    }

    if (NEUIK_MakeLabel((NEUIK_Label**)&plot->legend, "[Plot Legend]"))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Container_AddElements(plot->hg_data, 
        plot->y_label_trans,
        plot->drawing,
        plot->legend,
        NULL))
    {
        eNum = 9;
        goto out;
    }

    if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->x_label))
    {
        eNum = 5;
        goto out;
    }
    if (NEUIK_VGroup_SetVSpacing((NEUIK_VGroup*)(plot->x_label), 0))
    {
        eNum = 14;
        goto out;
    }

    if (NEUIK_Container_AddElements((NEUIK_VGroup*)plot->visual, 
            plot->title,
            plot->hg_data,
            plot->x_label,
            NULL))
    {
        eNum = 8;
        goto out;
    }

    plot->data_sets    = NULL;
    plot->data_configs = NULL;
    plot->n_allocated  = 0;
    plot->n_used       = 0;
    plot->x_range_cfg  = NEUIK_PLOTRANGECONFIG_AUTO;
    plot->x_range_min  = 0;
    plot->x_range_max  = 0;
    plot->y_range_cfg  = NEUIK_PLOTRANGECONFIG_AUTO;
    plot->y_range_min  = 0;
    plot->y_range_max  = 0;

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(plot->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }

    if (NEUIK_Plot_SetTitle(plot, "Title of Plot"))
    {
        eNum = 5;
        goto out;
    }
    neuik_Element_SetParentPointer(plot->visual, plot);
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
 *  Name:          neuik_Plot_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Plot(
    void * plotPtr)
{
    int                    eNum    = 0;    /* which error to report (if any) */
    unsigned int           uCtr    = 0;
    NEUIK_Plot           * plot    = NULL;
    neuik_PlotDataConfig * dataCfg = NULL;
    static char   funcName[] = "neuik_Object_Free__Plot";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `plotPtr` is NULL.",                           // [1]
        "Argument `plotPtr` is not of Plot class.",              // [2]
        "Failure in function `neuik_Object_Free` (superclass).", // [3]
        "Failure in function `neuik_Object_Free` (title).",      // [4]
        "Failure in function `neuik_Object_Free` (x_label).",    // [5]
        "Failure in function `neuik_Object_Free` (y_label).",    // [6]
        "Failure in function `neuik_Object_Free` (legend).",     // [7]
        "Failure in function `neuik_Object_Free` (visual).",     // [8]
    };

    if (plotPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(plotPtr, neuik__Class_Plot))
    {
        eNum = 2;
        goto out;
    }
    plot = (NEUIK_Plot*)plotPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(plot->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Free the typical plot elements (if still allocated                     */
    /*------------------------------------------------------------------------*/
    if (plot->title != NULL)
    {
        if(neuik_Object_Free(plot->title))
        {
            eNum = 4;
            goto out;
        }
    }
    if (plot->x_label != NULL)
    {
        if(neuik_Object_Free(plot->x_label))
        {
            eNum = 5;
            goto out;
        }
    }
    if (plot->y_label != NULL)
    {
        if(neuik_Object_Free(plot->y_label))
        {
            eNum = 6;
            goto out;
        }
    }
    if (plot->drawing != NULL)
    {
        if(neuik_Object_Free(plot->drawing))
        {
            eNum = 6;
            goto out;
        }
    }
    if (plot->legend != NULL)
    {
        if(neuik_Object_Free(plot->legend))
        {
            eNum = 7;
            goto out;
        }
    }
    if (plot->visual != NULL)
    {
        if(neuik_Object_Free(plot->visual))
        {
            eNum = 8;
            goto out;
        }
    }

    if (plot->data_configs != NULL)
    {
        for (uCtr = plot->n_used; uCtr < plot->n_allocated; uCtr++)
        {
            dataCfg = &(plot->data_configs[uCtr]);
            if (dataCfg->uniqueName != NULL) free(dataCfg->uniqueName);
            if (dataCfg->label      != NULL) free(dataCfg->label);
        }

        free(plot->data_configs);
    }

    free(plot);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


// /*******************************************************************************
//  *
//  *  Name:          neuik_Element_CaptureEvent__Plot
//  *
//  *  Description:   A virtual function reimplementation of the function
//  *                 neuik_Element_CaptureEvent.
//  *
//  *  Returns:       1 if the event was captured; 0 otherwise.
//  *
//  ******************************************************************************/
// neuik_EventState neuik_Element_CaptureEvent__Plot(
//  NEUIK_Element   cont, 
//  SDL_Event     * ev)
// {
//  int                ctr        = 0;
//  neuik_EventState   evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
//  NEUIK_Element      elem;
//  NEUIK_Plot       * cBase;

//  if (neuik_Object_GetClassObject_NoError(
//      cont, neuik__Class_Plot, (void**)&cBase)) goto out;

//  if (cBase->elems != NULL)
//  {
//      for (ctr = 0;; ctr++)
//      {
//          elem = cBase->elems[ctr];
//          if (elem == NULL) break;

//          if (!NEUIK_Element_IsShown(elem)) continue;

//          evCaputred = neuik_Element_CaptureEvent(elem, ev);
//          if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
//          {
//              goto out;
//          }
//          if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
//          {
//              neuik_Element_SetActive(cont, 1);
//              goto out;
//          }
//      }
//  }
// out:
//  return evCaputred;
// }


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer__Plot (redefined-vfunc)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
******************************************************************************/
int neuik_Element_SetWindowPointer__Plot (
    NEUIK_Element   plotPtr, 
    void          * win)
{
    int                 eNum     = 0;
    NEUIK_ElementBase * eBase    = NULL;
    NEUIK_Plot        * plot     = NULL;
    static int          nRecurse = 0; /* number of times recursively called */
    static char         funcName[] = "neuik_Element_SetWindowPointer__Plot";
    static char       * errMsgs[]  = {"",                                    // [0] no error
        "Argument `elem` caused `GetClassObject` to fail. Not a Plot?.",     // [1]
        "Child Element caused `SetWindowPointer` to fail.",                  // [2]
        "Argument `elem` caused `GetClassObject` to fail. Not an Element?.", // [3]
        "Argument `win` does not implement Window class.",                   // [4]
    };

    nRecurse++;
    if (nRecurse > NEUIK_MAX_RECURSION)
    {
        /*--------------------------------------------------------------------*/
        /* This is likely a case of appears to be runaway recursion; report   */
        /* an error to the user.                                              */
        /*--------------------------------------------------------------------*/
        neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
        goto out;
    }

    if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the window pointers for typical plot elements (if present).        */
    /*------------------------------------------------------------------------*/
    if (plot->title != NULL)
    {
        if (neuik_Element_SetWindowPointer(plot->title, win))
        {
            eNum = 2;
            goto out;
        }
    }
    if (plot->x_label != NULL)
    {
        if (neuik_Element_SetWindowPointer(plot->x_label, win))
        {
            eNum = 2;
            goto out;
        }
    }
    if (plot->y_label != NULL)
    {
        if (neuik_Element_SetWindowPointer(plot->y_label, win))
        {
            eNum = 2;
            goto out;
        }
    }
    if (plot->legend != NULL)
    {
        if (neuik_Element_SetWindowPointer(plot->legend, win))
        {
            eNum = 2;
            goto out;
        }
    }
    if (plot->visual != NULL)
    {
        if (neuik_Element_SetWindowPointer(plot->visual, win))
        {
            eNum = 2;
            goto out;
        }
    }

    if (neuik_Object_GetClassObject(plot, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 3;
        goto out;
    }

    if (!neuik_Object_ImplementsClass(win, neuik__Class_Window))
    {
        eNum = 4;
        goto out;
    }

    eBase->eSt.window = win;
out:
    nRecurse--;
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


// /*******************************************************************************
//  *
//  *  Name:          NEUIK_Plot_Configure
//  *
//  *  Description:   Configure one or more settings for a container.
//  *
//  *                 NOTE: This list of settings must be terminated by a NULL
//  *                 pointer.
//  *
//  *  Returns:       1 if there is an error; 0 otherwise.
//  *
//  ******************************************************************************/
// int NEUIK_Plot_Configure(
//  NEUIK_Element   cont,
//  const char    * set0,
//  ...)
// {
//  int          ctr;
//  int          isBool;
//  int          boolVal    = 0;
//  int          doRedraw   = 0;
//  int          typeMixup;
//  va_list      args;
//  char       * strPtr     = NULL;
//  char       * name       = NULL;
//  char       * value      = NULL;
//  const char * set        = NULL;
//  char         buf[4096];
//  NEUIK_Plot * cBase      = NULL;
//  /*------------------------------------------------------------------------*/
//  /* If a `name=value` string with an unsupported name is found, check to   */
//  /* see if a boolName was mistakenly used instead.                         */
//  /*------------------------------------------------------------------------*/
//  // static char     * boolNames[] = {
//  //  NULL,
//  // };
//  /*------------------------------------------------------------------------*/
//  /* If a boolName string with an unsupported name is found, check to see   */
//  /* if a supported nameValue type was mistakenly used instead.             */
//  /*------------------------------------------------------------------------*/
//  // static char     * valueNames[] = {
//  //  "HJustify",
//  //  "VJustify",
//  //  NULL,
//  // };
//  static char       funcName[] = "NEUIK_Plot_Configure";
//  static char     * errMsgs[]  = {"",                                  // [ 0] no error
//      "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [ 1]
//      "NamedSet.name is NULL, skipping.",                              // [ 2]
//      "NamedSet.name is blank, skipping.",                             // [ 3]
//      "NamedSet.name type unknown, skipping.",                         // [ 4]
//      "`name=value` string is too long.",                              // [ 5]
//      "Set string is empty.",                                          // [ 6]
//      "HJustify value is invalid.",                                    // [ 7]
//      "VJustify value is invalid.",                                    // [ 8]
//      "BoolType name unknown, skipping.",                              // [ 9]
//      "Invalid `name=value` string.",                                  // [10]
//      "ValueType name used as BoolType, skipping.",                    // [11]
//      "BoolType name used as ValueType, skipping.",                    // [12]
//  };

//  if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
//  {
//      NEUIK_RaiseError(funcName, errMsgs[1]);
//      return 1;
//  }

//  set = set0;

//  va_start(args, set0);

//  for (ctr = 0;; ctr++)
//  {
//      isBool = 0;
//      name   = NULL;
//      value  = NULL;

//      if (set == NULL) break;

//      // #ifndef NO_NEUIK_SIGNAL_TRAPPING
//      //  signal(SIGSEGV, neuik_Element_Configure_capture_segv);
//      // #endif

//      if (strlen(set) > 4095)
//      {
//          // #ifndef NO_NEUIK_SIGNAL_TRAPPING
//          //  signal(SIGSEGV, NULL);
//          // #endif
//          NEUIK_RaiseError(funcName, errMsgs[5]);
//          set = va_arg(args, const char *);
//          continue;
//      }
//      else
//      {
//          // #ifndef NO_NEUIK_SIGNAL_TRAPPING
//          //  signal(SIGSEGV, NULL);
//          // #endif
//          strcpy(buf, set);
//          /* Find the equals and set it to '\0' */
//          strPtr = strchr(buf, '=');
//          if (strPtr == NULL)
//          {
//              /*------------------------------------------------------------*/
//              /* Bool type configuration (or a mistake)                     */
//              /*------------------------------------------------------------*/
//              if (buf[0] == 0)
//              {
//                  NEUIK_RaiseError(funcName, errMsgs[6]);
//              }

//              isBool  = 1;
//              boolVal = 1;
//              name    = buf;
//              if (buf[0] == '!')
//              {
//                  boolVal = 0;
//                  name    = buf + 1;
//              }
//          }
//          else
//          {
//              *strPtr = 0;
//              strPtr++;
//              if (*strPtr == 0)
//              {
//                  /* `name=value` string is missing a value */
//                  NEUIK_RaiseError(funcName, errMsgs[10]);
//                  set = va_arg(args, const char *);
//                  continue;
//              }
//              name  = buf;
//              value = strPtr;
//          }
//      }

//      if (!isBool)
//      {
//          if (name == NULL)
//          {
//              NEUIK_RaiseError(funcName, errMsgs[2]);
//          }
//          else if (name[0] == 0)
//          {
//              NEUIK_RaiseError(funcName, errMsgs[3]);
//          }
//          else if (!strcmp("HJustify", name))
//          {
//              if (!strcmp("left", value))
//              {
//                  cBase->HJustify = NEUIK_HJUSTIFY_LEFT;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("center", value))
//              {
//                  cBase->HJustify = NEUIK_HJUSTIFY_CENTER;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("right", value))
//              {
//                  cBase->HJustify = NEUIK_HJUSTIFY_RIGHT;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("default", value))
//              {
//                  cBase->HJustify = NEUIK_HJUSTIFY_DEFAULT;
//                  doRedraw = 1;
//              }
//              else
//              {
//                  NEUIK_RaiseError(funcName, errMsgs[7]);
//              }
//          }
//          else if (!strcmp("VJustify", name))
//          {
//              if (!strcmp("top", value))
//              {
//                  cBase->VJustify = NEUIK_VJUSTIFY_TOP;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("center", value))
//              {
//                  cBase->VJustify = NEUIK_VJUSTIFY_CENTER;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("bottom", value))
//              {
//                  cBase->VJustify = NEUIK_VJUSTIFY_BOTTOM;
//                  doRedraw = 1;
//              }
//              else if (!strcmp("default", value))
//              {
//                  cBase->VJustify = NEUIK_VJUSTIFY_DEFAULT;
//                  doRedraw = 1;
//              }
//              else
//              {
//                  NEUIK_RaiseError(funcName, errMsgs[8]);
//              }
//          }
//          else
//          {
//              typeMixup = 0;
//              // for (nCtr = 0;; nCtr++)
//              // {
//              //  if (boolNames[nCtr] == NULL) break;

//              //  if (!strcmp(boolNames[nCtr], name))
//              //  {
//              //      typeMixup = 1;
//              //      break;
//              //  }
//              // }

//              if (typeMixup)
//              {
//                  /* A bool type was mistakenly used as a value type */
//                  NEUIK_RaiseError(funcName, errMsgs[12]);
//              }
//              else
//              {
//                  /* An unsupported name was used as a value type */
//                  NEUIK_RaiseError(funcName, errMsgs[4]);
//              }
//          }
//      }

//      /* before starting */
//      set = va_arg(args, const char *);
//  }
//  va_end(args);

//  if (doRedraw) neuik_Element_RequestRedraw(cont);

//  return 0;
// }

/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_SetTitle
 *
 *  Description:   Update the title of a NEUIK_Plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot_SetTitle(
    NEUIK_Element   plotPtr,
    const char    * text)
{
    int           eNum = 0; /* which error to report (if any) */
    char *        textCopy = NULL; /* should be freed when done */
    char *        strPtr0  = NULL;
    char *        strPtr1  = NULL;
    NEUIK_Label * newLabel = NULL;
    NEUIK_Plot  * plot     = NULL;
    RenderSize    rSize;
    RenderLoc     rLoc;
    static char   funcName[] = "NEUIK_Plot_SetTitle";
    static char * errMsgs[] = {"",                                         // [0] no error
        "Argument `plot` does not implement Plot class.",                  // [1]
        "Argument `plot` caused `neuik_Object_GetClassObject()` to fail.", // [2]
        "Failure to allocate memory.",                                     // [3]
        "Failure in `NEUIK_MakeLabel()`.",                                 // [4]
        "Failure to `String_Duplicate()`.",                                // [5]
        "Failure to `NEUIK_Container_AddElement()`.",                      // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                // [7]
    };

    if (!neuik_Object_ImplementsClass(plotPtr, neuik__Class_Plot))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Conditionally free Plot title before setting the new contents          */
    /*------------------------------------------------------------------------*/
    if (plot->title != NULL) {
        NEUIK_Container_DeleteElements(plot->title);
    }

    /*------------------------------------------------------------------------*/
    /* Set the new Label text contents                                        */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* Title will contain no text */
        goto out;
    }
    else if (text[0] == '\0')
    {
        /* Title will contain no text */
        goto out;
    }

    String_Duplicate(&textCopy, text);

    if (textCopy == NULL)
    {
        eNum = 5;
        goto out;
    }

    strPtr1 = textCopy;
    for (;;)
    {
        strPtr0 = strchr(strPtr1, '\n');
        if (strPtr0 == NULL)
        {
            /*----------------------------------------------------------------*/
            /* There are no more newlines in the string                       */
            /*----------------------------------------------------------------*/
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->title, newLabel))
            {
                eNum = 6;
                goto out;
            }
            break;
        } 
        else
        {
            *strPtr0 = '\0';
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->title, newLabel))
            {
                eNum = 6;
                goto out;
            }
            strPtr0++;
            strPtr1 = strPtr0;
        }
    }

    if (neuik_Element_GetSizeAndLocation(plot, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    neuik_Element_RequestRedraw(plot, rLoc, rSize);
out:
    if (textCopy != NULL) free(textCopy);
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }
out2:
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_SetXAxisLabel
 *
 *  Description:   Update the x-axis label of a NEUIK_Plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot_SetXAxisLabel(
    NEUIK_Element   plotPtr,
    const char    * text)
{
    int           eNum = 0; /* which error to report (if any) */
    char *        textCopy = NULL; /* should be freed when done */
    char *        strPtr0  = NULL;
    char *        strPtr1  = NULL;
    NEUIK_Label * newLabel = NULL;
    NEUIK_Plot  * plot     = NULL;
    RenderSize    rSize    = {0, 0};
    RenderLoc     rLoc     = {0, 0};;
    static char   funcName[] = "NEUIK_Plot_SetXAxisLabel";
    static char * errMsgs[] = {"",                                         // [0] no error
        "Argument `plot` does not implement Plot class.",                  // [1]
        "Argument `plot` caused `neuik_Object_GetClassObject()` to fail.", // [2]
        "Failure to allocate memory.",                                     // [3]
        "Failure in `NEUIK_MakeLabel()`.",                                 // [4]
        "Failure to `String_Duplicate()`.",                                // [5]
        "Failure to `NEUIK_Container_AddElement()`.",                      // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                // [7]
    };

    if (!neuik_Object_ImplementsClass(plotPtr, neuik__Class_Plot))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Conditionally free Plot xAxisLabel before setting the new contents     */
    /*------------------------------------------------------------------------*/
    if (plot->x_label != NULL) {
        NEUIK_Container_DeleteElements(plot->x_label);
    }

    /*------------------------------------------------------------------------*/
    /* Set the new Label text contents                                        */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* Title will contain no text */
        goto out;
    }
    else if (text[0] == '\0')
    {
        /* Title will contain no text */
        goto out;
    }

    String_Duplicate(&textCopy, text);

    if (textCopy == NULL)
    {
        eNum = 5;
        goto out;
    }

    strPtr1 = textCopy;
    for (;;)
    {
        strPtr0 = strchr(strPtr1, '\n');
        if (strPtr0 == NULL)
        {
            /*----------------------------------------------------------------*/
            /* There are no more newlines in the string                       */
            /*----------------------------------------------------------------*/
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->x_label, newLabel))
            {
                eNum = 6;
                goto out;
            }
            break;
        } 
        else
        {
            *strPtr0 = '\0';
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->x_label, newLabel))
            {
                eNum = 6;
                goto out;
            }
            strPtr0++;
            strPtr1 = strPtr0;
        }
    }

    if (neuik_Element_GetSizeAndLocation(plot, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    neuik_Element_RequestRedraw(plot, rLoc, rSize);
out:
    if (textCopy != NULL) free(textCopy);
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }
out2:
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_SetYAxisLabel
 *
 *  Description:   Update the x-axis label of a NEUIK_Plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot_SetYAxisLabel(
    NEUIK_Element   plotPtr,
    const char    * text)
{
    int           eNum = 0; /* which error to report (if any) */
    char *        textCopy = NULL; /* should be freed when done */
    char *        strPtr0  = NULL;
    char *        strPtr1  = NULL;
    NEUIK_Label * newLabel = NULL;
    NEUIK_Plot  * plot     = NULL;
    RenderSize    rSize    = {0, 0};
    RenderLoc     rLoc     = {0, 0};;
    static char   funcName[] = "NEUIK_Plot_SetYAxisLabel";
    static char * errMsgs[] = {"",                                         // [0] no error
        "Argument `plot` does not implement Plot class.",                  // [1]
        "Argument `plot` caused `neuik_Object_GetClassObject()` to fail.", // [2]
        "Failure to allocate memory.",                                     // [3]
        "Failure in `NEUIK_MakeLabel()`.",                                 // [4]
        "Failure to `String_Duplicate()`.",                                // [5]
        "Failure to `NEUIK_Container_AddElement()`.",                      // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                // [7]
    };

    if (!neuik_Object_ImplementsClass(plotPtr, neuik__Class_Plot))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Conditionally free Plot yAxisLabel before setting the new contents     */
    /*------------------------------------------------------------------------*/
    if (plot->y_label != NULL) {
        NEUIK_Container_DeleteElements(plot->y_label);
    }

    /*------------------------------------------------------------------------*/
    /* Set the new Label text contents                                        */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* Title will contain no text */
        goto out;
    }
    else if (text[0] == '\0')
    {
        /* Title will contain no text */
        goto out;
    }

    String_Duplicate(&textCopy, text);

    if (textCopy == NULL)
    {
        eNum = 5;
        goto out;
    }

    strPtr1 = textCopy;
    for (;;)
    {
        strPtr0 = strchr(strPtr1, '\n');
        if (strPtr0 == NULL)
        {
            /*----------------------------------------------------------------*/
            /* There are no more newlines in the string                       */
            /*----------------------------------------------------------------*/
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->y_label, newLabel))
            {
                eNum = 6;
                goto out;
            }
            break;
        } 
        else
        {
            *strPtr0 = '\0';
            if (NEUIK_MakeLabel(&newLabel, strPtr1))
            {
                eNum = 4;
                goto out;
            }
            if (NEUIK_Container_AddElement(plot->y_label, newLabel))
            {
                eNum = 6;
                goto out;
            }
            strPtr0++;
            strPtr1 = strPtr0;
        }
    }

    if (neuik_Element_GetSizeAndLocation(plot, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    neuik_Element_RequestRedraw(plot, rLoc, rSize);
out:
    if (textCopy != NULL) free(textCopy);
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }
out2:
    return eNum;
}
