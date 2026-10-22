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
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ "regex_impl.h"

#અન્તર્ભાવ "regutils.h"
#અન્તર્ભાવ "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#define	states1	long		/* for later use in llvm_regexec() decision */
#define	states	states1
#define	CLEAR(v)	((v) = 0)
#define	SET0(v, n)	((v) &= ~((unsigned long)1 << (n)))
#define	SET1(v, n)	((v) |= (unsigned long)1 << (n))
#define	ISSET(v, n)	(((v) & ((unsigned long)1 << (n))) != 0)
#define	ASSIGN(d, s)	((d) = (s))
#define	EQ(a, b)	((a) == (b))
#define	STATEVARS	long dummy	/* dummy version */
#define	STATESETUP(m, n)	/* nothing */
#define	STATETEARDOWN(m)	/* nothing */
#define	SETUP(v)	((v) = 0)
#define	onestate	long
#define	INIT(o, n)	((o) = (unsigned long)1 << (n))
#define	INC(o)		((o) = (unsigned long)(o) << 1)
#define	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#define	FWD(dst, src, n)	((dst) |= ((unsigned long)(src)&(here)) << (n))
#define	BACK(dst, src, n)	((dst) |= ((unsigned long)(src)&(here)) >> (n))
#define	ISSETBACK(v, n)		(((v) & ((unsigned long)here >> (n))) != 0)
/* function names */
#define SNAMES			/* engine.inc looks after details */

#અન્તર્ભાવ "regengine.inc"

/* now undo things */
#undef	states
#undef	CLEAR
#undef	SET0
#undef	SET1
#undef	ISSET
#undef	ASSIGN
#undef	EQ
#undef	STATEVARS
#undef	STATESETUP
#undef	STATETEARDOWN
#undef	SETUP
#undef	onestate
#undef	INIT
#undef	INC
#undef	ISSTATEIN
#undef	FWD
#undef	BACK
#undef	ISSETBACK
#undef	SNAMES

/* macros for manipulating states, large version */
#define	states	અક્ષર *
#define	CLEAR(v)	memset(v, 0, m->g->nstates)
#define	SET0(v, n)	((v)[n] = 0)
#define	SET1(v, n)	((v)[n] = 1)
#define	ISSET(v, n)	((v)[n])
#define	ASSIGN(d, s)	memmove(d, s, m->g->nstates)
#define	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#define	STATEVARS	long vn; અક્ષર *space
#define	STATESETUP(m, nv)	{ (m)->space = malloc((nv)*(m)->g->nstates); \
				જો ((m)->space == ખાલી) પાછા(REG_ESPACE); \
				(m)->vn = 0; }
#define	STATETEARDOWN(m)	{ મુક્ત((m)->space); }
#define	SETUP(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#define	onestate	long
#define	INIT(o, n)	((o) = (n))
#define	INC(o)	((o)++)
#define	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#define	FWD(dst, src, n)	((dst)[here+(n)] |= (src)[here])
#define	BACK(dst, src, n)	((dst)[here-(n)] |= (src)[here])
#define	ISSETBACK(v, n)	((v)[here - (n)])
/* function names */
#define	LNAMES			/* flag */

#અન્તર્ભાવ "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
પૂર્ણાંક				/* 0 success, REG_NOMATCH failure */
llvm_regexec(અચલ llvm_regex_t *preg, અચલ અક્ષર *string, size_t nmatch,
             llvm_regmatch_t pmatch[], પૂર્ણાંક eflags)
{
	સંરચના re_guts *g = preg->re_g;
#ifdef REDEBUG
#	define	GOODFLAGS(f)	(f)
#નહીંતર
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#endif

	જો (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		પાછા(REG_BADPAT);
	પ્રતિજ્ઞા(!(g->iflags&REGEX_BAD));
	જો (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		પાછા(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	જો (g->nstates <= (long)(CHAR_BIT*કદ(states1)) && !(eflags&REG_LARGE))
		પાછા(smatcher(g, string, nmatch, pmatch, eflags));
	નહીંતર
		પાછા(lmatcher(g, string, nmatch, pmatch, eflags));
}
