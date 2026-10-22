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

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "regex_impl.h"

#అన్తర్భావ "regutils.h"
#అన్తర్భావ "regex2.h"

/*
 - llvm_regfree - free everything
 */
శూన్య
llvm_regfree(llvm_regex_t *preg)
{
	నిర్మాణం re_guts *g;

	ఒకవేళ (preg->re_magic != MAGIC1)	/* oops */
		తిరిగి;			/* nice to complain, but hard */

	g = preg->re_g;
	ఒకవేళ (g == ఖాళీ || g->magic != MAGIC2)	/* oops again */
		తిరిగి;
	preg->re_magic = 0;		/* mark it invalid */
	g->magic = 0;			/* mark it invalid */

	ఒకవేళ (g->strip != ఖాళీ)
		విడుదల((అక్షరం *)g->strip);
	ఒకవేళ (g->sets != ఖాళీ)
		విడుదల((అక్షరం *)g->sets);
	ఒకవేళ (g->setbits != ఖాళీ)
		విడుదల((అక్షరం *)g->setbits);
	ఒకవేళ (g->must != ఖాళీ)
		విడుదల(g->must);
	విడుదల((అక్షరం *)g);
}
