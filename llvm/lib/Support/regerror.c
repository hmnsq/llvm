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

#ضمن <sys/types.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ضمن <limits.h>
#ضمن <stdlib.h>
#ضمن "regex_impl.h"

#ضمن "regutils.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

ساكن ثابت حرف *regatoi(ثابت llvm_regex_t *, حرف *, صحيح);

ساكن بنية rerr {
	صحيح code;
	ثابت حرف *name;
	ثابت حرف *explain;
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
llvm_regerror(صحيح errcode, ثابت llvm_regex_t *preg, حرف *errbuf, size_t errbuf_size)
{
	بنية rerr *r;
	size_t len;
	صحيح target = errcode &~ REG_ITOA;
	ثابت حرف *s;
	حرف convbuf[50];

	إذا (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, حجم convbuf);
	وإلا {
		لكل (r = rerrs; r->code != 0; r++)
			إذا (r->code == target)
				اكسر;
	
		إذا (errcode&REG_ITOA) {
			إذا (r->code != 0) {
				أكد(strlen(r->name) < حجم(convbuf));
				(فراغ) llvm_strlcpy(convbuf, r->name, حجم convbuf);
			} وإلا
				(فراغ)snprintf(convbuf, حجم convbuf,
				    "REG_0x%x", target);
			s = convbuf;
		} وإلا
			s = r->explain;
	}

	len = strlen(s) + 1;
	إذا (errbuf_size > 0) {
		llvm_strlcpy(errbuf, s, errbuf_size);
	}

	أرجع(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
ساكن ثابت حرف *
regatoi(ثابت llvm_regex_t *preg, حرف *localbuf, صحيح localbufsize)
{
	بنية rerr *r;

	لكل (r = rerrs; r->code != 0; r++)
		إذا (strcmp(r->name, preg->re_endp) == 0)
			اكسر;
	إذا (r->code == 0)
		أرجع("0");

	(فراغ)snprintf(localbuf, localbufsize, "%d", r->code);
	أرجع(localbuf);
}
