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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "regex_impl.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "regutils.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ snprintf _snprintf
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *regatoi(ਅਚਲ llvm_regex_t *, ਅੱਖਰ *, ਪੂਰਨ);

ਸਥਿਰ ਬਣਤਰ rerr {
	ਪੂਰਨ ਕੋਡ_2;
	ਅਚਲ ਅੱਖਰ *name;
	ਅਚਲ ਅੱਖਰ *explain;
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
	{ REG_ਖਾਲੀ_ਥਾਵਾਂ,	"REG_EMPTY",	"empty (sub)expression" },
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
llvm_regerror(ਪੂਰਨ errcode, ਅਚਲ llvm_regex_t *preg, ਅੱਖਰ *errbuf, size_t errbuf_ਮਾਪ)
{
	ਬਣਤਰ rerr *r;
	size_t len;
	ਪੂਰਨ ਨਿਸ਼ਾਨਾ = errcode &~ REG_ITOA;
	ਅਚਲ ਅੱਖਰ *s;
	ਅੱਖਰ convbuf[50];

	ਜੇ (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, ਆਕਾਰ convbuf);
	ਨਹੀਂਤਾਂ {
		ਲਈ (r = rerrs; r->ਕੋਡ_2 != 0; r++)
			ਜੇ (r->ਕੋਡ_2 == ਨਿਸ਼ਾਨਾ)
				ਰੋਕੋ;
	
		ਜੇ (errcode&REG_ITOA) {
			ਜੇ (r->ਕੋਡ_2 != 0) {
				ਪ੍ਰਤਿਜ੍ਞਾ(ਲੜੀ_ਲੰਬਾਈ(r->name) < ਆਕਾਰ(convbuf));
				(ਖਾਲੀ) llvm_strlcpy(convbuf, r->name, ਆਕਾਰ convbuf);
			} ਨਹੀਂਤਾਂ
				(ਖਾਲੀ)snprintf(convbuf, ਆਕਾਰ convbuf,
				    "REG_0x%x", ਨਿਸ਼ਾਨਾ);
			s = convbuf;
		} ਨਹੀਂਤਾਂ
			s = r->explain;
	}

	len = ਲੜੀ_ਲੰਬਾਈ(s) + 1;
	ਜੇ (errbuf_ਮਾਪ > 0) {
		llvm_strlcpy(errbuf, s, errbuf_ਮਾਪ);
	}

	ਵਾਪਸ(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
regatoi(ਅਚਲ llvm_regex_t *preg, ਅੱਖਰ *localbuf, ਪੂਰਨ localbufsize)
{
	ਬਣਤਰ rerr *r;

	ਲਈ (r = rerrs; r->ਕੋਡ_2 != 0; r++)
		ਜੇ (strcmp(r->name, preg->re_endp) == 0)
			ਰੋਕੋ;
	ਜੇ (r->ਕੋਡ_2 == 0)
		ਵਾਪਸ("0");

	(ਖਾਲੀ)snprintf(localbuf, localbufsize, "%d", r->ਕੋਡ_2);
	ਵਾਪਸ(localbuf);
}
