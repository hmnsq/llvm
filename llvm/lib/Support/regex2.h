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

#నిర్వచితం_కాకపోతే LLVM_SUPPORT_REGEX2_H
#నిర్వచించు LLVM_SUPPORT_REGEX2_H

#అన్తర్భావ "regutils.h"
#అన్తర్భావ <stddef.h>

/*
 * internals of regex_t
 */
#నిర్వచించు	MAGIC1	((('r'^0200)<<8) | 'e')

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
రకనిర్వచనం గుర్తులేని దీర్ఘ sop;	/* strip operator */
రకనిర్వచనం దీర్ఘ sopno;
#నిర్వచించు	OPRMASK	0xf8000000LU
#నిర్వచించు	OPDMASK	0x07ffffffLU
#నిర్వచించు	OPSHIFT	((గుర్తులేని)27)
#నిర్వచించు	OP(n)	((n)&OPRMASK)
#నిర్వచించు	OPND(n)	((n)&OPDMASK)
#నిర్వచించు	SOP(op, opnd)	((op)|(opnd))
/* operators			   meaning	operand			*/
/*						(back, fwd are offsets)	*/
#నిర్వచించు	OEND	(1LU<<OPSHIFT)	/* endmarker	-			*/
#నిర్వచించు	OCHAR	(2LU<<OPSHIFT)	/* character	unsigned char		*/
#నిర్వచించు	OBOL	(3LU<<OPSHIFT)	/* left anchor	-			*/
#నిర్వచించు	OEOL	(4LU<<OPSHIFT)	/* right anchor	-			*/
#నిర్వచించు	OANY	(5LU<<OPSHIFT)	/* .		-			*/
#నిర్వచించు	OANYOF	(6LU<<OPSHIFT)	/* [...]	set number		*/
#నిర్వచించు	OBACK_	(7LU<<OPSHIFT)	/* begin \d	paren number		*/
#నిర్వచించు	O_వెనుక	(8LU<<OPSHIFT)	/* end \d	paren number		*/
#నిర్వచించు	OPLUS_	(9LU<<OPSHIFT)	/* + prefix	fwd to suffix		*/
#నిర్వచించు	O_PLUS	(10LU<<OPSHIFT)	/* + suffix	back to prefix		*/
#నిర్వచించు	OQUEST_	(11LU<<OPSHIFT)	/* ? prefix	fwd to suffix		*/
#నిర్వచించు	O_QUEST	(12LU<<OPSHIFT)	/* ? suffix	back to prefix		*/
#నిర్వచించు	OLPAREN	(13LU<<OPSHIFT)	/* (		fwd to )		*/
#నిర్వచించు	ORPAREN	(14LU<<OPSHIFT)	/* )		back to (		*/
#నిర్వచించు	OCH_	(15LU<<OPSHIFT)	/* begin choice	fwd to OOR2		*/
#నిర్వచించు	OOR1	(16LU<<OPSHIFT)	/* | pt. 1	back to OOR1 or OCH_	*/
#నిర్వచించు	OOR2	(17LU<<OPSHIFT)	/* | pt. 2	fwd to OOR2 or O_CH	*/
#నిర్వచించు	O_CH	(18LU<<OPSHIFT)	/* end choice	back to OOR1		*/
#నిర్వచించు	OBOW	(19LU<<OPSHIFT)	/* begin word	-			*/
#నిర్వచించు	OEOW	(20LU<<OPSHIFT)	/* end word	-			*/

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
రకనిర్వచనం నిర్మాణం {
	uch *ptr;		/* -> uch [csetsize] */
	uch ముసుగు_2;		/* bit within array */
	uch hash;		/* hash code */
	size_t smultis;
	అక్షరం *multis;		/* -> char[smulti]  ab\0cd\0ef\0\0 */
} cset;
/* note that CHadd and CHsub are unsafe, and CHIN doesn't yield 0/1 */
#నిర్వచించు	CHadd(cs, c)	((cs)->ptr[(uch)(c)] |= (cs)->ముసుగు_2, (cs)->hash += (c))
#నిర్వచించు	CHsub(cs, c)	((cs)->ptr[(uch)(c)] &= ~(cs)->ముసుగు_2, (cs)->hash -= (c))
#నిర్వచించు	CHIN(cs, c)	((cs)->ptr[(uch)(c)] & (cs)->ముసుగు_2)
#నిర్వచించు	MCadd(p, cs, cp)	mcadd(p, cs, cp)	/* llvm_regcomp() internal fns */
#నిర్వచించు	MCsub(p, cs, cp)	mcsub(p, cs, cp)
#నిర్వచించు	MCin(p, cs, cp)	mcin(p, cs, cp)

/* stuff for character categories */
రకనిర్వచనం గుర్తులేని అక్షరం cat_t;

/*
 * main compiled-expression structure
 */
నిర్మాణం re_guts {
	పూర్ణాంకం magic;
#		define	MAGIC2	((('R'^0200)<<8)|'E')
	sop *strip;		/* malloced area for strip */
	పూర్ణాంకం csetsize;		/* number of bits in a cset vector */
	పూర్ణాంకం ncsets;		/* number of csets in use */
	cset *sets;		/* -> cset [ncsets] */
	uch *setbits;		/* -> uch[csetsize][ncsets/CHAR_BIT] */
	పూర్ణాంకం cflags;		/* copy of llvm_regcomp() cflags argument */
	sopno nstates;		/* = number of sops */
	sopno firststate;	/* the initial OEND (normally 0) */
	sopno laststate;	/* the final OEND */
	పూర్ణాంకం iflags;		/* internal flags */
#		define	USEBOL	01	/* used ^ */
#		define	USEEOL	02	/* used $ */
#		define	REGEX_BAD	04	/* something wrong */
	పూర్ణాంకం nbol;		/* number of ^ used */
	పూర్ణాంకం neol;		/* number of $ used */
	పూర్ణాంకం ncategories;	/* how many character categories */
	cat_t *categories;	/* ->catspace[-CHAR_MIN] */
	అక్షరం *must;		/* match must contain this string */
	పూర్ణాంకం mlen;		/* length of must */
	size_t nsub;		/* copy of re_nsub */
	పూర్ణాంకం backrefs;		/* does it use back references? */
	sopno nplus;		/* how deep does it nest +s? */
	/* catspace must be last */
	cat_t catspace[1];	/* actually [NC] */
};

/* misc utilities */
#నిర్వచించు	బహిర్గతం_3	(CHAR_MAX+1)	/* a non-character value */
#నిర్వచించు	ISWORD(c)	(అక్షరమా_అంకెనా(c&0xff) || (c) == '_')

#అయితే_ముగింపు
