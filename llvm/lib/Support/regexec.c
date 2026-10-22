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
#شامل <sys/types.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <limits.h>
#شامل <ctype.h>
#شامل "regex_impl.h"

#شامل "regutils.h"
#شامل "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#تعریف_کرو	states1	طویل		/* for later use in llvm_regexec() decision */
#تعریف_کرو	حالتیں	states1
#تعریف_کرو	CLEAR(v)	((v) = 0)
#تعریف_کرو	SET0(v, n)	((v) &= ~((غیرعلامتی طویل)1 << (n)))
#تعریف_کرو	SET1(v, n)	((v) |= (غیرعلامتی طویل)1 << (n))
#تعریف_کرو	ISSET(v, n)	(((v) & ((غیرعلامتی طویل)1 << (n))) != 0)
#تعریف_کرو	تفویض_3(d, s)	((d) = (s))
#تعریف_کرو	EQ(a, b)	((a) == (b))
#تعریف_کرو	STATEVARS	طویل dummy	/* dummy version */
#تعریف_کرو	STATESETUP(m, n)	/* nothing */
#تعریف_کرو	STATETEARDOWN(m)	/* nothing */
#تعریف_کرو	تیاری(v)	((v) = 0)
#تعریف_کرو	onestate	طویل
#تعریف_کرو	INIT(o, n)	((o) = (غیرعلامتی طویل)1 << (n))
#تعریف_کرو	INC(o)		((o) = (غیرعلامتی طویل)(o) << 1)
#تعریف_کرو	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#تعریف_کرو	FWD(dst, src, n)	((dst) |= ((غیرعلامتی طویل)(src)&(یہاں_2)) << (n))
#تعریف_کرو	عقب_2(dst, src, n)	((dst) |= ((غیرعلامتی طویل)(src)&(یہاں_2)) >> (n))
#تعریف_کرو	ISSETBACK(v, n)		(((v) & ((غیرعلامتی طویل)یہاں_2 >> (n))) != 0)
/* function names */
#تعریف_کرو SNAMES			/* engine.inc looks after details */

#شامل "regengine.inc"

/* now undo things */
#تعریف_منسوخ	حالتیں
#تعریف_منسوخ	CLEAR
#تعریف_منسوخ	SET0
#تعریف_منسوخ	SET1
#تعریف_منسوخ	ISSET
#تعریف_منسوخ	تفویض_3
#تعریف_منسوخ	EQ
#تعریف_منسوخ	STATEVARS
#تعریف_منسوخ	STATESETUP
#تعریف_منسوخ	STATETEARDOWN
#تعریف_منسوخ	تیاری
#تعریف_منسوخ	onestate
#تعریف_منسوخ	INIT
#تعریف_منسوخ	INC
#تعریف_منسوخ	ISSTATEIN
#تعریف_منسوخ	FWD
#تعریف_منسوخ	عقب_2
#تعریف_منسوخ	ISSETBACK
#تعریف_منسوخ	SNAMES

/* macros for manipulating states, large version */
#تعریف_کرو	حالتیں	حرف *
#تعریف_کرو	CLEAR(v)	memset(v, 0, m->g->nstates)
#تعریف_کرو	SET0(v, n)	((v)[n] = 0)
#تعریف_کرو	SET1(v, n)	((v)[n] = 1)
#تعریف_کرو	ISSET(v, n)	((v)[n])
#تعریف_کرو	تفویض_3(d, s)	memmove(d, s, m->g->nstates)
#تعریف_کرو	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#تعریف_کرو	STATEVARS	طویل vn; حرف *space
#تعریف_کرو	STATESETUP(m, nv)	{ (m)->space = حافظہ_دو((nv)*(m)->g->nstates); \
				اگر ((m)->space == تہی) واپس(REG_ESPACE); \
				(m)->vn = 0; }
#تعریف_کرو	STATETEARDOWN(m)	{ آزاد((m)->space); }
#تعریف_کرو	تیاری(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#تعریف_کرو	onestate	طویل
#تعریف_کرو	INIT(o, n)	((o) = (n))
#تعریف_کرو	INC(o)	((o)++)
#تعریف_کرو	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#تعریف_کرو	FWD(dst, src, n)	((dst)[یہاں_2+(n)] |= (src)[یہاں_2])
#تعریف_کرو	عقب_2(dst, src, n)	((dst)[یہاں_2-(n)] |= (src)[یہاں_2])
#تعریف_کرو	ISSETBACK(v, n)	((v)[یہاں_2 - (n)])
/* function names */
#تعریف_کرو	LNAMES			/* flag */

#شامل "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
صحیح				/* 0 success, REG_NOMATCH failure */
llvm_regexec(ثابت llvm_regex_t *preg, ثابت حرف *string, size_t nmatch,
             llvm_regmatch_t pmatch[], صحیح eflags)
{
	ساخت re_guts *g = preg->re_g;
#اگر_تعریف_شدہ REDEBUG
#	define	GOODFLAGS(f)	(f)
#ورنہ
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#اگر_ختم

	اگر (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		واپس(REG_BADPAT);
	أكد(!(g->iflags&REGEX_BAD));
	اگر (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		واپس(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	اگر (g->nstates <= (طویل)(CHAR_BIT*حجم(states1)) && !(eflags&REG_LARGE))
		واپس(smatcher(g, string, nmatch, pmatch, eflags));
	ورنہ
		واپس(lmatcher(g, string, nmatch, pmatch, eflags));
}
