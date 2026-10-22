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
 *	@(#)regerror.c	8.4 (Berkeley) 3/20/94
 */

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "regex_impl.h"

#అన్తర్భావ "regutils.h"

#నిర్వచితమైతే _MSC_VER
#నిర్వచించు snprintf _snprintf
#అయితే_ముగింపు

స్థిర స్థిరరాశి అక్షరం *regatoi(స్థిరరాశి llvm_regex_t *, అక్షరం *, పూర్ణాంకం);

స్థిర నిర్మాణం rerr {
	పూర్ణాంకం సంకేతం_3;
	స్థిరరాశి అక్షరం *name;
	స్థిరరాశి అక్షరం *explain;
} rerrs[] = {
	{ REG_NOMATCH,	"REG_NOMATCH",	"llvm_regexec() failed to match" },
	{ REG_BADPAT,	"REG_BADPAT",	"invalid regular expression" },
	{ REG_ECOLLATE,	"REG_ECOLLATE",	"invalid collating element" },
	{ REG_ECTYPE,	"REG_ECTYPE",	"invalid character class" },
	{ REG_EESCAPE,	"REG_EESCAPE",	"trailing backslash (\\)" },
	{ REG_ESUBREG,	"REG_ESUBREG",	"invalid backreference number" },
	{ REG_EBRACK,	"REG_EBRACK",	"brackets ([ ]) not balanced" },
	{ REG_EPAREN,	"REG_EPAREN",	"parentheses not balanced" },
	{ REG_EBRACE,	"REG_EBRACE",	"braces not balanced" },
	{ REG_BADBR,	"REG_BADBR",	"invalid repetition count(s)" },
	{ REG_ERANGE,	"REG_ERANGE",	"invalid character range" },
	{ REG_ESPACE,	"REG_ESPACE",	"out of memory" },
	{ REG_BADRPT,	"REG_BADRPT",	"repetition-operator operand invalid" },
	{ REG_ఖాళీలు,	"REG_EMPTY",	"empty (sub)expression" },
	{ REG_ASSERT,	"REG_ASSERT",	"\"can't happen\" -- you found a bug" },
	{ REG_INVARG,	"REG_INVARG",	"invalid argument to regex routine" },
	{ 0,		"",		"*** unknown regexp error code ***" }
};

/*
 - llvm_regerror - the interface to error numbers
 = extern size_t llvm_regerror(int, const llvm_regex_t *, char *, size_t);
 */
/* ARGSUSED */
size_t
llvm_regerror(పూర్ణాంకం errcode, స్థిరరాశి llvm_regex_t *preg, అక్షరం *errbuf, size_t errbuf_కొలత)
{
	నిర్మాణం rerr *r;
	size_t len;
	పూర్ణాంకం లక్ష్యం = errcode &~ REG_ITOA;
	స్థిరరాశి అక్షరం *s;
	అక్షరం convbuf[50];

	ఒకవేళ (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, పరిమాణం convbuf);
	లేకపోతే {
		కొరకు (r = rerrs; r->సంకేతం_3 != 0; r++)
			ఒకవేళ (r->సంకేతం_3 == లక్ష్యం)
				ఆపు;
	
		ఒకవేళ (errcode&REG_ITOA) {
			ఒకవేళ (r->సంకేతం_3 != 0) {
				ప్రతిజ్ఞా(అక్షరశ్రేణి_పొడవు(r->name) < పరిమాణం(convbuf));
				(శూన్య) llvm_strlcpy(convbuf, r->name, పరిమాణం convbuf);
			} లేకపోతే
				(శూన్య)snprintf(convbuf, పరిమాణం convbuf,
				    "REG_0x%x", లక్ష్యం);
			s = convbuf;
		} లేకపోతే
			s = r->explain;
	}

	len = అక్షరశ్రేణి_పొడవు(s) + 1;
	ఒకవేళ (errbuf_కొలత > 0) {
		llvm_strlcpy(errbuf, s, errbuf_కొలత);
	}

	తిరిగి(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
స్థిర స్థిరరాశి అక్షరం *
regatoi(స్థిరరాశి llvm_regex_t *preg, అక్షరం *localbuf, పూర్ణాంకం localbufsize)
{
	నిర్మాణం rerr *r;

	కొరకు (r = rerrs; r->సంకేతం_3 != 0; r++)
		ఒకవేళ (strcmp(r->name, preg->re_endp) == 0)
			ఆపు;
	ఒకవేళ (r->సంకేతం_3 == 0)
		తిరిగి("0");

	(శూన్య)snprintf(localbuf, localbufsize, "%d", r->సంకేతం_3);
	తిరిగి(localbuf);
}
