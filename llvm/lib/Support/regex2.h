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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LLVM_SUPPORT_REGEX2_H
#ವ್ಯಾಖ್ಯಾನಿಸು LLVM_SUPPORT_REGEX2_H

#ಅನ್ತರ್ಭಾವ "regutils.h"
#ಅನ್ತರ್ಭಾವ <stddef.h>

/*
 * internals of regex_t
 */
#ವ್ಯಾಖ್ಯಾನಿಸು	MAGIC1	((('r'^0200)<<8) | 'e')

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sop;	/* strip operator */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ sopno;
#ವ್ಯಾಖ್ಯಾನಿಸು	OPRMASK	0xf8000000LU
#ವ್ಯಾಖ್ಯಾನಿಸು	OPDMASK	0x07ffffffLU
#ವ್ಯಾಖ್ಯಾನಿಸು	OPSHIFT	((ಚಿಹ್ನೆರಹಿತ)27)
#ವ್ಯಾಖ್ಯಾನಿಸು	OP(n)	((n)&OPRMASK)
#ವ್ಯಾಖ್ಯಾನಿಸು	OPND(n)	((n)&OPDMASK)
#ವ್ಯಾಖ್ಯಾನಿಸು	SOP(op, opnd)	((op)|(opnd))
/* operators			   meaning	operand			*/
/*						(back, fwd are offsets)	*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OEND	(1LU<<OPSHIFT)	/* endmarker	-			*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OCHAR	(2LU<<OPSHIFT)	/* character	unsigned char		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OBOL	(3LU<<OPSHIFT)	/* left anchor	-			*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OEOL	(4LU<<OPSHIFT)	/* right anchor	-			*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OANY	(5LU<<OPSHIFT)	/* .		-			*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OANYOF	(6LU<<OPSHIFT)	/* [...]	set number		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OBACK_	(7LU<<OPSHIFT)	/* begin \d	paren number		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	O_ಹಿಂಭಾಗ	(8LU<<OPSHIFT)	/* end \d	paren number		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OPLUS_	(9LU<<OPSHIFT)	/* + prefix	fwd to suffix		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	O_PLUS	(10LU<<OPSHIFT)	/* + suffix	back to prefix		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OQUEST_	(11LU<<OPSHIFT)	/* ? prefix	fwd to suffix		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	O_QUEST	(12LU<<OPSHIFT)	/* ? suffix	back to prefix		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OLPAREN	(13LU<<OPSHIFT)	/* (		fwd to )		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	ORPAREN	(14LU<<OPSHIFT)	/* )		back to (		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OCH_	(15LU<<OPSHIFT)	/* begin choice	fwd to OOR2		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OOR1	(16LU<<OPSHIFT)	/* | pt. 1	back to OOR1 or OCH_	*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OOR2	(17LU<<OPSHIFT)	/* | pt. 2	fwd to OOR2 or O_CH	*/
#ವ್ಯಾಖ್ಯಾನಿಸು	O_CH	(18LU<<OPSHIFT)	/* end choice	back to OOR1		*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OBOW	(19LU<<OPSHIFT)	/* begin word	-			*/
#ವ್ಯಾಖ್ಯಾನಿಸು	OEOW	(20LU<<OPSHIFT)	/* end word	-			*/

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	uch *ptr;		/* -> uch [csetsize] */
	uch ಮುಖವಾಡ_2;		/* bit within array */
	uch hash;		/* hash code */
	size_t smultis;
	ಅಕ್ಷರ *multis;		/* -> char[smulti]  ab\0cd\0ef\0\0 */
} cset;
/* note that CHadd and CHsub are unsafe, and CHIN doesn't yield 0/1 */
#ವ್ಯಾಖ್ಯಾನಿಸು	CHadd(cs, c)	((cs)->ptr[(uch)(c)] |= (cs)->ಮುಖವಾಡ_2, (cs)->hash += (c))
#ವ್ಯಾಖ್ಯಾನಿಸು	CHsub(cs, c)	((cs)->ptr[(uch)(c)] &= ~(cs)->ಮುಖವಾಡ_2, (cs)->hash -= (c))
#ವ್ಯಾಖ್ಯಾನಿಸು	CHIN(cs, c)	((cs)->ptr[(uch)(c)] & (cs)->ಮುಖವಾಡ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು	MCadd(p, cs, cp)	mcadd(p, cs, cp)	/* llvm_regcomp() internal fns */
#ವ್ಯಾಖ್ಯಾನಿಸು	MCsub(p, cs, cp)	mcsub(p, cs, cp)
#ವ್ಯಾಖ್ಯಾನಿಸು	MCin(p, cs, cp)	mcin(p, cs, cp)

/* stuff for character categories */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ cat_t;

/*
 * main compiled-expression structure
 */
ರಚನೆ re_guts {
	ಪೂರ್ಣಾಂಕ magic;
#		define	MAGIC2	((('R'^0200)<<8)|'E')
	sop *strip;		/* malloced area for strip */
	ಪೂರ್ಣಾಂಕ csetsize;		/* number of bits in a cset vector */
	ಪೂರ್ಣಾಂಕ ncsets;		/* number of csets in use */
	cset *sets;		/* -> cset [ncsets] */
	uch *setbits;		/* -> uch[csetsize][ncsets/CHAR_BIT] */
	ಪೂರ್ಣಾಂಕ cflags;		/* copy of llvm_regcomp() cflags argument */
	sopno nstates;		/* = number of sops */
	sopno firststate;	/* the initial OEND (normally 0) */
	sopno laststate;	/* the final OEND */
	ಪೂರ್ಣಾಂಕ iflags;		/* internal flags */
#		define	USEBOL	01	/* used ^ */
#		define	USEEOL	02	/* used $ */
#		define	REGEX_BAD	04	/* something wrong */
	ಪೂರ್ಣಾಂಕ nbol;		/* number of ^ used */
	ಪೂರ್ಣಾಂಕ neol;		/* number of $ used */
	ಪೂರ್ಣಾಂಕ ncategories;	/* how many character categories */
	cat_t *categories;	/* ->catspace[-CHAR_MIN] */
	ಅಕ್ಷರ *must;		/* match must contain this string */
	ಪೂರ್ಣಾಂಕ mlen;		/* length of must */
	size_t nsub;		/* copy of re_nsub */
	ಪೂರ್ಣಾಂಕ backrefs;		/* does it use back references? */
	sopno nplus;		/* how deep does it nest +s? */
	/* catspace must be last */
	cat_t catspace[1];	/* actually [NC] */
};

/* misc utilities */
#ವ್ಯಾಖ್ಯಾನಿಸು	ಹೊರಹೋಗು_3	(CHAR_MAX+1)	/* a non-character value */
#ವ್ಯಾಖ್ಯಾನಿಸು	ISWORD(c)	(ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(c&0xff) || (c) == '_')

#ಆದರೆ_ಅಂತ್ಯ
