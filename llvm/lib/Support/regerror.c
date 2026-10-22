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

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "regex_impl.h"

#ಅನ್ತರ್ಭಾವ "regutils.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *regatoi(ಧ್ರುವ llvm_regex_t *, ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ);

ಸ್ಥಿರ ಸಂರಚನಾ rerr {
	ಪೂರ್ಣಾಙ್ಕ code;
	ಧ್ರುವ ವರ್ಣ *name;
	ಧ್ರುವ ವರ್ಣ *explain;
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
	{ REG_EMPTY,	"REG_EMPTY",	"empty (sub)expression" },
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
llvm_regerror(ಪೂರ್ಣಾಙ್ಕ errcode, ಧ್ರುವ llvm_regex_t *preg, ವರ್ಣ *errbuf, size_t errbuf_size)
{
	ಸಂರಚನಾ rerr *r;
	size_t len;
	ಪೂರ್ಣಾಙ್ಕ target = errcode &~ REG_ITOA;
	ಧ್ರುವ ವರ್ಣ *s;
	ವರ್ಣ convbuf[50];

	ಯದಿ (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, ಪರಿಮಾಣ convbuf);
	ಅನ್ಯಥಾ {
		ಪ್ರತಿ (r = rerrs; r->code != 0; r++)
			ಯದಿ (r->code == target)
				ಭಙ್ಗ;
	
		ಯದಿ (errcode&REG_ITOA) {
			ಯದಿ (r->code != 0) {
				ಪ್ರತಿಜ್ಞಾ(strlen(r->name) < ಪರಿಮಾಣ(convbuf));
				(ಶೂನ್ಯ) llvm_strlcpy(convbuf, r->name, ಪರಿಮಾಣ convbuf);
			} ಅನ್ಯಥಾ
				(ಶೂನ್ಯ)snprintf(convbuf, ಪರಿಮಾಣ convbuf,
				    "REG_0x%x", target);
			s = convbuf;
		} ಅನ್ಯಥಾ
			s = r->explain;
	}

	len = strlen(s) + 1;
	ಯದಿ (errbuf_size > 0) {
		llvm_strlcpy(errbuf, s, errbuf_size);
	}

	ಪ್ರತ್ಯಾಗಮ(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
regatoi(ಧ್ರುವ llvm_regex_t *preg, ವರ್ಣ *localbuf, ಪೂರ್ಣಾಙ್ಕ localbufsize)
{
	ಸಂರಚನಾ rerr *r;

	ಪ್ರತಿ (r = rerrs; r->code != 0; r++)
		ಯದಿ (strcmp(r->name, preg->re_endp) == 0)
			ಭಙ್ಗ;
	ಯದಿ (r->code == 0)
		ಪ್ರತ್ಯಾಗಮ("0");

	(ಶೂನ್ಯ)snprintf(localbuf, localbufsize, "%d", r->code);
	ಪ್ರತ್ಯಾಗಮ(localbuf);
}
