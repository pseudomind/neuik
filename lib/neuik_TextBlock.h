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
#ifndef NEUIK_TEXTBLOCK_H
#define NEUIK_TEXTBLOCK_H

// #include "NEUIK_defs.h"

typedef struct {
	size_t   firstLineNo;    /* 0 = start of */
	size_t   nLines;         /* number of actual lines in block */
	size_t   bytesAllocated; /* max size of text block (number of characters) */
	size_t   bytesInUse;     /* number of allocated bytes that are currently used */
	char   * data;           
	void   * previousBlock;  /* NULL = first block */
	void   * nextBlock;      /* NULL = last block  */
} neuik_TextBlockData;

typedef struct {
	size_t                 blockSize;         /* the number of blocks per chapter */
	size_t                 chapterSize;       /* the number of blocks per chapter */
	size_t                 nDataBlocks;       /* the number of data blocks in the TextBlock */
	size_t                 length;            /* total number of bytes of text in the TextBlock */
	size_t                 nLines;            /* total number of lines in the TextBlock */
	size_t                 nChapters;         /* total number of chapters in the TextBlock */
	size_t                 chaptersAllocated; /* size of allocated chapter array */
	unsigned int           overProvisionPct;  /* percent of textBlockData required to be unused. */
	neuik_TextBlockData *  firstBlock;
	neuik_TextBlockData *  lastBlock;
	neuik_TextBlockData ** chapters;      /*  */
} neuik_TextBlock;

int
	neuik_NewTextBlock(
			neuik_TextBlock ** tblkPtr,
			size_t             blockSize,
			size_t             chapterSize);

int 
	neuik_TextBlock_SetText(
			neuik_TextBlock * tblk,
			const char      * text);

/*----------------------------------------------------------------------------*/
/* Get the number of bytes contained by the TextBlock.                        */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetLength(
			neuik_TextBlock * tblk,
			size_t          * length);

/*----------------------------------------------------------------------------*/
/* Get the number of lines contained by the TextBlock.                        */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetLineCount(
			neuik_TextBlock * tblk,
			size_t          * nLines);

/*----------------------------------------------------------------------------*/
/* Check to see if a line number is contained by a TextBlock                  */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_HasLine(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			int             * hasLine);

/*----------------------------------------------------------------------------*/
/* Get the length of a line of text contained in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_GetLineLength(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			size_t          * length);

/*----------------------------------------------------------------------------*/
/* Get a copy of the text contained by a line in a TextBlock                  */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetLine(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			char           ** lineData);

/*----------------------------------------------------------------------------*/
/* Get a copy of the specified textSection from a TextBlock                   */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_GetSection(
			neuik_TextBlock * tblk,
			size_t            startLineNo,
			size_t            startLinePos,
			size_t            endLineNo,
			size_t            endLinePos,
			char           ** secData);

/*----------------------------------------------------------------------------*/
/* Get the number of characters encapsulated by the specified section.        */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_GetSectionLength(
			neuik_TextBlock * tblk,
			size_t            startLineNo,
			size_t            startLinePos,
			size_t            endLineNo,
			size_t            endLinePos,
			size_t          * secLen);

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
int 
	neuik_TextBlock_InsertChar(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			size_t            linePos,
			char              newChar);

/*----------------------------------------------------------------------------*/
/* Insert a string of characters at the specified position                    */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_InsertText(
			neuik_TextBlock * tblk,
			size_t            lineNo,        /* line in which to insert text */
			size_t            linePos,       /* position within the line to insert */
			const char      * text,          /* text section to be inserted */ 
			size_t          * finalLineNo,   /* line where resulting insert completed */
			size_t          * finalLinePos); /* line position where insert completed */

/*----------------------------------------------------------------------------*/
/* Delete a character at a position                                           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteChar(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			size_t            linePos);

/*----------------------------------------------------------------------------*/
/* Delete a number of characters at a position                                */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_DeleteSection(
			neuik_TextBlock * tblk,
			size_t            startLineNo,
			size_t            startLinePos,
			size_t            endLineNo,
			size_t            endLinePos);

/*----------------------------------------------------------------------------*/
/* Replace a character at the specified position with another                 */
/*----------------------------------------------------------------------------*/
int 
	neuik_TextBlock_ReplaceChar(
			neuik_TextBlock * tblk,
			size_t            lineNo,
			size_t            linePos,
			char              newChar);

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
/* Effectively deletes the line ending of the specified line and tacks on the */
/* contents of the following line to the end of the specified line.           */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_MergeLines(
			neuik_TextBlock * tblk,
			size_t            lineNo);

/*----------------------------------------------------------------------------*/
/* Refactoring an object allows it to perform housekeeping so that it can     */
/* perform at it's best.                                                      */
/*----------------------------------------------------------------------------*/
int
	neuik_TextBlock_Refactor(
		neuik_TextBlock * tblk,
		int               refactorLevel); /**/




#endif /* NEUIK_TEXTBLOCK_H */
