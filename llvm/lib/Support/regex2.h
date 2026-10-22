/*-
 * This code is derived from OpenBSD's libc/regex, original license follows:
 *
 * Copyright (c) 1992, 1993, 1994 Henry Spencer.
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Henry Spencer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)regex2.h	8.4 (Berkeley) 3/20/94
 */

#اگر_تعریف_نشده LLVM_SUPPORT_REGEX2_H
#تعریف_کن LLVM_SUPPORT_REGEX2_H

#ضمن "regutils.h"
#ضمن <stddef.h>

/*
 * internals of regex_t
 */
#تعریف_کن	MAGIC1	((('r'^0200)<<8) | 'e')

/*
 * The internal representation is a *strip*, a sequence of
 * operators ending with an endmarker.  (Some terminology etc. is a
 * historical relic of earlier versions which used multiple strips.)
 * Certain oddities in the representation are there to permit running
 * the machinery backwards; in particular, any deviation from sequential
 * flow must be marked at both its source and its destination.  Some
 * fine points:
 *
 * - OPLUS_ and O_PLUS are *inside* the loop they create.
 * - OQUEST_ and O_QUEST are *outside* the bypass they create.
 * - OCH_ and O_CH are *outside* the multi-way branch they create, while
 *   OOR1 and OOR2 are respectively the end and the beginning of one of
 *   the branches.  Note that there is an implicit OOR2 following OCH_
 *   and an implicit OOR1 preceding O_CH.
 *
 * In state representations, an operator's bit is on to signify a state
 * immediately *preceding* "execution" of that operator.
 */
تعريفنوع بیعلامت دراز sop;	/* strip operator */
تعريفنوع دراز sopno;
#تعریف_کن	OPRMASK	0xf8000000LU
#تعریف_کن	OPDMASK	0x07ffffffLU
#تعریف_کن	OPSHIFT	((بیعلامت)27)
#تعریف_کن	OP(n)	((n)&OPRMASK)
#تعریف_کن	OPND(n)	((n)&OPDMASK)
#تعریف_کن	SOP(op, opnd)	((op)|(opnd))
/* operators			   meaning	operand			*/
/*						(back, fwd are offsets)	*/
#تعریف_کن	OEND	(1LU<<OPSHIFT)	/* endmarker	-			*/
#تعریف_کن	OCHAR	(2LU<<OPSHIFT)	/* character	unsigned char		*/
#تعریف_کن	OBOL	(3LU<<OPSHIFT)	/* left anchor	-			*/
#تعریف_کن	OEOL	(4LU<<OPSHIFT)	/* right anchor	-			*/
#تعریف_کن	OANY	(5LU<<OPSHIFT)	/* .		-			*/
#تعریف_کن	OANYOF	(6LU<<OPSHIFT)	/* [...]	set number		*/
#تعریف_کن	OBACK_	(7LU<<OPSHIFT)	/* begin \d	paren number		*/
#تعریف_کن	O_پشت	(8LU<<OPSHIFT)	/* end \d	paren number		*/
#تعریف_کن	OPLUS_	(9LU<<OPSHIFT)	/* + prefix	fwd to suffix		*/
#تعریف_کن	O_PLUS	(10LU<<OPSHIFT)	/* + suffix	back to prefix		*/
#تعریف_کن	OQUEST_	(11LU<<OPSHIFT)	/* ? prefix	fwd to suffix		*/
#تعریف_کن	O_QUEST	(12LU<<OPSHIFT)	/* ? suffix	back to prefix		*/
#تعریف_کن	OLPAREN	(13LU<<OPSHIFT)	/* (		fwd to )		*/
#تعریف_کن	ORPAREN	(14LU<<OPSHIFT)	/* )		back to (		*/
#تعریف_کن	OCH_	(15LU<<OPSHIFT)	/* begin choice	fwd to OOR2		*/
#تعریف_کن	OOR1	(16LU<<OPSHIFT)	/* | pt. 1	back to OOR1 or OCH_	*/
#تعریف_کن	OOR2	(17LU<<OPSHIFT)	/* | pt. 2	fwd to OOR2 or O_CH	*/
#تعریف_کن	O_CH	(18LU<<OPSHIFT)	/* end choice	back to OOR1		*/
#تعریف_کن	OBOW	(19LU<<OPSHIFT)	/* begin word	-			*/
#تعریف_کن	OEOW	(20LU<<OPSHIFT)	/* end word	-			*/

