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
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "NEUIK_neuik.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Image.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_StockImage_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Image(void ** wPtr);
int neuik_Object_Free__Image(void ** wPtr);

int           neuik_Element_GetMinSize__Image(NEUIK_Element, RenderSize*);
SDL_Texture * neuik_Element_Render__Image(NEUIK_Element, RenderSize*, SDL_Renderer*);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Image_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Image,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Image,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Image_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Image,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Image,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Image
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Image()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Image";
	static char  * errMsgs[]  = {"",                 // [0] no error
		"NEUIK library must be initialized first.",  // [1]
		"Failed to register `Image` object class .", // [2]
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
		"NEUIK_Image",                        // className
		"An object which contains an image.", // classDescription
		neuik__Set_NEUIK,                     // classSet
		neuik__Class_Element,                 // superClass
		&neuik_Image_BaseFuncs,               // baseFuncs
		NULL,                                 // classFuncs
		&neuik__Class_Image))                 // newClass
	{
		eNum = 2;
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
 *  Name:          neuik_Object_New__Image
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Image(
	void ** imgPtr)
{
	int             eNum       = 0; /* which error to report (if any) */
	NEUIK_Image   * img        = NULL;
	NEUIK_Element * sClassPtr  = NULL;
	static char     funcName[] = "neuik_Object_New__Image";
	static char   * errMsgs[]  = {"",                        // [0] no error
		"Failure to allocate memory.",                       // [1]
		"Failure in NEUIK_NewImageConfig.",                  // [2]
		"Output Argument `imgPtr` is NULL.",                 // [3]
		"Failure in function `neuik_Object_New`.",           // [4]
		"Failure in function `neuik_Element_SetFuncTable`.", // [5]
		"Failure in `neuik_GetObjectBaseOfClass`.",          // [6]
	};

	if (imgPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*imgPtr) = (NEUIK_Image *)malloc(sizeof(NEUIK_Image));
	img = *imgPtr;
	if (img == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Image, 
			NULL,
			&(img->objBase)))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(img->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_Image_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	/* Allocation successful */
	img->cfg         = NULL;
	img->cfgPtr      = NULL;
	img->needsRedraw = 1;

	if (NEUIK_NewImageConfig(&img->cfg))
	{
		eNum = 2;
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
 *  Name:          neuik_Object_Free__Image
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Image(
	void  ** imgPtr)  /* [out] the image to free */
{
	int           eNum       = 0; /* which error to report (if any) */
	NEUIK_Image * img        = NULL;
	static char   funcName[] = "neuik_Object_Free__Image";
	static char * errMsgs[]  = {"",                 // [0] no error
		"Argument `imgPtr` is not of Image class.", // [1]
		"Failure in function `neuik_Object_Free`.", // [2]
		"Argument `imgPtr` is NULL.",               // [3]
	};

	if (imgPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	if (!neuik_Object_IsClass(*imgPtr, neuik__Class_Image))
	{
		eNum = 1;
		goto out;
	}
	img = (NEUIK_Image *)(*imgPtr);

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(&(img->objBase.superClassObj)))
	{
		eNum = 2;
		goto out;
	}
	if(img->image != NULL) SDL_FreeSurface(img->image);
	if(neuik_Object_Free((void**)&(img->cfg)))
	{
		eNum = 2;
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
 *  Name:          NEUIK_MakeImage
 *
 *  Description:   Create a new NEUIK_Image and load image data into it from
 *                 the specified image file.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeImage(
	NEUIK_Image ** imgPtr,   /* [out] The newly created NEUIK_Image. */
	const char   * filename) /* [in]  The filename of the image to load. */
{
	int           eNum       = 0; /* which error to report (if any) */
	NEUIK_Image * img        = NULL;
	static char   funcName[] = "NEUIK_MakeImage";
	static char * errMsgs[]  = {"",                       // [0] no error
		"Failure in function `neuik_Object_New__Image`.", // [1]
		"Failure in function `IMG_Load`.",                // [2]
	};

	if (neuik_Object_New__Image((void**)imgPtr))
	{
		eNum = 1;
		goto out;
	}
	img = *imgPtr;

	/*------------------------------------------------------------------------*/
	/* Set the new Image data contents                                        */
	/*------------------------------------------------------------------------*/
	if (filename == NULL){
		/* image will contain no data */
		img->image = NULL;
	}
	else if (filename[0] == '\0')
	{
		/* image will contain no data */
		img->image = NULL;
	}
	else
	{
		img->image = IMG_Load(filename);
		if (img->image == NULL)
		{
			eNum = 2;
			goto out;
		}
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
 *  Name:          NEUIK_MakeImage_FromSource
 *
 *  Description:   Create a new NEUIK_Image and load image data into it from
 *                 a data stream.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeImage_FromSource(
	NEUIK_Image         ** imgPtr,    /* [out] The newly created NEUIK_Image. */
	int                    width,     /* [in] width of image */
	int                    height,    /* [in] height of image */
	int                    bytespp,   /* [in] bytes per pixel [= 2, 3, or 4] */
	const unsigned char  * pixeldata) /* [in] pixel data. */
{
	int                   eNum    = 0;    /* which error to report (if any) */
	int                   wCtr;           /* counter for image width */
	int                   hCtr;           /* counter for image height */
	NEUIK_Image         * img     = NULL;
	SDL_Surface         * imgSurf = NULL;
	unsigned char         pixelR;
	unsigned char         pixelG;
	unsigned char         pixelB;
	unsigned char         pixelA;
	unsigned int          pixRGBA;
	unsigned int        * destRGBA;
	Uint32                rmask;
	Uint32                gmask;
	Uint32                bmask;
	Uint32                amask;
	const unsigned char * srcPixels;
	static char     funcName[] = "NEUIK_MakeImage_FromSource";
	static char   * errMsgs[]  = {"",                                       // [0] no error
		"Failure in function `neuik_Object_New__Image`.",                   // [1]
		"Argument `bytespp` (bytes-per-pixel) is invalid.",                 // [2]
		"Argument `bytespp` (bytes-per-pixel) supplied unsupported value.", // [3]
		"Failed to create RGB surface.",                                    // [4]
		"Argument `width` has invalid value.",                              // [5]
		"Argument `height` has invalid value.",                             // [6]
		"Argument `pixelData` is NULL.",                                    // [7]
	};

	if (neuik_Object_New__Image((void**)imgPtr))
	{
		eNum = 1;
		goto out;
	}
	img = *imgPtr;

	if (width <= 0)
	{
		eNum = 5;
		goto out;
	}
	else if (height <= 0)
	{
		eNum = 6;
		goto out;
	}

	if (bytespp == 2)
	{
		/*--------------------------------------------------------------------*/
		/* RGB16... currently unsupported but should be supported eventually  */
		/*--------------------------------------------------------------------*/
		eNum = 3;
		goto out;
	}
	if (bytespp == 3)
	{
		/*--------------------------------------------------------------------*/
		/* RGB16... currently unsupported but should be supported eventually  */
		/*--------------------------------------------------------------------*/
		eNum = 3;
		goto out;
	}
	else if (bytespp != 3 && bytespp != 4)
	{
		/*--------------------------------------------------------------------*/
		/* This pixel format is just not supported                            */
		/*--------------------------------------------------------------------*/
		eNum = 2;
		goto out;
	}

	if (bytespp == 4)
	{
		/*--------------------------------------------------------------------*/
		/* pixelData is expected to contain RGBA data (in that order) at      */
		/* 32 bits-per-pixel.            d                                    */
		/*--------------------------------------------------------------------*/
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;

		/*--------------------------------------------------------------------*/
		/* Create the surface                                                 */
		/*--------------------------------------------------------------------*/
		img->image = SDL_CreateRGBSurface(0, 
			width, height, 32, rmask, gmask, bmask, amask);
		if (img->image == NULL)
		{
			eNum = 4;
			goto out;
		}
		srcPixels = pixeldata;
		imgSurf   = (SDL_Surface*)img->image;
		destRGBA  = imgSurf->pixels;

		/*--------------------------------------------------------------------*/
		/* Set the new Image data contents                                    */
		/*--------------------------------------------------------------------*/
		for (hCtr = 0; hCtr < height; hCtr++)
		{
			for (wCtr = 0; wCtr < width; wCtr++)
			{
				pixelR = *(srcPixels++);
				pixelG = *(srcPixels++);
				pixelB = *(srcPixels++);
				pixelA = *(srcPixels++);

				pixRGBA =  pixelA;
				pixRGBA += (pixelB << 8);
				pixRGBA += (pixelG << 16);
				pixRGBA += (pixelR << 24);

				*(destRGBA++) = pixRGBA;
			}
		}
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
 *  Name:          NEUIK_MakeImage_FromStock
 *
 *  Description:   Create a new NEUIK_Image and load image data into it from
 *                 a data stream.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeImage_FromStock(
	NEUIK_Image      ** imgPtr,     /* [out] The newly created NEUIK_Image. */
	NEUIK_StockImage    stockImage) /* [in]  The name of the stock image. */
{
	int           eNum       = 0;    /* return value */
	static char   funcName[] = "NEUIK_MakeImage_FromStock";
	static char * errMsgs[]  = {"",                               // [0] no error
		"Failure in function `neuik_GetStockImage_app_crashed`.", // [1]
	};

	switch (stockImage)
	{
		case NEUIK_STOCKIMAGE_APP_CRASHED:
			if (neuik_GetStockImage_app_crashed(imgPtr))
			{
				eNum = 1;
			}
			break;
		case NEUIK_STOCKIMAGE_NEUIK_ICON:
			if (neuik_GetStockImage_neuik_icon(imgPtr))
			{
				eNum = 1;
			}
			break;
		case NEUIK_STOCKIMAGE_NEUIK_LOGO:
			if (neuik_GetStockImage_neuik_logo(imgPtr))
			{
				eNum = 1;
			}
			break;
	}

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewImage
 *
 *  Description:   Create a new NEUIK_Image with no data.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewImage(
	NEUIK_Image ** imgPtr) /* [out] The newly created NEUIK_Image. */
{
	return neuik_Object_New__Image((void**)imgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Image_GetMinSize
 *
 *  Description:   Returns the rendered size of a given Image.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Image(
	NEUIK_Element    imgElem,
	RenderSize     * rSize)
{
	int                       imW        = 0;
	int                       imH        = 0;
	int                       eNum       = 0;    /* which error to report (if any) */
	NEUIK_Image             * img        = NULL;
	const NEUIK_ImageConfig * aCfg       = NULL; /* the active Image config */
	static char               funcName[] = "neuik_Element_GetMinSize__Image";
	static char             * errMsgs[]  = {"",      // [0] no error
		"Argument `imgElem` is not of Image class.", // [1]
		"ImageConfig* is NULL.",                     // [2]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(imgElem, neuik__Class_Image))
	{
		eNum = 1;
		goto out;
	}
	img = (NEUIK_Image *)(imgElem);
	
	/* select the correct Image config to use (pointer or internal) */
	if (img->cfgPtr != NULL)
	{
		aCfg = img->cfgPtr;
	}
	else 
	{
		aCfg = img->cfg;
	}

	if (aCfg == NULL)
	{
		rSize->w = -2;
		rSize->h = -2;

		eNum = 2;
		goto out;
	} 

	if (img->image != NULL)
	{
		/* this Image contains data */
		imW = ((SDL_Surface *)img->image)->w;
		imH = ((SDL_Surface *)img->image)->h;
	}

	rSize->w = imW;
	rSize->h = imH;
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
 *  Name:          neuik_Element_Render__Image
 *
 *  Description:   Renders a single Image as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * neuik_Element_Render__Image(
	NEUIK_Element    elem, 
	RenderSize     * rSize, /* [in] the size the tex occupies when complete */
	SDL_Renderer   * xRend) /* the external renderer to prepare the texture for */
{
	SDL_Surface             * surf       = NULL;
	SDL_Surface             * imgSurf    = NULL;
	SDL_Renderer            * rend       = NULL;
	SDL_Texture             * imgTex     = NULL; /* image texture */
	SDL_Rect                  rect;
	int                       imW        = 0;
	int                       imH        = 0;
	int                       eNum       = 0; /* which error to report (if any) */
	NEUIK_Image             * img        = NULL;
	NEUIK_ElementBase       * eBase      = NULL;
	// const NEUIK_ImageConfig * aCfg       = NULL; /* the active Image config */
	static char               funcName[] = "neuik_Element_Render__Image";
	static char             * errMsgs[]  = {"",                          // [0] no error
		"Argument `elem` is not of Image class.",                        // [1]
		"Failure in Element_Resize().",                                  // [2]
		"SDL_CreateTextureFromSurface returned NULL.",                   // [3]
		"Invalid specified `rSize` (negative values).",                  // [4]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [5]
	};


	if (!neuik_Object_IsClass(elem, neuik__Class_Image))
	{
		eNum = 1;
		goto out;
	}
	img = (NEUIK_Image *)elem;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w == eBase->eSt.rSizeOld.w  &&
		eBase->eSt.rSize.h == eBase->eSt.rSizeOld.h)
	{
		if (!neuik_Element_NeedsRedraw(img) && eBase->eSt.texture != NULL) 
		{
			(*rSize) = eBase->eSt.rSize;
			return eBase->eSt.texture;
		}
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w != eBase->eSt.rSizeOld.w  ||
		eBase->eSt.rSize.h != eBase->eSt.rSizeOld.h)
	{
		/*--------------------------------------------------------------------*/
		/* This will create a new SDL_Surface & SDL_Renderer; also it will    */
		/* free old ones if they are allocated.                               */
		/*--------------------------------------------------------------------*/
		if (neuik_Element_Resize(img, *rSize) != 0)
		{
			eNum = 2;
			goto out;
		}
	}
	surf    = eBase->eSt.surf;
	rend    = eBase->eSt.rend;
	imgSurf = (SDL_Surface *)(img->image);

	/*------------------------------------------------------------------------*/
	/* select the correct Image config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	// if (img->cfgPtr != NULL)
	// {
	// 	aCfg = img->cfgPtr;
	// }
	// else 
	// {
	// 	aCfg = img->cfg;
	// }

	/*------------------------------------------------------------------------*/
	/* Fill the background of the Image using a transparent color             */
	/*------------------------------------------------------------------------*/
	SDL_SetRenderDrawColor(rend, 255, 255, 255, 0);
	SDL_RenderClear(rend);

	/*------------------------------------------------------------------------*/
	/* Render the Image                                                       */
	/*------------------------------------------------------------------------*/
	if (imgSurf != NULL)
	{
		imW = imgSurf->w;
		imH = imgSurf->h;
		imgTex = SDL_CreateTextureFromSurface(rend, imgSurf);

		switch (eBase->eCfg.HJustify)
		{
			case NEUIK_HJUSTIFY_LEFT:
				rect.x = 0;
				rect.y = (int) ((float)(rSize->h - imH)/2.0);
				break;

			case NEUIK_HJUSTIFY_CENTER:
				rect.x = (int) ((float)(rSize->w - imW)/2.0);
				rect.y = (int) ((float)(rSize->h - imH)/2.0);
				break;

			case NEUIK_HJUSTIFY_RIGHT:
				rect.x = (int) (rSize->w - imW);
				rect.y = (int) ((float)(rSize->h - imH)/2.0);
				break;
		}
		rect.w = imW;
		rect.h = imH;

		SDL_RenderCopy(rend, imgTex, NULL, &rect);
	}

	/*------------------------------------------------------------------------*/
	/* Copy the text onto the renderer and update it                          */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 3;
		goto out;
	}
	eBase->eSt.doRedraw = 0;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	ConditionallyDestroyTexture(&imgTex);

	if (eBase == NULL) return NULL;
	return eBase->eSt.texture;
}
