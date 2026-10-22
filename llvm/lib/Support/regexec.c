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
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ "regex_impl.h"

#ಅನ್ತರ್ಭಾವ "regutils.h"
#ಅನ್ತರ್ಭಾವ "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#ವ್ಯಾಖ್ಯಾನಿಸು	states1	ದೀರ್ಘ		/* for later use in llvm_regexec() decision */
#ವ್ಯಾಖ್ಯಾನಿಸು	ಸ್ಥಿತಿಗಳು	states1
#ವ್ಯಾಖ್ಯಾನಿಸು	CLEAR(v)	((v) = 0)
#ವ್ಯಾಖ್ಯಾನಿಸು	SET0(v, n)	((v) &= ~((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)1 << (n)))
#ವ್ಯಾಖ್ಯಾನಿಸು	SET1(v, n)	((v) |= (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)1 << (n))
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSET(v, n)	(((v) & ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)1 << (n))) != 0)
#ವ್ಯಾಖ್ಯಾನಿಸು	ನಿಯೋಜಿಸು_3(d, s)	((d) = (s))
#ವ್ಯಾಖ್ಯಾನಿಸು	EQ(a, b)	((a) == (b))
#ವ್ಯಾಖ್ಯಾನಿಸು	STATEVARS	ದೀರ್ಘ dummy	/* dummy version */
#ವ್ಯಾಖ್ಯಾನಿಸು	STATESETUP(m, n)	/* nothing */
#ವ್ಯಾಖ್ಯಾನಿಸು	STATETEARDOWN(m)	/* nothing */
#ವ್ಯಾಖ್ಯಾನಿಸು	ಸಿದ್ಧತೆ(v)	((v) = 0)
#ವ್ಯಾಖ್ಯಾನಿಸು	onestate	ದೀರ್ಘ
#ವ್ಯಾಖ್ಯಾನಿಸು	INIT(o, n)	((o) = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)1 << (n))
#ವ್ಯಾಖ್ಯಾನಿಸು	INC(o)		((o) = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(o) << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#ವ್ಯಾಖ್ಯಾನಿಸು	FWD(dst, src, n)	((dst) |= ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(src)&(ಇಲ್ಲಿ_2)) << (n))
#ವ್ಯಾಖ್ಯಾನಿಸು	ಹಿಂಭಾಗ_2(dst, src, n)	((dst) |= ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(src)&(ಇಲ್ಲಿ_2)) >> (n))
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSETBACK(v, n)		(((v) & ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಇಲ್ಲಿ_2 >> (n))) != 0)
/* function names */
#ವ್ಯಾಖ್ಯಾನಿಸು SNAMES			/* engine.inc looks after details */

#ಅನ್ತರ್ಭಾವ "regengine.inc"

/* now undo things */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ಸ್ಥಿತಿಗಳು
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	CLEAR
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	SET0
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	SET1
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ISSET
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ನಿಯೋಜಿಸು_3
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	EQ
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	STATEVARS
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	STATESETUP
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	STATETEARDOWN
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ಸಿದ್ಧತೆ
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	onestate
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	INIT
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	INC
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ISSTATEIN
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	FWD
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ಹಿಂಭಾಗ_2
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	ISSETBACK
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು	SNAMES

/* macros for manipulating states, large version */
#ವ್ಯಾಖ್ಯಾನಿಸು	ಸ್ಥಿತಿಗಳು	ಅಕ್ಷರ *
#ವ್ಯಾಖ್ಯಾನಿಸು	CLEAR(v)	memset(v, 0, m->g->nstates)
#ವ್ಯಾಖ್ಯಾನಿಸು	SET0(v, n)	((v)[n] = 0)
#ವ್ಯಾಖ್ಯಾನಿಸು	SET1(v, n)	((v)[n] = 1)
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSET(v, n)	((v)[n])
#ವ್ಯಾಖ್ಯಾನಿಸು	ನಿಯೋಜಿಸು_3(d, s)	memmove(d, s, m->g->nstates)
#ವ್ಯಾಖ್ಯಾನಿಸು	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#ವ್ಯಾಖ್ಯಾನಿಸು	STATEVARS	ದೀರ್ಘ vn; ಅಕ್ಷರ *space
#ವ್ಯಾಖ್ಯಾನಿಸು	STATESETUP(m, nv)	{ (m)->space = ಸ್ಮರಣೆ_ಕೊಡು((nv)*(m)->g->nstates); \
				ಆದರೆ ((m)->space == ಖಾಲಿ) ಹಿಂತಿರುಗಿ(REG_ESPACE); \
				(m)->vn = 0; }
#ವ್ಯಾಖ್ಯಾನಿಸು	STATETEARDOWN(m)	{ ಬಿಡುಗಡೆ((m)->space); }
#ವ್ಯಾಖ್ಯಾನಿಸು	ಸಿದ್ಧತೆ(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#ವ್ಯಾಖ್ಯಾನಿಸು	onestate	ದೀರ್ಘ
#ವ್ಯಾಖ್ಯಾನಿಸು	INIT(o, n)	((o) = (n))
#ವ್ಯಾಖ್ಯಾನಿಸು	INC(o)	((o)++)
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#ವ್ಯಾಖ್ಯಾನಿಸು	FWD(dst, src, n)	((dst)[ಇಲ್ಲಿ_2+(n)] |= (src)[ಇಲ್ಲಿ_2])
#ವ್ಯಾಖ್ಯಾನಿಸು	ಹಿಂಭಾಗ_2(dst, src, n)	((dst)[ಇಲ್ಲಿ_2-(n)] |= (src)[ಇಲ್ಲಿ_2])
#ವ್ಯಾಖ್ಯಾನಿಸು	ISSETBACK(v, n)	((v)[ಇಲ್ಲಿ_2 - (n)])
/* function names */
#ವ್ಯಾಖ್ಯಾನಿಸು	LNAMES			/* flag */

#ಅನ್ತರ್ಭಾವ "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
ಪೂರ್ಣಾಂಕ				/* 0 success, REG_NOMATCH failure */
llvm_regexec(ಸ್ಥಿರಾಂಕ llvm_regex_t *preg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string, size_t nmatch,
             llvm_regmatch_t pmatch[], ಪೂರ್ಣಾಂಕ eflags)
{
	ರಚನೆ re_guts *g = preg->re_g;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ REDEBUG
#	define	GOODFLAGS(f)	(f)
#ಇಲ್ಲದಿದ್ದರೆ
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		ಹಿಂತಿರುಗಿ(REG_BADPAT);
	ಪ್ರತಿಜ್ಞಾ(!(g->iflags&REGEX_BAD));
	ಆದರೆ (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		ಹಿಂತಿರುಗಿ(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	ಆದರೆ (g->nstates <= (ದೀರ್ಘ)(CHAR_BIT*ಗಾತ್ರ(states1)) && !(eflags&REG_LARGE))
		ಹಿಂತಿರುಗಿ(smatcher(g, string, nmatch, pmatch, eflags));
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ(lmatcher(g, string, nmatch, pmatch, eflags));
}
