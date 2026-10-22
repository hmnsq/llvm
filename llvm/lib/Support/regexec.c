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
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "regex_impl.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "regutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	states1	ਲੰਮਾ		/* for later use in llvm_regexec() decision */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਹਾਲਤਾਂ	states1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CLEAR(v)	((v) = 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SET0(v, n)	((v) &= ~((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << (n)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SET1(v, n)	((v) |= (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSET(v, n)	(((v) & ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << (n))) != 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਸੌਂਪੋ_3(d, s)	((d) = (s))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	EQ(a, b)	((a) == (b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATEVARS	ਲੰਮਾ dummy	/* dummy version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATESETUP(m, n)	/* nothing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATETEARDOWN(m)	/* nothing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਤਿਆਰੀ(v)	((v) = 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	onestate	ਲੰਮਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	INIT(o, n)	((o) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	INC(o)		((o) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(o) << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	FWD(dst, src, n)	((dst) |= ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(src)&(ਇੱਥੇ_2)) << (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਪਿੱਠ_2(dst, src, n)	((dst) |= ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(src)&(ਇੱਥੇ_2)) >> (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSETBACK(v, n)		(((v) & ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਇੱਥੇ_2 >> (n))) != 0)
/* function names */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SNAMES			/* engine.inc looks after details */

#ਅਨ੍ਤਰ੍ਭਾਵ "regengine.inc"

/* now undo things */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ਹਾਲਤਾਂ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	CLEAR
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	SET0
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	SET1
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ISSET
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ਸੌਂਪੋ_3
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	EQ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	STATEVARS
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	STATESETUP
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	STATETEARDOWN
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ਤਿਆਰੀ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	onestate
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	INIT
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	INC
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ISSTATEIN
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	FWD
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ਪਿੱਠ_2
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	ISSETBACK
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ	SNAMES

/* macros for manipulating states, large version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਹਾਲਤਾਂ	ਅੱਖਰ *
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	CLEAR(v)	memset(v, 0, m->g->nstates)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SET0(v, n)	((v)[n] = 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	SET1(v, n)	((v)[n] = 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSET(v, n)	((v)[n])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਸੌਂਪੋ_3(d, s)	memmove(d, s, m->g->nstates)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATEVARS	ਲੰਮਾ vn; ਅੱਖਰ *space
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATESETUP(m, nv)	{ (m)->space = ਸਮ੍ਰਿਤੀ_ਦਿਓ((nv)*(m)->g->nstates); \
				ਜੇ ((m)->space == ਨਿਰਾ) ਵਾਪਸ(REG_ESPACE); \
				(m)->vn = 0; }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	STATETEARDOWN(m)	{ ਮੁਕਤ((m)->space); }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਤਿਆਰੀ(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	onestate	ਲੰਮਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	INIT(o, n)	((o) = (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	INC(o)	((o)++)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	FWD(dst, src, n)	((dst)[ਇੱਥੇ_2+(n)] |= (src)[ਇੱਥੇ_2])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਪਿੱਠ_2(dst, src, n)	((dst)[ਇੱਥੇ_2-(n)] |= (src)[ਇੱਥੇ_2])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ISSETBACK(v, n)	((v)[ਇੱਥੇ_2 - (n)])
/* function names */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	LNAMES			/* flag */

#ਅਨ੍ਤਰ੍ਭਾਵ "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
ਪੂਰਨ				/* 0 success, REG_NOMATCH failure */
llvm_regexec(ਅਚਲ llvm_regex_t *preg, ਅਚਲ ਅੱਖਰ *string, size_t nmatch,
             llvm_regmatch_t pmatch[], ਪੂਰਨ eflags)
{
	ਬਣਤਰ re_guts *g = preg->re_g;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ REDEBUG
#	define	GOODFLAGS(f)	(f)
#ਨਹੀਂਤਾਂ
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#ਜੇ_ਅੰਤ

	ਜੇ (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		ਵਾਪਸ(REG_BADPAT);
	ਪ੍ਰਤਿਜ੍ਞਾ(!(g->iflags&REGEX_BAD));
	ਜੇ (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		ਵਾਪਸ(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	ਜੇ (g->nstates <= (ਲੰਮਾ)(CHAR_BIT*ਆਕਾਰ(states1)) && !(eflags&REG_LARGE))
		ਵਾਪਸ(smatcher(g, string, nmatch, pmatch, eflags));
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ(lmatcher(g, string, nmatch, pmatch, eflags));
}
