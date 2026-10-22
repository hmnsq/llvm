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
#समावेश <sys/types.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <limits.h>
#समावेश <ctype.h>
#समावेश "regex_impl.h"

#समावेश "regutils.h"
#समावेश "regex2.h"

/* macros for manipulating states, small version */
/* FIXME: 'states' is assumed as 'long' on small version. */
#घोषणा	states1	दीर्घ		/* for later use in llvm_regexec() decision */
#घोषणा	states	states1
#घोषणा	CLEAR(v)	((v) = 0)
#घोषणा	SET0(v, n)	((v) &= ~((अचिन्हित दीर्घ)1 << (n)))
#घोषणा	SET1(v, n)	((v) |= (अचिन्हित दीर्घ)1 << (n))
#घोषणा	ISSET(v, n)	(((v) & ((अचिन्हित दीर्घ)1 << (n))) != 0)
#घोषणा	ASSIGN(d, s)	((d) = (s))
#घोषणा	EQ(a, b)	((a) == (b))
#घोषणा	STATEVARS	दीर्घ dummy	/* dummy version */
#घोषणा	STATESETUP(m, n)	/* nothing */
#घोषणा	STATETEARDOWN(m)	/* nothing */
#घोषणा	SETUP(v)	((v) = 0)
#घोषणा	onestate	दीर्घ
#घोषणा	INIT(o, n)	((o) = (अचिन्हित दीर्घ)1 << (n))
#घोषणा	INC(o)		((o) = (अचिन्हित दीर्घ)(o) << 1)
#घोषणा	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#घोषणा	FWD(dst, src, n)	((dst) |= ((अचिन्हित दीर्घ)(src)&(here)) << (n))
#घोषणा	BACK(dst, src, n)	((dst) |= ((अचिन्हित दीर्घ)(src)&(here)) >> (n))
#घोषणा	ISSETBACK(v, n)		(((v) & ((अचिन्हित दीर्घ)here >> (n))) != 0)
/* function names */
#घोषणा SNAMES			/* engine.inc looks after details */

#समावेश "regengine.inc"

/* now undo things */
#अघोषित	states
#अघोषित	CLEAR
#अघोषित	SET0
#अघोषित	SET1
#अघोषित	ISSET
#अघोषित	ASSIGN
#अघोषित	EQ
#अघोषित	STATEVARS
#अघोषित	STATESETUP
#अघोषित	STATETEARDOWN
#अघोषित	SETUP
#अघोषित	onestate
#अघोषित	INIT
#अघोषित	INC
#अघोषित	ISSTATEIN
#अघोषित	FWD
#अघोषित	BACK
#अघोषित	ISSETBACK
#अघोषित	SNAMES

/* macros for manipulating states, large version */
#घोषणा	states	अक्षर *
#घोषणा	CLEAR(v)	स_रखो(v, 0, m->g->nstates)
#घोषणा	SET0(v, n)	((v)[n] = 0)
#घोषणा	SET1(v, n)	((v)[n] = 1)
#घोषणा	ISSET(v, n)	((v)[n])
#घोषणा	ASSIGN(d, s)	स_हटाओ(d, s, m->g->nstates)
#घोषणा	EQ(a, b)	(स_भेद(a, b, m->g->nstates) == 0)
#घोषणा	STATEVARS	दीर्घ vn; अक्षर *space
#घोषणा	STATESETUP(m, nv)	{ (m)->space = दो_स्मृति((nv)*(m)->g->nstates); \
				अगर ((m)->space == शून्य) वापस(REG_ESPACE); \
				(m)->vn = 0; }
#घोषणा	STATETEARDOWN(m)	{ मुक्त((m)->space); }
#घोषणा	SETUP(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#घोषणा	onestate	दीर्घ
#घोषणा	INIT(o, n)	((o) = (n))
#घोषणा	INC(o)	((o)++)
#घोषणा	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#घोषणा	FWD(dst, src, n)	((dst)[here+(n)] |= (src)[here])
#घोषणा	BACK(dst, src, n)	((dst)[here-(n)] |= (src)[here])
#घोषणा	ISSETBACK(v, n)	((v)[here - (n)])
/* function names */
#घोषणा	LNAMES			/* flag */

#समावेश "regengine.inc"

/*
 - llvm_regexec - interface for matching
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
पूर्णांक				/* 0 success, REG_NOMATCH failure */
llvm_regexec(स्थिर llvm_regex_t *preg, स्थिर अक्षर *string, माप_प्रकार nmatch,
             llvm_regmatch_t pmatch[], पूर्णांक eflags)
{
	काष्ठा re_guts *g = preg->re_g;
#अगर_घोषित REDEBUG
#	define	GOODFLAGS(f)	(f)
#अन्यथा
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#पूर्ण_अगर

	अगर (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		वापस(REG_BADPAT);
	निश्चित(!(g->iflags&REGEX_BAD));
	अगर (g->iflags&REGEX_BAD)		/* backstop for no-debug case */
		वापस(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	अगर (g->nstates <= (दीर्घ)(अक्षर_बिट*माप(states1)) && !(eflags&REG_LARGE))
		वापस(smatcher(g, string, nmatch, pmatch, eflags));
	अन्यथा
		वापस(lmatcher(g, string, nmatch, pmatch, eflags));
}
