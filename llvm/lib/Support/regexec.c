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
 *	@(#)regexec.c	8.3 (Berkeley) 3/20/94
 */

/*
 * the outer shell of llvm_regexec()
 *
 * This file includes engine.inc *twice*, after muchos fiddling with the
 * macros that code uses.  This lets the same code operate on two different
 * representations for state sets.
 */
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ "regex_impl.h"

#అన్తర్భావ "regutils.h"
#అన్తర్భావ "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#నిర్వచించు	states1	దీర్ఘ		/* for later use in llvm_regexec() decision */
#నిర్వచించు	స్థితులు	states1
#నిర్వచించు	CLEAR(v)	((v) = 0)
#నిర్వచించు	SET0(v, n)	((v) &= ~((గుర్తులేని దీర్ఘ)1 << (n)))
#నిర్వచించు	SET1(v, n)	((v) |= (గుర్తులేని దీర్ఘ)1 << (n))
#నిర్వచించు	ISSET(v, n)	(((v) & ((గుర్తులేని దీర్ఘ)1 << (n))) != 0)
#నిర్వచించు	కేటాయించు_3(d, s)	((d) = (s))
#నిర్వచించు	EQ(a, b)	((a) == (b))
#నిర్వచించు	STATEVARS	దీర్ఘ dummy	/* dummy version */
#నిర్వచించు	STATESETUP(m, n)	/* nothing */
#నిర్వచించు	STATETEARDOWN(m)	/* nothing */
#నిర్వచించు	అమరిక(v)	((v) = 0)
#నిర్వచించు	onestate	దీర్ఘ
#నిర్వచించు	INIT(o, n)	((o) = (గుర్తులేని దీర్ఘ)1 << (n))
#నిర్వచించు	INC(o)		((o) = (గుర్తులేని దీర్ఘ)(o) << 1)
#నిర్వచించు	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#నిర్వచించు	FWD(dst, src, n)	((dst) |= ((గుర్తులేని దీర్ఘ)(src)&(ఇక్కడ_2)) << (n))
#నిర్వచించు	వెనుక_2(dst, src, n)	((dst) |= ((గుర్తులేని దీర్ఘ)(src)&(ఇక్కడ_2)) >> (n))
#నిర్వచించు	ISSETBACK(v, n)		(((v) & ((గుర్తులేని దీర్ఘ)ఇక్కడ_2 >> (n))) != 0)
/* function names */
#నిర్వచించు SNAMES			/* engine.inc looks after details */

#అన్తర్భావ "regengine.inc"

/* now undo things */
#నిర్వచనం_రద్దు	స్థితులు
#నిర్వచనం_రద్దు	CLEAR
#నిర్వచనం_రద్దు	SET0
#నిర్వచనం_రద్దు	SET1
#నిర్వచనం_రద్దు	ISSET
#నిర్వచనం_రద్దు	కేటాయించు_3
#నిర్వచనం_రద్దు	EQ
#నిర్వచనం_రద్దు	STATEVARS
#నిర్వచనం_రద్దు	STATESETUP
#నిర్వచనం_రద్దు	STATETEARDOWN
#నిర్వచనం_రద్దు	అమరిక
#నిర్వచనం_రద్దు	onestate
#నిర్వచనం_రద్దు	INIT
#నిర్వచనం_రద్దు	INC
#నిర్వచనం_రద్దు	ISSTATEIN
#నిర్వచనం_రద్దు	FWD
#నిర్వచనం_రద్దు	వెనుక_2
#నిర్వచనం_రద్దు	ISSETBACK
#నిర్వచనం_రద్దు	SNAMES

/* macros for manipulating states, large version */
#నిర్వచించు	స్థితులు	అక్షరం *
#నిర్వచించు	CLEAR(v)	memset(v, 0, m->g->nstates)
#నిర్వచించు	SET0(v, n)	((v)[n] = 0)
#నిర్వచించు	SET1(v, n)	((v)[n] = 1)
#నిర్వచించు	ISSET(v, n)	((v)[n])
#నిర్వచించు	కేటాయించు_3(d, s)	memmove(d, s, m->g->nstates)
#నిర్వచించు	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#నిర్వచించు	STATEVARS	దీర్ఘ vn; అక్షరం *space
#నిర్వచించు	STATESETUP(m, nv)	{ (m)->space = జ్ఞాపకం_ఇవ్వు((nv)*(m)->g->nstates); \
				ఒకవేళ ((m)->space == ఖాళీ) తిరిగి(REG_ESPACE); \
				(m)->vn = 0; }
#నిర్వచించు	STATETEARDOWN(m)	{ విడుదల((m)->space); }
#నిర్వచించు	అమరిక(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#నిర్వచించు	onestate	దీర్ఘ
#నిర్వచించు	INIT(o, n)	((o) = (n))
#నిర్వచించు	INC(o)	((o)++)
#నిర్వచించు	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#నిర్వచించు	FWD(dst, src, n)	((dst)[ఇక్కడ_2+(n)] |= (src)[ఇక్కడ_2])
#నిర్వచించు	వెనుక_2(dst, src, n)	((dst)[ఇక్కడ_2-(n)] |= (src)[ఇక్కడ_2])
#నిర్వచించు	ISSETBACK(v, n)	((v)[ఇక్కడ_2 - (n)])
/* function names */
#నిర్వచించు	LNAMES			/* flag */

#అన్తర్భావ "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
పూర్ణాంకం				/* 0 success, REG_NOMATCH failure */
llvm_regexec(స్థిరరాశి llvm_regex_t *preg, స్థిరరాశి అక్షరం *string, size_t nmatch,
             llvm_regmatch_t pmatch[], పూర్ణాంకం eflags)
{
	నిర్మాణం re_guts *g = preg->re_g;
#నిర్వచితమైతే REDEBUG
#	define	GOODFLAGS(f)	(f)
#లేకపోతే
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#అయితే_ముగింపు

	ఒకవేళ (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		తిరిగి(REG_BADPAT);
	ప్రతిజ్ఞా(!(g->iflags&REGEX_BAD));
	ఒకవేళ (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		తిరిగి(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	ఒకవేళ (g->nstates <= (దీర్ఘ)(CHAR_BIT*పరిమాణం(states1)) && !(eflags&REG_LARGE))
		తిరిగి(smatcher(g, string, nmatch, pmatch, eflags));
	లేకపోతే
		తిరిగి(lmatcher(g, string, nmatch, pmatch, eflags));
}
