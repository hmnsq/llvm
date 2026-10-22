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
#ضمن <sys/types.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <limits.h>
#ضمن <ctype.h>
#ضمن "regex_impl.h"

#ضمن "regutils.h"
#ضمن "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#تعریف_کرو	states1	لما		/* for later use in llvm_regexec() decision */
#تعریف_کرو	حالتاں	states1
#تعریف_کرو	CLEAR(v)	((v) = 0)
#تعریف_کرو	SET0(v, n)	((v) &= ~((بنانشانی لما)1 << (n)))
#تعریف_کرو	SET1(v, n)	((v) |= (بنانشانی لما)1 << (n))
#تعریف_کرو	ISSET(v, n)	(((v) & ((بنانشانی لما)1 << (n))) != 0)
#تعریف_کرو	سونپو_3(d, s)	((d) = (s))
#تعریف_کرو	EQ(a, b)	((a) == (b))
#تعریف_کرو	STATEVARS	لما dummy	/* dummy version */
#تعریف_کرو	STATESETUP(m, n)	/* nothing */
#تعریف_کرو	STATETEARDOWN(m)	/* nothing */
#تعریف_کرو	تیاری(v)	((v) = 0)
#تعریف_کرو	onestate	لما
#تعریف_کرو	INIT(o, n)	((o) = (بنانشانی لما)1 << (n))
#تعریف_کرو	INC(o)		((o) = (بنانشانی لما)(o) << 1)
#تعریف_کرو	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#تعریف_کرو	FWD(dst, src, n)	((dst) |= ((بنانشانی لما)(src)&(ایتھے_2)) << (n))
#تعریف_کرو	پٹھ_2(dst, src, n)	((dst) |= ((بنانشانی لما)(src)&(ایتھے_2)) >> (n))
#تعریف_کرو	ISSETBACK(v, n)		(((v) & ((بنانشانی لما)ایتھے_2 >> (n))) != 0)
/* function names */
#تعریف_کرو SNAMES			/* engine.inc looks after details */

#ضمن "regengine.inc"

/* now undo things */
#تعریف_رد	حالتاں
#تعریف_رد	CLEAR
#تعریف_رد	SET0
#تعریف_رد	SET1
#تعریف_رد	ISSET
#تعریف_رد	سونپو_3
#تعریف_رد	EQ
#تعریف_رد	STATEVARS
#تعریف_رد	STATESETUP
#تعریف_رد	STATETEARDOWN
#تعریف_رد	تیاری
#تعریف_رد	onestate
#تعریف_رد	INIT
#تعریف_رد	INC
#تعریف_رد	ISSTATEIN
#تعریف_رد	FWD
#تعریف_رد	پٹھ_2
#تعریف_رد	ISSETBACK
#تعریف_رد	SNAMES

/* macros for manipulating states, large version */
#تعریف_کرو	حالتاں	حرف *
#تعریف_کرو	CLEAR(v)	memset(v, 0, m->g->nstates)
#تعریف_کرو	SET0(v, n)	((v)[n] = 0)
#تعریف_کرو	SET1(v, n)	((v)[n] = 1)
#تعریف_کرو	ISSET(v, n)	((v)[n])
#تعریف_کرو	سونپو_3(d, s)	memmove(d, s, m->g->nstates)
#تعریف_کرو	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#تعریف_کرو	STATEVARS	لما vn; حرف *space
#تعریف_کرو	STATESETUP(m, nv)	{ (m)->space = حافظہ_دیو((nv)*(m)->g->nstates); \
				إذا ((m)->space == عدم) أرجع(REG_ESPACE); \
				(m)->vn = 0; }
#تعریف_کرو	STATETEARDOWN(m)	{ حرر((m)->space); }
#تعریف_کرو	تیاری(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#تعریف_کرو	onestate	لما
#تعریف_کرو	INIT(o, n)	((o) = (n))
#تعریف_کرو	INC(o)	((o)++)
#تعریف_کرو	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#تعریف_کرو	FWD(dst, src, n)	((dst)[ایتھے_2+(n)] |= (src)[ایتھے_2])
#تعریف_کرو	پٹھ_2(dst, src, n)	((dst)[ایتھے_2-(n)] |= (src)[ایتھے_2])
#تعریف_کرو	ISSETBACK(v, n)	((v)[ایتھے_2 - (n)])
/* function names */
#تعریف_کرو	LNAMES			/* flag */

#ضمن "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
صحيح				/* 0 success, REG_NOMATCH failure */
llvm_regexec(ثابت llvm_regex_t *preg, ثابت حرف *string, size_t nmatch,
             llvm_regmatch_t pmatch[], صحيح eflags)
{
	بنية re_guts *g = preg->re_g;
#جے_تعریف_شدہ REDEBUG
#	define	GOODFLAGS(f)	(f)
#وإلا
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#جے_انت

	إذا (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		أرجع(REG_BADPAT);
	أكد(!(g->iflags&REGEX_BAD));
	إذا (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		أرجع(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	إذا (g->nstates <= (لما)(CHAR_BIT*حجم(states1)) && !(eflags&REG_LARGE))
		أرجع(smatcher(g, string, nmatch, pmatch, eflags));
	وإلا
		أرجع(lmatcher(g, string, nmatch, pmatch, eflags));
}
