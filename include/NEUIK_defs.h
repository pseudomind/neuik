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
#ifndef NEUIK_DEFS_H
#define NEUIK_DEFS_H

enum neuik_HJustify
{
	NEUIK_HJUSTIFY_DEFAULT,
	NEUIK_HJUSTIFY_LEFT,
	NEUIK_HJUSTIFY_CENTER,
	NEUIK_HJUSTIFY_RIGHT,
};

enum neuik_VJustify
{
	NEUIK_VJUSTIFY_DEFAULT,
	NEUIK_VJUSTIFY_TOP,
	NEUIK_VJUSTIFY_CENTER,
	NEUIK_VJUSTIFY_BOTTOM,
};

#define NEUIK_RESTRICT_NONE             0 /*    *            */
#define NEUIK_RESTRICT_ALPHA            1 /*    \w*          */
#define NEUIK_RESTRICT_NUM              2 /*    \d*          */
#define NEUIK_RESTRICT_NUM_SIGNED       3 /*    [+-\d]\d*    */
#define NEUIK_RESTRICT_ALPHANUM         4 /*    [\d\w]*      */
#define NEUIK_RESTRICT_FLOAT            5 /*    [+-\d.]      */
#define NEUIK_RESTRICT_CUSTOM_STR_ONLY  6 /* input can only be chars in custom_str */
#define NEUIK_RESTRICT_CUSTOM_STR_NOT   8 /* input can be anything but chars in custom_str */

#define NEUIK_DOUBLE_CLICK_TIMEOUT      200 /* time to register as dbl click (ms) */

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef UNDEFINED
	#define UNDEFINED -1
#endif

#endif /* NEUIK_DEFS_H */
