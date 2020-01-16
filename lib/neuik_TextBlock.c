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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NEUIK_error.h"
#include "neuik_TextBlock.h"
// #include "NEUIK_defs.h"

const unsigned int DefaultBlockSize         = 2048;
const unsigned int DefaultChapterSize       = 10;
const unsigned int DefaultChaptersAllocated = 20;
const double       DefaultBlockFracitonFree = 0.05; /* 5% */

// typedef struct {
// 	unsigned int   firstLineNo;    /* 0 = start of */
// 	unsigned int   nLines;         /* number of actual lines in block */
// 	unsigned int   bytesAllocated; /* max size of text block (number of characters) */
// 	unsigned int   bytesInUse;     /* number of allocated bytes that are currently used */
// 	char         * data;           
// 	void         * previousBlock;  /* NULL = first block */
// 	void         * nextBlock;      /* NULL = last block  */
// } neuik_TextBlockData;

// typedef struct {
// 	unsigned int           blockSize;         /* the number of blocks per chapter */
// 	unsigned int           chapterSize;       /* the number of blocks per chapter */
// 	unsigned int           nDataBlocks;       /* the number of data blocks in the TextBlock */
// 	unsigned int           nLines;             total number of lines in the TextBlock 
// 	unsigned int           nChapters;         /* total number of chapters in the TextBlock */
// 	unsigned int           chaptersAllocated; /* size of allocated chapter array */
// 	neuik_TextBlockData *  firstBlock;
// 	neuik_TextBlockData *  lastBlock;
// 	neuik_TextBlockData ** chapters;      /*  */
// } neuik_TextBlock;

