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
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव "regex_impl.h"

#अन्तर्भाव "regutils.h"
#अन्तर्भाव "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#define	states1	दीर्घ		/* for later use in llvm_regexec() decision */
#define	states	states1
#define	CLEAR(v)	((v) = 0)
#define	SET0(v, n)	((v) &= ~((अचिन्हित दीर्घ)1 << (n)))
#define	SET1(v, n)	((v) |= (अचिन्हित दीर्घ)1 << (n))
#define	ISSET(v, n)	(((v) & ((अचिन्हित दीर्घ)1 << (n))) != 0)
#define	ASSIGN(d, s)	((d) = (s))
#define	EQ(a, b)	((a) == (b))
#define	STATEVARS	दीर्घ dummy	/* dummy version */
#define	STATESETUP(m, n)	/* nothing */
#define	STATETEARDOWN(m)	/* nothing */
#define	SETUP(v)	((v) = 0)
#define	onestate	दीर्घ
#define	INIT(o, n)	((o) = (अचिन्हित दीर्घ)1 << (n))
#define	INC(o)		((o) = (अचिन्हित दीर्घ)(o) << 1)
#define	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#define	FWD(dst, src, n)	((dst) |= ((अचिन्हित दीर्घ)(src)&(here)) << (n))
#define	BACK(dst, src, n)	((dst) |= ((अचिन्हित दीर्घ)(src)&(here)) >> (n))
#define	ISSETBACK(v, n)		(((v) & ((अचिन्हित दीर्घ)here >> (n))) != 0)
/* function names */
#define SNAMES			/* engine.inc looks after details */

#अन्तर्भाव "regengine.inc"

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
#define	states	अक्षर *
#define	CLEAR(v)	स_रखो(v, 0, m->g->nstates)
#define	SET0(v, n)	((v)[n] = 0)
#define	SET1(v, n)	((v)[n] = 1)
#define	ISSET(v, n)	((v)[n])
#define	ASSIGN(d, s)	स_हटाओ(d, s, m->g->nstates)
#define	EQ(a, b)	(स_भेद(a, b, m->g->nstates) == 0)
#define	STATEVARS	दीर्घ vn; अक्षर *space
#define	STATESETUP(m, nv)	{ (m)->space = दो_स्मृति((nv)*(m)->g->nstates); \
				जर ((m)->space == रिक्त) परत(REG_ESPACE); \
				(m)->vn = 0; }
#define	STATETEARDOWN(m)	{ मुक्त((m)->space); }
#define	SETUP(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#define	onestate	दीर्घ
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

#अन्तर्भाव "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
पूर्णांक				/* 0 success, REG_NOMATCH failure */
llvm_regexec(स्थिरांक llvm_regex_t *preg, स्थिरांक अक्षर *string, माप_प्रकार nmatch,
             llvm_regmatch_t pmatch[], पूर्णांक eflags)
{
	संरचना re_guts *g = preg->re_g;
#ifdef REDEBUG
#	define	GOODFLAGS(f)	(f)
#अन्यथा
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#endif

	जर (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		परत(REG_BADPAT);
	प्रतिज्ञा(!(g->iflags&REGEX_BAD));
	जर (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		परत(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	जर (g->nstates <= (दीर्घ)(अक्षर_बिट*आकार(states1)) && !(eflags&REG_LARGE))
		परत(smatcher(g, string, nmatch, pmatch, eflags));
	अन्यथा
		परत(lmatcher(g, string, nmatch, pmatch, eflags));
}