/*
 * Structure for [] character-set representation.  Character sets are
 * done as bit vectors, grouped 8 to a byte vector for compactness.
 * The individual set therefore has both a pointer to the byte vector
 * and a mask to pick out the relevant bit of each byte.  A hash code
 * simplifies testing whether two sets could be identical.
 *
 * This will get trickier for multicharacter collating elements.  As
 * preliminary hooks for dealing with such things, we also carry along
 * a string of multi-character elements, and decide the size of the
 * vectors at run time.
 */
تعريفنوع بنية {
	uch *ptr;		/* -> uch [csetsize] */
	uch ماسک_2;		/* bit within array */
	uch hash;		/* hash code */
	size_t smultis;
	حرف *multis;		/* -> char[smulti]  ab\0cd\0ef\0\0 */
} cset;
/* note that CHadd and CHsub are unsafe, and CHIN doesn't yield 0/1 */
#تعریف_کن	CHadd(cs, c)	((cs)->ptr[(uch)(c)] |= (cs)->ماسک_2, (cs)->hash += (c))
#تعریف_کن	CHsub(cs, c)	((cs)->ptr[(uch)(c)] &= ~(cs)->ماسک_2, (cs)->hash -= (c))
#تعریف_کن	CHIN(cs, c)	((cs)->ptr[(uch)(c)] & (cs)->ماسک_2)
#تعریف_کن	MCadd(p, cs, cp)	mcadd(p, cs, cp)	/* llvm_regcomp() internal fns */
#تعریف_کن	MCsub(p, cs, cp)	mcsub(p, cs, cp)
#تعریف_کن	MCin(p, cs, cp)	mcin(p, cs, cp)

/* stuff for character categories */
تعريفنوع بیعلامت حرف cat_t;

/*
 * main compiled-expression structure
 */
بنية re_guts {
	صحيح magic;
#		define	MAGIC2	((('R'^0200)<<8)|'E')
	sop *strip;		/* malloced area for strip */
	صحيح csetsize;		/* number of bits in a cset vector */
	صحيح ncsets;		/* number of csets in use */
	cset *sets;		/* -> cset [ncsets] */
	uch *setbits;		/* -> uch[csetsize][ncsets/CHAR_BIT] */
	صحيح cflags;		/* copy of llvm_regcomp() cflags argument */
	sopno nstates;		/* = number of sops */
	sopno firststate;	/* the initial OEND (normally 0) */
	sopno laststate;	/* the final OEND */
	صحيح iflags;		/* internal flags */
#		define	USEBOL	01	/* used ^ */
#		define	USEEOL	02	/* used $ */
#		define	REGEX_BAD	04	/* something wrong */
	صحيح nbol;		/* number of ^ used */
	صحيح neol;		/* number of $ used */
	صحيح ncategories;	/* how many character categories */
	cat_t *categories;	/* ->catspace[-CHAR_MIN] */
	حرف *must;		/* match must contain this string */
	صحيح mlen;		/* length of must */
	size_t nsub;		/* copy of re_nsub */
	صحيح backrefs;		/* does it use back references? */
	sopno nplus;		/* how deep does it nest +s? */
	/* catspace must be last */
	cat_t catspace[1];	/* actually [NC] */
};

/* misc utilities */
#تعریف_کن	بیرون_3	(CHAR_MAX+1)	/* a non-character value */
#تعریف_کن	ISWORD(c)	(آیا_حرف_یا_رقم(c&0xff) || (c) == '_')

#پایان_اگر