int neuik_NewTextBlockData(
	neuik_TextBlockData ** dataPtr,
	size_t                 blockSize)
{
	int                   eNum       = 0; /* which error to report (if any) */
	neuik_TextBlockData * data       = NULL;
	static char           funcName[] = "neuik_NewTextBlockData";
	static char         * errMsgs[]  = {"",   // [0] no error
		"Output argument `dataPtr` is NULL.", // [1]
		"Failure to allocate memory.",        // [2]
	};

	if (dataPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	(*dataPtr) = (neuik_TextBlockData*) malloc(sizeof(neuik_TextBlockData));
	data = (*dataPtr);
	if (data == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set initial values                                                     */
	/*------------------------------------------------------------------------*/
	data->firstLineNo    = 0;
	data->nLines         = 0;
	data->bytesInUse     = 0;
	data->bytesAllocated = blockSize;
	data->previousBlock  = NULL;
	data->nextBlock      = NULL;

	data->data = (char*) malloc((blockSize+1)*sizeof(char));
	if (data->data == NULL)
	{
		eNum = 2;
		goto out;
	}
	data->data[0] = '\0';
	data->data[blockSize] = '\0';
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

int neuik_NewTextBlock(
	neuik_TextBlock ** tblkPtr,
	size_t             blockSize,
	size_t             chapterSize)
{
	int               eNum       = 0; /* which error to report (if any) */
	neuik_TextBlock * tblk       = NULL;
	static char       funcName[] = "neuik_NewTextBlock";
	static char     * errMsgs[]  = {"",                  // [0] no error
		"Output argument `tblkPtr` is NULL.",            // [1]
		"Failure to allocate memory.",                   // [2]
		"Failure in function `neuik_NewTextBlockData`.", // [3]
	};

	if (tblkPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	(*tblkPtr) = (neuik_TextBlock*) malloc(sizeof(neuik_TextBlock));
	tblk = (*tblkPtr);
	if (tblk == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set initial/default values                                             */
	/*------------------------------------------------------------------------*/
	tblk->blockSize         = DefaultBlockSize;
	tblk->chapterSize       = DefaultChapterSize;
	tblk->nDataBlocks       = 1;
	tblk->length            = 0;
	tblk->nLines            = 1;
	tblk->nChapters         = 1;
	tblk->chaptersAllocated = DefaultChaptersAllocated;

	/*------------------------------------------------------------------------*/
	/* Update TextBlock size and if a non-default value was specified.        */
	/*------------------------------------------------------------------------*/
	if (blockSize > 0)
	{
		/*--------------------------------------------------------------------*/
		/* A non-default TextBlock Size was specified; use that instead       */
		/*--------------------------------------------------------------------*/
		tblk->blockSize = blockSize;
	}

	/*------------------------------------------------------------------------*/
	/* Update ChapterSize and if a non-default value was specified.           */
	/*------------------------------------------------------------------------*/
	if (chapterSize > 0)
	{
		/*--------------------------------------------------------------------*/
		/* A non-default TextBlock Size was specified; use that instead       */
		/*--------------------------------------------------------------------*/
		tblk->chapterSize = chapterSize;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate the first TextBlock.                                          */
	/*------------------------------------------------------------------------*/
	if (neuik_NewTextBlockData(&(tblk->firstBlock), tblk->blockSize))
	{
		eNum = 3;
		goto out;
	}
	tblk->lastBlock = tblk->firstBlock;

	/*------------------------------------------------------------------------*/
	/* Allocate the chapters pointers.                                        */
	/*------------------------------------------------------------------------*/
	tblk->chapters = (neuik_TextBlockData**) malloc(
		DefaultChaptersAllocated*sizeof(neuik_TextBlockData*));
	if (tblk->chapters == NULL)
	{
		eNum = 2;
		goto out;
	}
	tblk->chapters[0] = tblk->firstBlock;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Append an empty data block to the end of a TextBlock.                      */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_AppendDataBlock(
	neuik_TextBlock * tblk)
{
	int                   eNum       = 0; /* which error to report (if any) */
	size_t                nChapters;
	size_t                nChaptersOld;
	neuik_TextBlockData * lastBlock;
	static char           funcName[] = "neuik_TextBlock_AppendDataBlock";
	static char         * errMsgs[]  = {"",              // [0] no error
		"Output argument `tblk` is NULL.",               // [1]
		"Failure in function `neuik_NewTextBlockData`.", // [2]
		"Failure to reallocate memory.",                 // [3]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Go to the current final data block and add one more after that         */
	/*------------------------------------------------------------------------*/
	lastBlock = tblk->lastBlock;
	if (neuik_NewTextBlockData(
		(neuik_TextBlockData **)&(lastBlock->nextBlock), tblk->blockSize))
	{
		eNum = 2;
		goto out;
	}
	lastBlock = lastBlock->nextBlock;
	tblk->lastBlock = lastBlock;

	nChaptersOld = 1 + (tblk->nDataBlocks/tblk->chapterSize);
	tblk->nDataBlocks++;
	nChapters = 1 + (tblk->nDataBlocks/tblk->chapterSize);

	if (nChapters > nChaptersOld)
	{
		/*--------------------------------------------------------------------*/
		/* This new data block is now the start of a new chapter              */
		/*--------------------------------------------------------------------*/
		if (tblk->nChapters >= tblk->chaptersAllocated)
		{
			tblk->chapters = (neuik_TextBlockData**) realloc(tblk->chapters,
				(tblk->chaptersAllocated + DefaultChaptersAllocated)
				* sizeof(neuik_TextBlockData*));
			if (tblk->chapters == NULL)
			{
				eNum = 3;
				goto out;
			}

		}

		tblk->chapters[tblk->nChapters] = lastBlock;
		tblk->nChapters++;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Set the entire contents of a TextBlock.                                    */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_SetText(
	neuik_TextBlock * tblk,
	const char      * text)
{
	size_t                dataLen;
	size_t                textLen;
	neuik_TextBlockData * aBlock;
	size_t                charCtr;
	size_t                writeCtr         = 0;
	size_t                lineCtr          = 1;
	size_t                nBlocksRequried  = 0;
	unsigned int          maxInitBlockFill = (unsigned int)(
		(1.0 - DefaultBlockFracitonFree) * (double)(DefaultBlockSize));
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_SetText";
	static char         * errMsgs[]  = {"",                       // [0] no error
		"Output argument `tblk` is NULL.",                        // [1]
		"Argument `text` is NULL.",                               // [2]
		"Failure in function `neuik_TextBlock_AppendDataBlock`.", // [3]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (text == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the number of data blocks required to load this text.        */
	/*------------------------------------------------------------------------*/
	dataLen = strlen(text);
	textLen = dataLen;

	/*------------------------------------------------------------------------*/
	/* Count the number of (`\r`,`\n`, and `\r\n`) combos in the text and add */
	/* in the extra required space for line terminating `\0` chars.           */
	/*------------------------------------------------------------------------*/
	if (textLen > 1)
	{
		for (charCtr = 1; charCtr < textLen; charCtr++)
		{
			if (text[charCtr-1] == '\r' && text[charCtr-1] == '\n')
			{
				/*------------------------------------------------------------*/
				/* This is a CR-LF line ending (m$-windows-style)             */
				/*------------------------------------------------------------*/
				lineCtr++;
				charCtr++;
			}
			else if (text[charCtr-1] == '\r')
			{
				/*------------------------------------------------------------*/
				/* This is a CR line ending (macos-style)                     */
				/*------------------------------------------------------------*/
				lineCtr++;
			}
			else if (text[charCtr-1] == '\n')
			{
				/*------------------------------------------------------------*/
				/* This is a LF line ending (linux-style)                     */
				/*------------------------------------------------------------*/
				lineCtr++;
			}
		}
		if (text[charCtr] == '\r' || text[charCtr] == '\n')
		{
			/*----------------------------------------------------------------*/
			/* This would be a final single character trailing newline        */
			/*----------------------------------------------------------------*/
			lineCtr++;
		}
	}
	textLen += lineCtr;

	nBlocksRequried = textLen/maxInitBlockFill;
	if (textLen % maxInitBlockFill > 0)
	{
		nBlocksRequried++;
	}

	/*------------------------------------------------------------------------*/
	/* If not enough data blocks are allocated, allocate more now.            */
	/*------------------------------------------------------------------------*/
	if (nBlocksRequried > tblk->nDataBlocks)
	{
		while (tblk->nDataBlocks < nBlocksRequried)
		{
			if (neuik_TextBlock_AppendDataBlock(tblk))
			{
				eNum = 3;
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* If too many data blocks are allocated, free some now.                  */
	/*------------------------------------------------------------------------*/
	if (nBlocksRequried < tblk->nDataBlocks)
	{
		#pragma message("[TODO]: `neuik_TextBlock_SetText` Implement Data Block trim")
	}

	/*------------------------------------------------------------------------*/
	/* Begin copying over data into the data blocks.                          */
	/*------------------------------------------------------------------------*/
	if (nBlocksRequried == 1)
	{
		aBlock = tblk->firstBlock;

		if (textLen > 1)
		{
			/*----------------------------------------------------------------*/
			/* Copying over data one byte at a time taking special care to    */
			/* include a `\0` character after each line ending sequence.      */
			/*----------------------------------------------------------------*/
			for (charCtr = 1; charCtr < textLen; charCtr++)
			{
				if (text[charCtr-1] == '\r' && text[charCtr-1] == '\n')
				{
					/*------------------------------------------------------------*/
					/* This is a CR-LF line ending (m$-windows-style)             */
					/*------------------------------------------------------------*/
					charCtr++;
					aBlock->data[writeCtr++] = '\r';
					aBlock->data[writeCtr++] = '\n';
					aBlock->data[writeCtr++] = '\0';
				}
				else if (text[charCtr-1] == '\r')
				{
					/*------------------------------------------------------------*/
					/* This is a CR line ending (macos-style)                     */
					/*------------------------------------------------------------*/
					aBlock->data[writeCtr++] = '\r';
					aBlock->data[writeCtr++] = '\0';
				}
				else if (text[charCtr-1] == '\n')
				{
					/*------------------------------------------------------------*/
					/* This is a LF line ending (linux-style)                     */
					/*------------------------------------------------------------*/
					aBlock->data[writeCtr++] = '\n';
					aBlock->data[writeCtr++] = '\0';
				}
				else
				{
					aBlock->data[writeCtr++] = text[charCtr-1];
				}
			}
			/*----------------------------------------------------------------*/
			/* This would be a final single trailing character                */
			/*----------------------------------------------------------------*/
			aBlock->data[writeCtr++] = text[charCtr];
		}

		aBlock->bytesInUse = textLen;
		tblk->nLines       = lineCtr;
		aBlock->nLines     = lineCtr;
	}
	else
	{
		#pragma message("[TODO]: `neuik_TextBlock_SetText` Copy Data into multiple blocks.")
	}
	tblk->length = dataLen;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

int neuik_TextBlock_GetLength(
	neuik_TextBlock * tblk,
	size_t          * length)
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_TextBlock_GetLength";
	static char * errMsgs[]  = {"", // [0] no error
		"Argument `tblk` is NULL.",          // [1]
		"Output argument `length` is NULL.", // [2]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (length == NULL)
	{
		eNum = 2;
		goto out;
	}

	*length = tblk->length;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


int neuik_TextBlock_GetLineCount(
	neuik_TextBlock * tblk,
	size_t          * nLines)
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_TextBlock_GetLineCount";
	static char * errMsgs[]  = {"", // [0] no error
		"Argument `tblk` is NULL.",          // [1]
		"Output argument `nLines` is NULL.", // [2]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (nLines == NULL)
	{
		eNum = 2;
		goto out;
	}

	*nLines = tblk->nLines;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


/*----------------------------------------------------------------------------*/
/* Locate the position within a TextBlockData where a desired line starts.    */
/*                                                                            */
/* Returns : 1 if the call failed; 0 if successful                            */
/*                                                                            */
/* [noErrChecks] :                                                            */
/*    This function does not check for NULL pointer arguments. Appropriate    */
/*    checks should be done externally before calling this function.          */
/*----------------------------------------------------------------------------*/
int neuik_TextBlockData_GetLineStartOffset__noErrChecks (
	neuik_TextBlockData * data,
	size_t                lineNo,
	size_t              * offset)
{
	size_t position;
	size_t final;
	size_t lineCtr;
	int    hasErr  = 0;

	if (data->bytesInUse > data->bytesAllocated)
	{
		hasErr = 1;
		goto out;
	}

	position = 0;
	final    = data->bytesInUse;
	lineCtr  = data->firstLineNo;

	if (lineCtr == lineNo)
	{
		/*--------------------------------------------------------------------*/
		/* The desired line starts at the beginning of this data block.       */
		/*--------------------------------------------------------------------*/
		*offset = position;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Now iterate through the data and count instances of `\0` chars.        */
	/* Once lineCtr == lineNo, we have found the start of the desired line.   */
	/*------------------------------------------------------------------------*/
	for (position = 1; position <= final; position++)
	{
		if (data->data[position-1] == '\0')
		{
			lineCtr++;
		}
		if (lineCtr == lineNo)
		{
			/*----------------------------------------------------------------*/
			/* This is the start of the desired line                          */
			/*----------------------------------------------------------------*/
			*offset = position;
			break;
		}
	}
out:
	return hasErr;
}

/*----------------------------------------------------------------------------*/
/* Locate the data block which contains the start of the desired line         */
/*                                                                            */
/* Returns : 1 if the call failed; 0 if successful                            */
/*                                                                            */
/* [noErrChecks] :                                                            */
/*    This function does not check for NULL pointer arguments. Appropriate    */
/*    checks should be done externally before calling this function.          */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetLineStartBlockData__noErrChecks (
	neuik_TextBlock      * tblk,
	size_t                 lineNo,
	neuik_TextBlockData ** blockPtr)
{
	int                   hasErr   = 0;
	unsigned int          chapter;
	neuik_TextBlockData * block    = NULL;

	/*------------------------------------------------------------------------*/
	/* Iterate over the chapters till either the first line number of a       */
	/* chapter exceeds the line that we are looking for or, we reach the      */
	/* final chapter.                                                         */
	/*------------------------------------------------------------------------*/
	for (chapter = 0; chapter < tblk->nChapters; chapter++)
	{
		block = tblk->chapters[chapter];

		if (block->firstLineNo > lineNo)
		{
			/*----------------------------------------------------------------*/
			/* This means that we have gone too far... walk back to the       */
			/* previous chapter.                                              */
			/*----------------------------------------------------------------*/
			block = tblk->chapters[chapter-1];
			break;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Now check one block at a time looking for the block that contains the  */
	/* start of the desired line.                                             */
	/*------------------------------------------------------------------------*/
	for (;;)
	{
		if (block == NULL)
		{
			hasErr = 1;
			goto out;
		}

		if (block->firstLineNo <= lineNo && 
			lineNo <= block->firstLineNo + block->nLines)
		{
			/*----------------------------------------------------------------*/
			/* This block contains the start of the desired line              */
			/*----------------------------------------------------------------*/
			break;
		}
		block = block->nextBlock;
	}
out:
	*blockPtr = block;
	return hasErr;
}

/*----------------------------------------------------------------------------*/
/* Locate the data block and offset that points to the start of the desired   */
/* line.                                                                      */
/*                                                                            */
/* Returns : 1 if the call failed; 0 if successful                            */
/*                                                                            */
/* [noErrChecks] :                                                            */
/*    This function does not check for NULL pointer arguments. Appropriate    */
/*    checks should be done externally before calling this function.          */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetPositionLineStart__noErrChecks (
	neuik_TextBlock      * tblk,
	size_t                 lineNo,
	neuik_TextBlockData ** blockPtr,
	size_t               * offset)
{
	int hasErr = 0;

	/*------------------------------------------------------------------------*/
	/* Locate the block which contains the start of the desired line          */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineStartBlockData__noErrChecks(tblk, lineNo, blockPtr))
	{
		hasErr = 1;
		goto out;
	}
	/*------------------------------------------------------------------------*/
	/* Get the position of the start of the line in that data block.          */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlockData_GetLineStartOffset__noErrChecks(*blockPtr, lineNo, offset))
	{
		hasErr = 1;
	}
out:
	return hasErr;
}


/*----------------------------------------------------------------------------*/
/* Locate the data block and offset that points to the the desired character  */
/* number of the desired line.                                                */
/* line.                                                                      */
/*                                                                            */
/* Returns : 1 if the call failed; 0 if successful                            */
/*                                                                            */
/* [noErrChecks] :                                                            */
/*    This function does not check for NULL pointer arguments. Appropriate    */
/*    checks should be done externally before calling this function.          */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetPositionInLine__noErrChecks (
	neuik_TextBlock      * tblk,
	size_t                 lineNo,
	size_t                 byteNo,
	neuik_TextBlockData ** blockPtr,
	size_t               * offset)
{
	int                   hasErr   = 0;
	size_t                byteCtr  = 0;
	size_t                position = 0;
	size_t                final    = 0;
	neuik_TextBlockData * data     = NULL;
	neuik_TextBlockData * block    = NULL;

	/*------------------------------------------------------------------------*/
	/* Locate the block which contains the start of the desired line and the  */
	/* position of the start of the line in that data block.                  */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetPositionLineStart__noErrChecks(tblk, 
		lineNo, &block, offset))
	{
		hasErr = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start moving from the starting point of the line.                      */
	/*------------------------------------------------------------------------*/
	data     = block;
	position = *offset;

	/*------------------------------------------------------------------------*/
	/* Start walking through the data block(s) until the end of the line or   */
	/* the desired index point is found.                                      */
	/*------------------------------------------------------------------------*/
	for (;;)
	{
		/*--------------------------------------------------------------------*/
		/* This loop marks the point where a data block is entered.           */
		/*--------------------------------------------------------------------*/
		if (data->bytesInUse > data->bytesAllocated)
		{
			hasErr = 1;
			goto out;
		}

		final = data->bytesInUse;

		/*--------------------------------------------------------------------*/
		/* Now iterate through the data and count instances of newline chars. */
		/* Once lineCtr == lineNo, we have found the start of the desired     */
		/* line.                                                              */
		/*--------------------------------------------------------------------*/
		for (; position <= final; position++)
		{
			if (byteCtr == byteNo)
			{
				/*------------------------------------------------------------*/
				/* This is the exact location that we were looking for. Set   */
				/* the return values and return.                              */
				/*------------------------------------------------------------*/
				*blockPtr = data;
				*offset   = position;
				goto out;
			}
			if (data->data[position] == '\0')
			{
				/*------------------------------------------------------------*/
				/* This is the end of the line but we should have reached the */
				/* byte index before we found this.                           */
				/*------------------------------------------------------------*/
				hasErr = 1;
				goto out;
			}
			byteCtr++;
		}

		data = data->nextBlock;
		if (data == NULL)
		{
			/*----------------------------------------------------------------*/
			/* This means that this line was the final line in the TextBlock. */
			/*----------------------------------------------------------------*/
			goto out;
		}

		position = 0;
	}
out:
	return hasErr;
}

/*----------------------------------------------------------------------------*/
/* Check to see if a line number is contained by a TextBlock                  */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_HasLine(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	int             * hasLine) /* [out] set to 1 if it has the line; else 0 */
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_TextBlock_HasLine";
	static char * errMsgs[]  = {"",           // [0] no error
		"Argument `tblk` is NULL.",           // [1]
		"Output argument `hasLine` is NULL.", // [2]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (hasLine == NULL)
	{
		eNum = 2;
		goto out;
	}

	if (tblk->nLines >= lineNo)
	{
		*hasLine = 1;
	}
	else
	{
		*hasLine = 0;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Get the length of a line of text contained in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetLineLength__noErrChecks(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	size_t          * length)
{
	int                   hasErr   = 0;
	size_t                offset   = 0;
	size_t                position = 0;
	size_t                final    = 0;
	neuik_TextBlockData * data     = NULL;
	neuik_TextBlockData * block    = NULL;

	(*length) = 0;

	/*------------------------------------------------------------------------*/
	/* Locate the block which contains the start of the desired line and the  */
	/* position of the start of the line in that data block.                  */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetPositionLineStart__noErrChecks(tblk, 
		lineNo, &block, &offset))
	{
		hasErr = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start searching from the starting point of the line.                   */
	/*------------------------------------------------------------------------*/
	data     = block;
	position = offset;

	/*------------------------------------------------------------------------*/
	/* Start walking through the data block(s) until the end of the line or   */
	/* the end of the TextBlock data is found.                                */
	/*------------------------------------------------------------------------*/
	for (;;)
	{
		/*--------------------------------------------------------------------*/
		/* This loop marks the point where a data block is entered.           */
		/*--------------------------------------------------------------------*/
		if (data->bytesInUse > data->bytesAllocated)
		{
			hasErr = 1;
			goto out;
		}

		final = data->bytesInUse;

		/*--------------------------------------------------------------------*/
		/* Now iterate through the data and count instances of newline chars. */
		/* Once lineCtr == lineNo, we have found the start of the desired     */
		/* line.                                                              */
		/*--------------------------------------------------------------------*/
		for (; position < final; position++)
		{
			(*length)++;
			if (data->data[position] == '\n' || data->data[position] == '\r')
			{
				/*------------------------------------------------------------*/
				/* This is the end of the line that we were looking for.      */
				/* Don't include the line break/newline characters in the     */
				/* line length.                                               */
				/*------------------------------------------------------------*/
				if (*length > 0) (*length)--;
				goto out;
			}
		}

		data = data->nextBlock;
		if (data == NULL)
		{
			/*----------------------------------------------------------------*/
			/* This means that this line was the final line in the TextBlock. */
			/*----------------------------------------------------------------*/
			goto out;
		}

		position = 0;
	}
out:
	return hasErr;
}

/*----------------------------------------------------------------------------*/
/* Get the length of a line of text contained in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetLineLength(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	size_t          * length)
{
	int               eNum       = 0; /* which error to report (if any) */
	static char       funcName[] = "neuik_TextBlock_GetLineLength";
	static char     * errMsgs[]  = {"",                          // [0] no error
		"Argument `tblk` is NULL.",                              // [1]
		"Output argument `length` is NULL.",                     // [2]
		"Requested Line not in TextBlock.",                      // [3]
		"Fundamental error in basic function `GetLineLength` .", // [4]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (length == NULL)
	{
		eNum = 2;
		goto out;
	}
	if (lineNo > tblk->nLines)
	{
		eNum = 3;
		goto out;
	}

	if (neuik_TextBlock_GetLineLength__noErrChecks(tblk, lineNo, length))
	{
		eNum = 4;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Get a copy of the text contained by a line in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetLine(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	char           ** lineData)
{
	int                   eNum     = 0; /* which error to report (if any) */
	char                * writeStr = NULL;
	size_t                length;
	size_t                position = 0;
	size_t                writePos = 0; /* character index to write to */
	size_t                final    = 0;
	neuik_TextBlockData * data     = NULL;
	static char           funcName[] = "neuik_TextBlock_GetLine";
	static char         * errMsgs[]  = {"",                       // [0] no error
		"Argument `tblk` is NULL.",                               // [1]
		"Output argument `lineData` is NULL.",                    // [2]
		"Requested Line not in TextBlock.",                       // [3]
		"Call to `neuik_TextBlock_GetLineLength` failed.",        // [4]
		"Failure to allocate memory.",                            // [5]
		"Fundamental error in basic function `GetLineLength`.",   // [6]
		"Malformed TextBlockData (bytesInUse > bytesAllocated).", // [7]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (lineData == NULL)
	{
		eNum = 2;
		goto out;
	}
	if (lineNo > tblk->nLines)
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Get the total length of the line and allocate enough memory to extract */
	/* it.                                                                    */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &length))
	{
		eNum = 4;
		goto out;
	}

	(*lineData) = (char *)malloc((length+1)*sizeof(char));
	writeStr = *lineData;
	if (writeStr == NULL)
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Locate the block which contains the start of the desired line and the  */
	/* position of the start of the line in that data block.                  */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetPositionLineStart__noErrChecks(tblk, 
		lineNo, &data, &position))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start walking through the data block(s) until the end of the line or   */
	/* the end of the TextBlock data is found.                                */
	/*------------------------------------------------------------------------*/
	for (;;)
	{
		/*--------------------------------------------------------------------*/
		/* This loop marks the point where a data block is entered.           */
		/*--------------------------------------------------------------------*/
		if (data->bytesInUse > data->bytesAllocated)
		{
			eNum = 7;
			goto out;
		}

		final = data->bytesInUse;

		/*--------------------------------------------------------------------*/
		/* Now iterate through the data and count instances of newline chars. */
		/* Once lineCtr == lineNo, we have found the start of the desired     */
		/* line.                                                              */
		/*--------------------------------------------------------------------*/
		for (; position < final; position++)
		{
			if (data->data[position] == '\n' || data->data[position] == '\r')
			{
				/*------------------------------------------------------------*/
				/* This is the end of the line that we were looking for.      */
				/*------------------------------------------------------------*/
				writeStr[writePos++] = '\0';
				goto out;
			}
			writeStr[writePos++] = data->data[position];
		}

		data = data->nextBlock;
		if (data == NULL)
		{
			/*----------------------------------------------------------------*/
			/* This means that this line was the final line in the TextBlock. */
			/*----------------------------------------------------------------*/
			writeStr[writePos++] = '\0';
			goto out;
		}

		position = 0;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Get a copy of the specified textSection from a TextBlock                   */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_GetSection(
	neuik_TextBlock  * tblk,
	size_t             startLineNo,
	size_t             startLinePos,
	size_t             endLineNo,
	size_t             endLinePos,
	char            ** secData)
{
	neuik_TextBlockData * startBlock;
	neuik_TextBlockData * endBlock;
	size_t                copyCtr;
	size_t                writeCtr;
	size_t                copySize;
	size_t                startLineLen;
	size_t                endLineLen;
	size_t                startPosition;
	size_t                endPosition;
	char                  copyChar;
	char                * writeStr = NULL;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_GetSection";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Argument `startLineNo` has value in excess of line length.",  // [3]
		"Argument `endLineNo` has value in excess of line length.",    // [4]
		"Fundamental error in basic function `GetPositionInLine`.",    // [5]
		"Output argument `secData` is NULL.",                          // [6]
		"Requested Line not in TextBlock.",                            // [7]
		"Failure to allocate memory.",                                 // [8]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (secData == NULL)
	{
		eNum = 6;
		goto out;
	}
	if (startLineNo > tblk->nLines || endLineNo > tblk->nLines)
	{
		eNum = 7;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we weren't given an impossible start or end location.        */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, startLineNo, &startLineLen))
	{
		eNum = 2;
		goto out;
	}
	if (startLinePos > startLineLen)
	{
		eNum = 3;
		goto out;
	}
	if (neuik_TextBlock_GetLineLength(tblk, endLineNo, &endLineLen))
	{
		eNum = 2;
		goto out;
	}
	if (endLinePos > endLineLen)
	{
		eNum = 4;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, startLineNo, startLinePos, &startBlock, &startPosition))
	{
		eNum = 5;
		goto out;
	}
	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, endLineNo, endLinePos, &endBlock, &endPosition))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the overall size of the string that is to be returned        */
	/*------------------------------------------------------------------------*/
	if (startBlock == endBlock)
	{
		/*--------------------------------------------------------------------*/
		/* The section being copied is all contained within a single block    */
		/*--------------------------------------------------------------------*/
		if (startLineNo == endLineNo && startLinePos > endLinePos)
		{
			/*----------------------------------------------------------------*/
			/* Nothing is selected, there is nothing to be done.              */
			/*----------------------------------------------------------------*/
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* The section being copied is one or more characters                 */
		/*--------------------------------------------------------------------*/
		copySize = endPosition - startPosition;

		(*secData) = (char *)malloc((copySize+1)*sizeof(char));
		writeStr = *secData;
		if (writeStr == NULL)
		{
			eNum = 8;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Copy over the data one byte at a time                              */
		/*--------------------------------------------------------------------*/
		writeCtr = 0;
		for (copyCtr = startPosition; copyCtr < endPosition; copyCtr++)
		{
			copyChar = startBlock->data[copyCtr];
			if (copyChar != '\0')
			{
				writeStr[writeCtr++] = copyChar;
			}
		}
		writeStr[writeCtr++] = '\0';
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* The section being deleted spans more than one block                */
		/*--------------------------------------------------------------------*/
		#pragma message("[TODO]: `neuik_TextBlock_GetSection` Get over multiple blocks.")
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


/*----------------------------------------------------------------------------*/
/* Replace an actual line of data with another                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_ReplaceLine(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Delete the specified line number                                           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteLine(
			neuik_TextBlock * tblk,
			size_t            lineNo);

/*----------------------------------------------------------------------------*/
/* Insert a line before the specified line number                             */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_InsertLine(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Insert a line after the specified line number                              */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_InsertLineAfter(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Insert a character at the specified position                               */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_InsertChar(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	size_t            byteNo,
	char              newChar)
{
	neuik_TextBlockData * aBlock;
	size_t                copyCtr;
	size_t                lineLen;
	size_t                lineBreakByte = 0;
	size_t                position;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_InsertChar";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Argument `byteNo` has value in excess of line length.",       // [3]
		"Fundamental error in basic function `GetPositionLineStart`.", // [4]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we aren't attempting to insert a character into a line at a  */
	/* position that is outside of its scope.                                 */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
	{
		eNum = 2;
		goto out;
	}
	if (byteNo > lineLen)
	{
		if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
		{
			eNum = 2;
			goto out;
		}
		eNum = 3;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, lineNo, byteNo, &aBlock, &position))
	{
		eNum = 4;
		goto out;
	}

	if (newChar == '\n')
	{
		lineBreakByte = 1;
	}

	/*------------------------------------------------------------------------*/
	/* It is likely that line data will need to be shifted over to accomodate */
	/* this new character.                                                    */
	/*------------------------------------------------------------------------*/
	if (aBlock->bytesInUse + lineBreakByte < aBlock->bytesAllocated)
	{
		/*--------------------------------------------------------------------*/
		/* Simply shift over the bytes by one                                 */
		/*--------------------------------------------------------------------*/
		aBlock->data[1 + aBlock->bytesInUse] = '\0';
		for (copyCtr = aBlock->bytesInUse; copyCtr > position; copyCtr--)
		{
			aBlock->data[copyCtr] = aBlock->data[copyCtr-1];
		}
		aBlock->data[position] = newChar;
		aBlock->bytesInUse++;

		if (newChar == '\n')
		{
			/*----------------------------------------------------------------*/
			/* Shift over the bytes by one more character                     */
			/*----------------------------------------------------------------*/
			aBlock->data[1 + aBlock->bytesInUse] = '\0';
			for (copyCtr = aBlock->bytesInUse; copyCtr > position; copyCtr--)
			{
				aBlock->data[copyCtr] = aBlock->data[copyCtr-1];
			}
			aBlock->data[position+1] = '\0';
			aBlock->bytesInUse++;
		}
	}
	else
	{
		#pragma message("[TODO] `neuik_TextBlock_InsertChar` Complex byte Shift")
	}

	if (newChar == '\n')
	{
		tblk->nLines++;
		aBlock->nLines++;
	}
	// printf("neuik_TextBlock_InsertChar(): [%u:%u] `%s`\n", 
	// 	lineNo, byteNo, aBlock->data);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Insert a string of characters at the specified position                    */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_InsertText(
	neuik_TextBlock * tblk,
	size_t            lineNo,       /* line in which to insert text */
	size_t            linePos,      /* position within the line to insert */
	const char      * text,         /* text section to be inserted */
	size_t          * finalLineNo,  /* line where resulting insert completed */
	size_t          * finalLinePos) /* line position where insert completed */
{
	neuik_TextBlockData * aBlock;
	size_t                textLen;
	size_t                copyCtr;
	size_t                charCtr;
	size_t                lineLen;
	size_t                startPosition;
	size_t                startOfCopy;
	size_t                endOfCopy;
	size_t                writeCtr         = 0;
	size_t                posCtr           = 0;
	size_t                lineCtr          = 0;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_InsertText";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Argument `text` is NULL.",                                    // [2]
		"Failure in function `neuik_TextBlock_AppendDataBlock`.",      // [3]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [4]
		"Argument `linePos` has value in excess of line length.",      // [5]
		"Fundamental error in basic function `GetPositionLineStart`.", // [6]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}
	if (text == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we aren't attempting to insert text into a line at a         */
	/* position that is outside of its scope.                                 */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
	{
		eNum = 4;
		goto out;
	}
	if (linePos > lineLen)
	{
		eNum = 5;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, lineNo, linePos, &aBlock, &startPosition))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the length of the text to insert.                            */
	/*------------------------------------------------------------------------*/
	textLen = strlen(text);
	if (textLen == 0)
	{
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Count the number of (`\r`,`\n`, and `\r\n`) combos in the text and add */
	/* in the extra required space for line terminating `\0` chars.           */
	/*------------------------------------------------------------------------*/
	if (textLen > 1)
	{
		for (charCtr = 1; charCtr < textLen; charCtr++)
		{
			if (text[charCtr-1] == '\r' && text[charCtr-1] == '\n')
			{
				/*------------------------------------------------------------*/
				/* This is a CR-LF line ending (m$-windows-style)             */
				/*------------------------------------------------------------*/
				lineCtr++;
				charCtr++;
				posCtr = 0;
			}
			else if (text[charCtr-1] == '\r')
			{
				/*------------------------------------------------------------*/
				/* This is a CR line ending (macos-style)                     */
				/*------------------------------------------------------------*/
				lineCtr++;
				posCtr = 0;
			}
			else if (text[charCtr-1] == '\n')
			{
				/*------------------------------------------------------------*/
				/* This is a LF line ending (linux-style)                     */
				/*------------------------------------------------------------*/
				lineCtr++;
				posCtr = 0;
			}
			else
			{
				posCtr++;
			}
		}
		if (text[charCtr] == '\r' || text[charCtr] == '\n')
		{
			/*----------------------------------------------------------------*/
			/* This would be a final single character trailing newline        */
			/*----------------------------------------------------------------*/
			lineCtr++;
			posCtr = 0;
		}
		else
		{
			posCtr++;
		}
	}
	else if (textLen == 1)
	{
		if (text[0] == '\n' || text[0] == '\r')
		{
			lineCtr++;
			posCtr = 0;
		}
		else
		{
			posCtr++;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Determine if the receiving data block has enough extra space to hold   */
	/* the addtional text in its entirety.                                    */
	/*------------------------------------------------------------------------*/
	if (aBlock->bytesInUse + (textLen + lineCtr) < aBlock->bytesAllocated)
	{
		startOfCopy = (aBlock->bytesInUse - 1) + (textLen + lineCtr); 
		endOfCopy   = startPosition + (textLen + lineCtr);

		/*--------------------------------------------------------------------*/
		/* Shift over the existing data to make room for the new text         */
		/*--------------------------------------------------------------------*/
		for (copyCtr = startOfCopy; copyCtr >= endOfCopy; copyCtr--)
		{
			aBlock->data[copyCtr] = aBlock->data[copyCtr - (textLen + lineCtr)];
		}
		aBlock->bytesInUse += (textLen + lineCtr);
		aBlock->data[aBlock->bytesInUse] = '\0';


		/*--------------------------------------------------------------------*/
		/* Copy over the new text into the desired location                   */
		/*--------------------------------------------------------------------*/
		if (textLen > 1)
		{
			/*----------------------------------------------------------------*/
			/* Copying over data one byte at a time taking special care to    */
			/* include a `\0` character after each line ending sequence.      */
			/*----------------------------------------------------------------*/
			writeCtr = startPosition;
			for (charCtr = 1; charCtr <= textLen; charCtr++)
			{
				if (text[charCtr-1] == '\r' && text[charCtr] == '\n')
				{
					/*--------------------------------------------------------*/
					/* This is a CR-LF line ending (m$-windows-style)         */
					/*--------------------------------------------------------*/
					charCtr++;
					aBlock->data[writeCtr++] = '\r';
					aBlock->data[writeCtr++] = '\n';
					aBlock->data[writeCtr++] = '\0';
					tblk->nLines++;
					aBlock->nLines++;
				}
				else if (text[charCtr-1] == '\r')
				{
					/*--------------------------------------------------------*/
					/* This is a CR line ending (macos-style)                 */
					/*--------------------------------------------------------*/
					aBlock->data[writeCtr++] = '\r';
					aBlock->data[writeCtr++] = '\0';
					tblk->nLines++;
					aBlock->nLines++;
				}
				else if (text[charCtr-1] == '\n')
				{
					/*--------------------------------------------------------*/
					/* This is a LF line ending (linux-style)                 */
					/*--------------------------------------------------------*/
					aBlock->data[writeCtr++] = '\n';
					aBlock->data[writeCtr++] = '\0';
					tblk->nLines++;
					aBlock->nLines++;
				}
				else
				{

					aBlock->data[writeCtr++] = text[charCtr-1];
				}
			}
		}
		else if (textLen == 1)
		{
			aBlock->data[startPosition] = text[0];
			if (text[0] == '\n' || text[0] == '\r')
			{
				aBlock->data[startPosition + 1] = '\0';
				tblk->nLines++;
				aBlock->nLines++;
			}
		}
	}
	else
	{
		#pragma message("[TODO]: `neuik_TextBlock_InsertText` Insert Text into multiple blocks.")
	}

	/*------------------------------------------------------------------------*/
	/* Return the position immediately after the end of the text insertion    */
	/*------------------------------------------------------------------------*/
	if (lineCtr > 0)
	{
		*finalLineNo  = lineNo + lineCtr;
		*finalLinePos = posCtr;
	}
	else
	{
		*finalLinePos = linePos + posCtr;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


/*----------------------------------------------------------------------------*/
/* Delete a character at a position                                           */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_DeleteChar(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	size_t            byteNo)
{
	neuik_TextBlockData * aBlock;
	size_t                copyCtr;
	size_t                lineLen;
	size_t                lineBreakByte = 0;
	size_t                position;
	char                  remChar;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_DeleteChar";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Argument `byteNo` has value in excess of line length.",       // [3]
		"Fundamental error in basic function `GetPositionLineStart`.", // [4]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we aren't attempting to remove a character into a line at a  */
	/* position that is outside of its scope.                                 */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
	{
		eNum = 2;
		goto out;
	}
	if (byteNo > lineLen)
	{
		eNum = 3;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, lineNo, byteNo, &aBlock, &position))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* It is likely that line data will need to be shifted over to accomodate */
	/* this new character.                                                    */
	/*------------------------------------------------------------------------*/
	if (aBlock->bytesInUse + lineBreakByte < aBlock->bytesAllocated)
	{
		/*--------------------------------------------------------------------*/
		/* Store the value of the deleted character.                          */
		/*--------------------------------------------------------------------*/
		remChar = aBlock->data[position];

		/*--------------------------------------------------------------------*/
		/* Simply shift over the bytes by one                                 */
		/*--------------------------------------------------------------------*/
		for (copyCtr = position; copyCtr < aBlock->bytesInUse; copyCtr++)
		{
			aBlock->data[copyCtr] = aBlock->data[copyCtr+1];
		}
		aBlock->data[aBlock->bytesInUse] = '\0';
		aBlock->bytesInUse--;

		if (remChar == '\0')
		{
			/*----------------------------------------------------------------*/
			/* Shift over the bytes by one more character                     */
			/*----------------------------------------------------------------*/
			tblk->nLines--;
			aBlock->nLines--;
		}
	}
	// printf("neuik_TextBlock_InsertChar(): [%u:%u] `%s`\n", 
	// 	lineNo, byteNo, aBlock->data);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}


/*----------------------------------------------------------------------------*/
/* Effectively deletes the line ending of the specified line and tacks on the */
/* contents of the following line to the end of the specified line.           */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_MergeLines(
	neuik_TextBlock * tblk,
	size_t            lineNo)
{
	neuik_TextBlockData * aBlock;
	neuik_TextBlockData * aBlock2;
	size_t                lineLen;
	size_t                position;
	size_t                position2;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_MergeLines";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Fundamental error in basic function `GetPositionInLine`.",    // [3]
		"Fundamental error in basic function `GetPositionLineStart`.", // [4]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure that the user isn't attempting to merge the final line of    */
	/* the text block.                                                        */
	/*------------------------------------------------------------------------*/
	if (lineNo == tblk->nLines)
	{
		/*--------------------------------------------------------------------*/
		/* The final line of text data cannot be merged. Do nothing.          */
		/*--------------------------------------------------------------------*/
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Get the data block and position within the textblock of the final      */
	/* character of the specified line.                                       */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
	{
		eNum = 2;
		goto out;
	}
	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, lineNo, lineLen, &aBlock, &position))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the where the data of the subsequent line begins.            */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetPositionLineStart__noErrChecks(
			tblk, lineNo+1, &aBlock2, &position2))
	{
		eNum = 4;
		goto out;
	}

	if (aBlock == aBlock2)
	{
		/*--------------------------------------------------------------------*/
		/* The end of the first line and the start of the second line are     */
		/* both within the same data block.                                   */
		/*--------------------------------------------------------------------*/
		for (;position2 < aBlock->bytesInUse;)
		{
			aBlock->data[position] = aBlock->data[position2];
			position++;
			position2++;
		}
		aBlock->bytesInUse -= (position2 - position);
		aBlock->nLines--;
		tblk->nLines--;
	}
	else
	{
		#pragma message("[TODO] `neuik_TextBlock_MergeLines` OutOfBlock Merge")
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Delete a section of data                                                   */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_DeleteSection(
	neuik_TextBlock * tblk,
	size_t            startLineNo,
	size_t            startLinePos,
	size_t            endLineNo,
	size_t            endLinePos)
{
	neuik_TextBlockData * startBlock;
	neuik_TextBlockData * endBlock;
	size_t                checkCtr;
	size_t                copyCtr;
	size_t                copyOffset;
	size_t                endOfCopy;
	size_t                startLineLen;
	size_t                endLineLen;
	size_t                startPosition;
	size_t                endPosition;
	char                  remChar;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_DeleteSection";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Argument `startLineNo` has value in excess of line length.",  // [3]
		"Argument `endLineNo` has value in excess of line length.",    // [4]
		"Fundamental error in basic function `GetPositionInLine`.",    // [5]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we aren't attempting to remove a character into a line at a  */
	/* position that is outside of its scope.                                 */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, startLineNo, &startLineLen))
	{
		eNum = 2;
		goto out;
	}
	if (startLinePos > startLineLen)
	{
		eNum = 3;
		goto out;
	}
	if (neuik_TextBlock_GetLineLength(tblk, endLineNo, &endLineLen))
	{
		eNum = 2;
		goto out;
	}
	if (endLinePos > endLineLen)
	{
		eNum = 4;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, startLineNo, startLinePos, &startBlock, &startPosition))
	{
		eNum = 5;
		goto out;
	}
	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, endLineNo, endLinePos, &endBlock, &endPosition))
	{
		eNum = 5;
		goto out;
	}

	printf("Deleting Section from [%u:%u] to [%u:%u]\n",
		(unsigned int)(startLineNo), 
		(unsigned int)(startLinePos), 
		(unsigned int)(endLineNo), 
		(unsigned int)(endLinePos));

	if (startBlock == endBlock)
	{
		/*--------------------------------------------------------------------*/
		/* The section being deleted is all contained within a single block   */
		/*--------------------------------------------------------------------*/
		if (startLineNo == endLineNo && startLinePos > endLinePos)
		{
			/*----------------------------------------------------------------*/
			/* Nothing is selected, there is nothing to be done.              */
			/*----------------------------------------------------------------*/
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* The section being deleted is one or more characters                */
		/*--------------------------------------------------------------------*/
		copyOffset = endPosition - startPosition;

		/*--------------------------------------------------------------------*/
		/* Check for any captured lineBreaks/newline characters               */
		/*--------------------------------------------------------------------*/
		for (checkCtr = startPosition; checkCtr <= endPosition; checkCtr++)
		{
			remChar = startBlock->data[checkCtr];
			if (remChar == '\0')
			{
				/*------------------------------------------------------------*/
				/* Shift over the bytes by one more character                 */
				/*------------------------------------------------------------*/
				// copyOffset++;
				tblk->nLines--;
				startBlock->nLines--;
			}
		}

		endOfCopy  = startBlock->bytesInUse - copyOffset;

		/*--------------------------------------------------------------------*/
		/* Simply shift over the bytes by one                                 */
		/*--------------------------------------------------------------------*/
		for (copyCtr = startPosition; copyCtr < endOfCopy; copyCtr++)
		{
			/*----------------------------------------------------------------*/
			/* First store the value of the deleted character.                */
			/*----------------------------------------------------------------*/
			remChar = startBlock->data[copyCtr];
			startBlock->data[copyCtr] = startBlock->data[copyCtr+copyOffset];
		}
		startBlock->bytesInUse -= copyOffset;
		startBlock->data[startBlock->bytesInUse] = '\0';
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* The section being deleted spans more than one block                */
		/*--------------------------------------------------------------------*/
		#pragma message("[TODO]: `neuik_TextBlock_DeleteSection` Delete over multiple blocks.")
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Replace a character at the specified position with another                 */
/*----------------------------------------------------------------------------*/
int neuik_TextBlock_ReplaceChar(
	neuik_TextBlock * tblk,
	size_t            lineNo,
	size_t            byteNo,
	char              newChar)
{
	neuik_TextBlockData * aBlock;
	size_t                copyCtr;
	size_t                lineLen;
	size_t                lineBreakByte = 0;
	size_t                position;
	int                   eNum       = 0; /* which error to report (if any) */
	static char           funcName[] = "neuik_TextBlock_ReplaceChar";
	static char         * errMsgs[]  = {"",                            // [0] no error
		"Output argument `tblk` is NULL.",                             // [1]
		"Failure in function `neuik_TextBlock_GetLineLength`.",        // [2]
		"Argument `byteNo` has value in excess of line length.",       // [3]
		"Fundamental error in basic function `GetPositionLineStart`.", // [4]
	};

	if (tblk == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Make sure we aren't attempting to insert a character into a line at a  */
	/* position that is outside of its scope.                                 */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
	{
		eNum = 2;
		goto out;
	}
	if (byteNo > lineLen)
	{
		if (neuik_TextBlock_GetLineLength(tblk, lineNo, &lineLen))
		{
			eNum = 2;
			goto out;
		}
		eNum = 3;
		goto out;
	}

	if (neuik_TextBlock_GetPositionInLine__noErrChecks(
		tblk, lineNo, byteNo, &aBlock, &position))
	{
		if (neuik_TextBlock_GetPositionInLine__noErrChecks(
			tblk, lineNo, byteNo, &aBlock, &position))
		{
			eNum = 4;
			goto out;
		}

		eNum = 4;
		goto out;
	}

	if (newChar == '\n')
	{
		lineBreakByte = 1;
	}

	/*------------------------------------------------------------------------*/
	/* It is likely that line data will need to be shifted over to accomodate */
	/* this new character.                                                    */
	/*------------------------------------------------------------------------*/
	if (aBlock->bytesInUse + lineBreakByte < aBlock->bytesAllocated)
	{
		/*--------------------------------------------------------------------*/
		/* Simply shift over the bytes by one                                 */
		/*--------------------------------------------------------------------*/
		aBlock->data[position] = newChar;

		if (newChar == '\n')
		{
			/*----------------------------------------------------------------*/
			/* Shift over the bytes by one more character                     */
			/*----------------------------------------------------------------*/
			aBlock->data[1 + aBlock->bytesInUse] = '\0';
			for (copyCtr = aBlock->bytesInUse; copyCtr > position; copyCtr--)
			{
				aBlock->data[copyCtr] = aBlock->data[copyCtr-1];
			}
			aBlock->data[position+1] = '\0';
			aBlock->bytesInUse++;
		}
	}
	else
	{
		#pragma message("[TODO] `neuik_TextBlock_InsertChar` Complex byte Shift")
	}

	if (newChar == '\n')
	{
		tblk->nLines++;
		aBlock->nLines++;
	}
	// printf("neuik_TextBlock_InsertChar(): [%u:%u] `%s`\n", 
	// 	lineNo, byteNo, aBlock->data);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

/*----------------------------------------------------------------------------*/
/* Replace one or more characters at the specified position with one or more  */
/* characters.                                                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_ReplaceChars(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			size_t            linePos,
			char            * newString);

/*----------------------------------------------------------------------------*/
/* Refactoring an object allows it to perform housekeeping so that it can     */
/* perform at it's best.                                                      */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_Refactor(
		neuik_TextBlock * tblk,
		int               refactorLevel); /**/



