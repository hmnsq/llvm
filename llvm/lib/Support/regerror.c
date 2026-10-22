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

#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব "regex_impl.h"

#অন্তর্ভাব "regutils.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

স্থির ধ্রুবক অক্ষর *regatoi(ধ্রুবক llvm_regex_t *, অক্ষর *, পূর্ণসংখ্যা);

স্থির কাঠামো rerr {
	পূর্ণসংখ্যা code;
	ধ্রুবক অক্ষর *name;
	ধ্রুবক অক্ষর *explain;
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
llvm_regerror(পূর্ণসংখ্যা errcode, ধ্রুবক llvm_regex_t *preg, অক্ষর *errbuf, size_t errbuf_আকার)
{
	কাঠামো rerr *r;
	size_t len;
	পূর্ণসংখ্যা target = errcode &~ REG_ITOA;
	ধ্রুবক অক্ষর *s;
	অক্ষর convbuf[50];

	যদি (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, আকার convbuf);
	অন্যথা {
		জন্য (r = rerrs; r->code != 0; r++)
			যদি (r->code == target)
				বিরতি;
	
		যদি (errcode&REG_ITOA) {
			যদি (r->code != 0) {
				প্রতিজ্ঞা(strlen(r->name) < আকার(convbuf));
				(শূন্য) llvm_strlcpy(convbuf, r->name, আকার convbuf);
			} অন্যথা
				(শূন্য)snprintf(convbuf, আকার convbuf,
				    "REG_0x%x", target);
			s = convbuf;
		} অন্যথা
			s = r->explain;
	}

	len = strlen(s) + 1;
	যদি (errbuf_আকার > 0) {
		llvm_strlcpy(errbuf, s, errbuf_আকার);
	}

	ফেরত(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
স্থির ধ্রুবক অক্ষর *
regatoi(ধ্রুবক llvm_regex_t *preg, অক্ষর *localbuf, পূর্ণসংখ্যা localbufsize)
{
	কাঠামো rerr *r;

	জন্য (r = rerrs; r->code != 0; r++)
		যদি (strcmp(r->name, preg->re_endp) == 0)
			বিরতি;
	যদি (r->code == 0)
		ফেরত("0");

	(শূন্য)snprintf(localbuf, localbufsize, "%d", r->code);
	ফেরত(localbuf);
}
