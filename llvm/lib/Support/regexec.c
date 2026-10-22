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
#परिभाषय	states1	दीर्घ		/* for later use in llvm_regexec() decision */
#परिभाषय	अवस्थाः	states1
#परिभाषय	CLEAR(v)	((v) = 0)
#परिभाषय	SET0(v, n)	((v) &= ~((अचिह्नित दीर्घ)1 << (n)))
#परिभाषय	SET1(v, n)	((v) |= (अचिह्नित दीर्घ)1 << (n))
#परिभाषय	ISSET(v, n)	(((v) & ((अचिह्नित दीर्घ)1 << (n))) != 0)
#परिभाषय	नियोजय_3(d, s)	((d) = (s))
#परिभाषय	EQ(a, b)	((a) == (b))
#परिभाषय	STATEVARS	दीर्घ dummy	/* dummy version */
#परिभाषय	STATESETUP(m, n)	/* nothing */
#परिभाषय	STATETEARDOWN(m)	/* nothing */
#परिभाषय	सज्जता(v)	((v) = 0)
#परिभाषय	onestate	दीर्घ
#परिभाषय	INIT(o, n)	((o) = (अचिह्नित दीर्घ)1 << (n))
#परिभाषय	INC(o)		((o) = (अचिह्नित दीर्घ)(o) << 1)
#परिभाषय	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#परिभाषय	FWD(dst, src, n)	((dst) |= ((अचिह्नित दीर्घ)(src)&(अत्र_2)) << (n))
#परिभाषय	पृष्ठभागः_2(dst, src, n)	((dst) |= ((अचिह्नित दीर्घ)(src)&(अत्र_2)) >> (n))
#परिभाषय	ISSETBACK(v, n)		(((v) & ((अचिह्नित दीर्घ)अत्र_2 >> (n))) != 0)
/* function names */
#परिभाषय SNAMES			/* engine.inc looks after details */

#अन्तर्भाव "regengine.inc"

/* now undo things */
#अपरिभाषय	अवस्थाः
#अपरिभाषय	CLEAR
#अपरिभाषय	SET0
#अपरिभाषय	SET1
#अपरिभाषय	ISSET
#अपरिभाषय	नियोजय_3
#अपरिभाषय	EQ
#अपरिभाषय	STATEVARS
#अपरिभाषय	STATESETUP
#अपरिभाषय	STATETEARDOWN
#अपरिभाषय	सज्जता
#अपरिभाषय	onestate
#अपरिभाषय	INIT
#अपरिभाषय	INC
#अपरिभाषय	ISSTATEIN
#अपरिभाषय	FWD
#अपरिभाषय	पृष्ठभागः_2
#अपरिभाषय	ISSETBACK
#अपरिभाषय	SNAMES

/* macros for manipulating states, large version */
#परिभाषय	अवस्थाः	वर्ण *
#परिभाषय	CLEAR(v)	स्मृतिं_पूरय(v, 0, m->g->nstates)
#परिभाषय	SET0(v, n)	((v)[n] = 0)
#परिभाषय	SET1(v, n)	((v)[n] = 1)
#परिभाषय	ISSET(v, n)	((v)[n])
#परिभाषय	नियोजय_3(d, s)	memmove(d, s, m->g->nstates)
#परिभाषय	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#परिभाषय	STATEVARS	दीर्घ vn; वर्ण *space
#परिभाषय	STATESETUP(m, nv)	{ (m)->space = स्मृतिं_देहि((nv)*(m)->g->nstates); \
				यदि ((m)->space == रिक्त) प्रत्यागम(REG_ESPACE); \
				(m)->vn = 0; }
#परिभाषय	STATETEARDOWN(m)	{ मुक्त((m)->space); }
#परिभाषय	सज्जता(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#परिभाषय	onestate	दीर्घ
#परिभाषय	INIT(o, n)	((o) = (n))
#परिभाषय	INC(o)	((o)++)
#परिभाषय	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#परिभाषय	FWD(dst, src, n)	((dst)[अत्र_2+(n)] |= (src)[अत्र_2])
#परिभाषय	पृष्ठभागः_2(dst, src, n)	((dst)[अत्र_2-(n)] |= (src)[अत्र_2])
#परिभाषय	ISSETBACK(v, n)	((v)[अत्र_2 - (n)])
/* function names */
#परिभाषय	LNAMES			/* flag */

#अन्तर्भाव "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
पूर्णाङ्क				/* 0 success, REG_NOMATCH failure */
llvm_regexec(ध्रुव llvm_regex_t *preg, ध्रुव वर्ण *string, size_t nmatch,
             llvm_regmatch_t pmatch[], पूर्णाङ्क eflags)
{
	संरचना re_guts *g = preg->re_g;
#यदि_परिभाषितम् REDEBUG
#	define	GOODFLAGS(f)	(f)
#अन्यथा
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#यदि_अन्तः

	यदि (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		प्रत्यागम(REG_BADPAT);
	प्रतिज्ञा(!(g->iflags&REGEX_BAD));
	यदि (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		प्रत्यागम(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	यदि (g->nstates <= (दीर्घ)(CHAR_BIT*परिमाण(states1)) && !(eflags&REG_LARGE))
		प्रत्यागम(smatcher(g, string, nmatch, pmatch, eflags));
	अन्यथा
		प्रत्यागम(lmatcher(g, string, nmatch, pmatch, eflags));
}
