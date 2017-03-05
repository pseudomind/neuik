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
#ifndef NEUIK_TEXTBLOCK_H
#define NEUIK_TEXTBLOCK_H

// #include "NEUIK_defs.h"

typedef struct {
	unsigned int   firstLineNo;    /* 0 = start of */
	unsigned int   nLines;         /* number of actual lines in block */
	unsigned int   bytesAllocated; /* max size of text block (number of characters) */
	unsigned int   bytesInUse;     /* number of allocated bytes that are currently used */
	char         * data;           
	void         * previousBlock;  /* NULL = first block */
	void         * nextBlock;      /* NULL = last block  */
} neuik_TextBlockData;

typedef struct {
	unsigned int           blockSize;         /* the number of blocks per chapter */
	unsigned int           chapterSize;       /* the number of blocks per chapter */
	unsigned int           nDataBlocks;       /* the number of data blocks in the TextBlock */
	unsigned int           nLines;            /* total number of lines in the TextBlock */
	unsigned int           nChapters;         /* total number of chapters in the TextBlock */
	unsigned int           chaptersAllocated; /* size of allocated chapter array */
	neuik_TextBlockData *  firstBlock;
	neuik_TextBlockData *  lastBlock;
	neuik_TextBlockData ** chapters;      /*  */
} neuik_TextBlock;

int
	neuik_NewTextBlock(
			neuik_TextBlock ** tblkPtr,
			unsigned int       blockSize,
			unsigned int       chapterSize);

int 
	neuik_TextBlock_SetText(
			neuik_TextBlock * tblk,
			const char      * text);

/*----------------------------------------------------------------------------*/
/* Get the number of lines contained by the TextBlock.                        */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetLineCount(
			neuik_TextBlock * tblk,
			unsigned int    * nLines);

/*----------------------------------------------------------------------------*/
/* Check to see if a line number is contained by a TextBlock                  */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_HasLine(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			int             * hasLine);

/*----------------------------------------------------------------------------*/
/* Get the length of a line of text contained in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_GetLineLength(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int    * length);

/*----------------------------------------------------------------------------*/
/* Get a copy of the text contained by a line in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetLine(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			char           ** lineData);

/*----------------------------------------------------------------------------*/
/* Get a copy of the specified textSection from a TextBlock                   */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetSection(
			neuik_TextBlock * tblk,
			unsigned int      startLineNo,
			unsigned int      startLinePos,
			unsigned int      endLineNo,
			unsigned int      endLinePos,
			char           ** secData);

/*----------------------------------------------------------------------------*/
/* Replace an actual line of data with another                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_ReplaceLine(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Delete the specified line number                                           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteLine(
			neuik_TextBlock * tblk,
			unsigned int      lineNo);

/*----------------------------------------------------------------------------*/
/* Insert a line before the specified line number                             */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_InsertLine(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Insert a line after the specified line number                              */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_InsertLineAfter(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			const char      * lineData);

/*----------------------------------------------------------------------------*/
/* Insert a character at the specified position                               */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_InsertChar(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int      linePos,
			char              newChar);

/*----------------------------------------------------------------------------*/
/* Insert multiple characters at the specified position                       */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_InsertChars(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int      linePos,
			char            * newString);

/*----------------------------------------------------------------------------*/
/* Delete a character at a position                                           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteChar(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int      linePos);

/*----------------------------------------------------------------------------*/
/* Delete a number of characters at a position                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteSection(
			neuik_TextBlock * tblk,
			unsigned int      startLineNo,
			unsigned int      startLinePos,
			unsigned int      endLineNo,
			unsigned int      endLinePos);

/*----------------------------------------------------------------------------*/
/* Replace a character at the specified position with another                 */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_ReplaceChar(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int      linePos,
			char              newChar);

/*----------------------------------------------------------------------------*/
/* Replace one or more characters at the specified position with one or more  */
/* characters.                                                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_ReplaceChars(
			neuik_TextBlock * tblk,
			unsigned int      lineNo,
			unsigned int      linePos,
			char            * newString);

/*----------------------------------------------------------------------------*/
/* Effectively deletes the line ending of the specified line and tacks on the */
/* contents of the following line to the end of the specified line.           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_MergeLines(
			neuik_TextBlock * tblk,
			unsigned int      lineNo);

/*----------------------------------------------------------------------------*/
/* Refactoring an object allows it to perform housekeeping so that it can     */
/* perform at it's best.                                                      */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_Refactor(
		neuik_TextBlock * tblk,
		int               refactorLevel); /**/




#endif /* NEUIK_TEXTBLOCK_H */
