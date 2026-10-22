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
 *	@(#)regfree.c	8.3 (Berkeley) 3/20/94
 */

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "regex_impl.h"

#ଅନ୍ତର୍ଭାଵ "regutils.h"
#ଅନ୍ତର୍ଭାଵ "regex2.h"

/*
 - llvm_regfree - free everything
 */
ଶୂନ୍ୟ
llvm_regfree(llvm_regex_t *preg)
{
	ସଂରଚନା re_guts *g;

	ଯଦି (preg->re_magic != MAGIC1)	/* oops */
		ଫେରନ୍ତୁ;			/* nice to complain, but hard */

	g = preg->re_g;
	ଯଦି (g == ଖାଲି || g->magic != MAGIC2)	/* oops again */
		ଫେରନ୍ତୁ;
	preg->re_magic = 0;		/* mark it invalid */
	g->magic = 0;			/* mark it invalid */

	ଯଦି (g->strip != ଖାଲି)
		ମୁକ୍ତ((ଅକ୍ଷର *)g->strip);
	ଯଦି (g->sets != ଖାଲି)
		ମୁକ୍ତ((ଅକ୍ଷର *)g->sets);
	ଯଦି (g->setbits != ଖାଲି)
		ମୁକ୍ତ((ଅକ୍ଷର *)g->setbits);
	ଯଦି (g->must != ଖାଲି)
		ମୁକ୍ତ(g->must);
	ମୁକ୍ତ((ଅକ୍ଷର *)g);
}
