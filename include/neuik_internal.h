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
#ifndef NEUIK_INTERNAL_H
#define NEUIK_INTERNAL_H

#include <stdlib.h>

#define NEUIK_MAX_RECURSION 1000

typedef int neuik_SetID;
typedef int neuik_ClassID;

typedef enum {
	NEUIK_EVENTSTATE_NOT_CAPTURED,
	NEUIK_EVENTSTATE_CAPTURED,
	NEUIK_EVENTSTATE_OBJECT_FREED,
} neuik_EventState;

typedef enum {
	NEUIK_FATALERROR_NO_ERROR,
	NEUIK_FATALERROR_RUNAWAY_RECURSION,
	NEUIK_FATALERROR_SIGSEGV_CAPTURED,
} neuik_FatalError;

extern neuik_FatalError neuik_Fatal;

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
typedef struct {
	/* Init(): Class initialization (in most cases will not be needed) */
	int  (*Init)  (void *);
	/* New(): Allocate and Initialize the object */
	int  (*New)   (void **);
	/* Copy(): Copy the contents of one object into another */
	int  (*Copy)  (void *, const void *);
	/* Free(): Free the allocated memory of an object */
	int  (*Free)  (void *);
} neuik_Class_BaseFuncs;

typedef struct {
	neuik_SetID   SetID;          /* identifies origin object class set */
	char        * SetName;
	char        * SetDescription;
} neuik_Set;

typedef struct {
	neuik_ClassID           ClassID;    /* identifies origin object class set */
	char                  * ClassName;
	char                  * ClassDescription;
	neuik_Set             * Set;        /* points to the parent class set */
	void                  * SuperClass; /* (neuik_Class*) */
	neuik_Class_BaseFuncs * baseFuncs;
	void                  * classFuncs;
} neuik_Class;


typedef void * NEUIK_Object;

/*----------------------------------------------------------------------------*/
/* neuik_Object                                                               */
/*                                                                            */
/* This struct identifies that this object is an NEUIK object, identifies the */
/* parent set, and the type of class and contains a pointer to the object     */
/* function table.                                                            */
/*----------------------------------------------------------------------------*/
typedef struct {
	unsigned int    mustBe_1337;   /* generates a runtime error if not set to 1337  */
	unsigned int    mustBe_90210;  /* generates a runtime error if not set to 90210 */
	neuik_Set     * nSet;          /* pointer to parent set */
	neuik_Class   * nClass;        /* pointer to class */
	void          * superClassObj; /* ptr to the superClass object of this Object (NULL if None) */
} neuik_Object;

/*----------------------------------------------------------------------------*/
/* neuik_Object_base                                                          */
/*                                                                            */
/* This struct that NEUIK objects will be casted into for treatment as an     */
/* object.                                                                    */
/*----------------------------------------------------------------------------*/
typedef struct {
	neuik_Object object;
} neuik_Object_Base;

/*----------------------------------------------------------------------------*/
/* neuik_virtualFuncPair                                                      */
/*                                                                            */
/* This struct represents a pairing of a class and its class-specific         */
/* implementation of the virtual function.                                    */
/*----------------------------------------------------------------------------*/
typedef struct {
	neuik_Class * nClass;
	void        * funcImp;
} neuik_virtualFuncPair;

/*----------------------------------------------------------------------------*/
/* neuik_VirtualFunc                                                          */
/*                                                                            */
/* One or more virtual function pairs define a virtual function set.          */
/*----------------------------------------------------------------------------*/
typedef neuik_virtualFuncPair ** neuik_VirtualFunc;


int
	neuik_RegisterClassSet(
			const char  * setName,
			const char  * setDescription,
			neuik_Set  ** newSet);


int
	neuik_RegisterClass(
			const char             * className,
			const char             * classDescription,
			neuik_Set              * classSet,
			neuik_Class            * superClass,
			neuik_Class_BaseFuncs  * baseFuncs,
			void                   * classFuncs,
			neuik_Class           ** newClass);

int
	neuik_HasFatalError();

int 
	neuik_Object_New(
			neuik_Class  * objClass,
			void        ** object);

int 
	neuik_Object_Free(
			void * objPtr);

int
	neuik_Object_IsNEUIKObject(
			const void * objPtr);

int	
	neuik_Object_IsNEUIKObject_NoError(
			const void * objPtr);

int
	neuik_Object_ImplementsClass(
			const void    * object,
			neuik_Class   * nClass);

int
	neuik_Object_ImplementsClass_NoError(
			const void    * object,
			neuik_Class   * nClass);

int
	neuik_Object_IsClass(
			const void  * object,
			neuik_Class * nClass);

int	
	neuik_Object_IsClass_NoErr(
			const void  * objPtr,
			neuik_Class * nClass);

int
	neuik_Object_InClassSet(
			const void  * object,
			neuik_SetID   setID);

int
	neuik_GetParentSetOfClass(
			neuik_ClassID   classID,
			neuik_SetID   * setID);

int 
	neuik_GetObjectBaseOfClass(
			neuik_Set    * objSet,
			neuik_Class  * objClass,
			void         * superClassObj,
			neuik_Object * object);

int
	neuik_InitObjectOfClass(
			neuik_Object   * object,
			neuik_ClassID  * thisClassID);

int	
	neuik_Object_GetClassObject(
			void         * objPtr,
			neuik_Class  * nClass,
			void        ** classObject);

int	
	neuik_Object_GetClassObject_NoError(
			void         * objPtr,
			neuik_Class  * nClass,
			void        ** classObject);

void 
	neuik_Object_Show(
			void * objPtr,
			int    verbosity);

int
	neuik_VirtualFunc_RegisterImplementation(
			neuik_VirtualFunc * vFunc,
			neuik_Class       * nClass,
			void              * funcImp);

void * 
	neuik_VirtualFunc_GetImplementation(
			neuik_VirtualFunc   vFunc,
			void              * object);


#endif /* NEUIK_INTERNAL_H */
