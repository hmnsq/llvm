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
#وصف_ڪريو	states1	ڊگهو		/* for later use in llvm_regexec() decision */
#وصف_ڪريو	حالتون	states1
#وصف_ڪريو	CLEAR(v)	((v) = 0)
#وصف_ڪريو	SET0(v, n)	((v) &= ~((بنانشان ڊگهو)1 << (n)))
#وصف_ڪريو	SET1(v, n)	((v) |= (بنانشان ڊگهو)1 << (n))
#وصف_ڪريو	ISSET(v, n)	(((v) & ((بنانشان ڊگهو)1 << (n))) != 0)
#وصف_ڪريو	سونپيو_3(d, s)	((d) = (s))
#وصف_ڪريو	EQ(a, b)	((a) == (b))
#وصف_ڪريو	STATEVARS	ڊگهو dummy	/* dummy version */
#وصف_ڪريو	STATESETUP(m, n)	/* nothing */
#وصف_ڪريو	STATETEARDOWN(m)	/* nothing */
#وصف_ڪريو	تياري(v)	((v) = 0)
#وصف_ڪريو	onestate	ڊگهو
#وصف_ڪريو	INIT(o, n)	((o) = (بنانشان ڊگهو)1 << (n))
#وصف_ڪريو	INC(o)		((o) = (بنانشان ڊگهو)(o) << 1)
#وصف_ڪريو	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#وصف_ڪريو	FWD(dst, src, n)	((dst) |= ((بنانشان ڊگهو)(src)&(هتي_2)) << (n))
#وصف_ڪريو	پٺ_2(dst, src, n)	((dst) |= ((بنانشان ڊگهو)(src)&(هتي_2)) >> (n))
#وصف_ڪريو	ISSETBACK(v, n)		(((v) & ((بنانشان ڊگهو)هتي_2 >> (n))) != 0)
/* function names */
#وصف_ڪريو SNAMES			/* engine.inc looks after details */

#ضمن "regengine.inc"

/* now undo things */
#وصف_رد	حالتون
#وصف_رد	CLEAR
#وصف_رد	SET0
#وصف_رد	SET1
#وصف_رد	ISSET
#وصف_رد	سونپيو_3
#وصف_رد	EQ
#وصف_رد	STATEVARS
#وصف_رد	STATESETUP
#وصف_رد	STATETEARDOWN
#وصف_رد	تياري
#وصف_رد	onestate
#وصف_رد	INIT
#وصف_رد	INC
#وصف_رد	ISSTATEIN
#وصف_رد	FWD
#وصف_رد	پٺ_2
#وصف_رد	ISSETBACK
#وصف_رد	SNAMES

/* macros for manipulating states, large version */
#وصف_ڪريو	حالتون	حرف *
#وصف_ڪريو	CLEAR(v)	memset(v, 0, m->g->nstates)
#وصف_ڪريو	SET0(v, n)	((v)[n] = 0)
#وصف_ڪريو	SET1(v, n)	((v)[n] = 1)
#وصف_ڪريو	ISSET(v, n)	((v)[n])
#وصف_ڪريو	سونپيو_3(d, s)	memmove(d, s, m->g->nstates)
#وصف_ڪريو	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#وصف_ڪريو	STATEVARS	ڊگهو vn; حرف *space
#وصف_ڪريو	STATESETUP(m, nv)	{ (m)->space = يادگيري_ڏيو((nv)*(m)->g->nstates); \
				إذا ((m)->space == عدم) أرجع(REG_ESPACE); \
				(m)->vn = 0; }
#وصف_ڪريو	STATETEARDOWN(m)	{ حرر((m)->space); }
#وصف_ڪريو	تياري(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#وصف_ڪريو	onestate	ڊگهو
#وصف_ڪريو	INIT(o, n)	((o) = (n))
#وصف_ڪريو	INC(o)	((o)++)
#وصف_ڪريو	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#وصف_ڪريو	FWD(dst, src, n)	((dst)[هتي_2+(n)] |= (src)[هتي_2])
#وصف_ڪريو	پٺ_2(dst, src, n)	((dst)[هتي_2-(n)] |= (src)[هتي_2])
#وصف_ڪريو	ISSETBACK(v, n)	((v)[هتي_2 - (n)])
/* function names */
#وصف_ڪريو	LNAMES			/* flag */

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
#جيڪڏهن_وصف_ٿيل REDEBUG
#	define	GOODFLAGS(f)	(f)
#وإلا
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#جيڪڏهن_پورو

	إذا (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		أرجع(REG_BADPAT);
	أكد(!(g->iflags&REGEX_BAD));
	إذا (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		أرجع(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	إذا (g->nstates <= (ڊگهو)(CHAR_BIT*حجم(states1)) && !(eflags&REG_LARGE))
		أرجع(smatcher(g, string, nmatch, pmatch, eflags));
	وإلا
		أرجع(lmatcher(g, string, nmatch, pmatch, eflags));
}
