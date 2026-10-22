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

#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "regex_impl.h"

#അന്തര്ഭാവ "regutils.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

സ്ഥിതം സ്ഥിരം അക്ഷരം *regatoi(സ്ഥിരം llvm_regex_t *, അക്ഷരം *, പൂർണ്ണസംഖ്യ);

സ്ഥിതം ഘടന rerr {
	പൂർണ്ണസംഖ്യ code;
	സ്ഥിരം അക്ഷരം *name;
	സ്ഥിരം അക്ഷരം *explain;
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
llvm_regerror(പൂർണ്ണസംഖ്യ errcode, സ്ഥിരം llvm_regex_t *preg, അക്ഷരം *errbuf, size_t errbuf_size)
{
	ഘടന rerr *r;
	size_t len;
	പൂർണ്ണസംഖ്യ target = errcode &~ REG_ITOA;
	സ്ഥിരം അക്ഷരം *s;
	അക്ഷരം convbuf[50];

	എങ്കിൽ (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, വലുപ്പം convbuf);
	അല്ലെങ്കിൽ {
		വേണ്ടി (r = rerrs; r->code != 0; r++)
			എങ്കിൽ (r->code == target)
				നിർത്തുക;
	
		എങ്കിൽ (errcode&REG_ITOA) {
			എങ്കിൽ (r->code != 0) {
				പ്രതിജ്ഞാ(strlen(r->name) < വലുപ്പം(convbuf));
				(ശൂന്യം) llvm_strlcpy(convbuf, r->name, വലുപ്പം convbuf);
			} അല്ലെങ്കിൽ
				(ശൂന്യം)snprintf(convbuf, വലുപ്പം convbuf,
				    "REG_0x%x", target);
			s = convbuf;
		} അല്ലെങ്കിൽ
			s = r->explain;
	}

	len = strlen(s) + 1;
	എങ്കിൽ (errbuf_size > 0) {
		llvm_strlcpy(errbuf, s, errbuf_size);
	}

	തിരികെ(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
സ്ഥിതം സ്ഥിരം അക്ഷരം *
regatoi(സ്ഥിരം llvm_regex_t *preg, അക്ഷരം *localbuf, പൂർണ്ണസംഖ്യ localbufsize)
{
	ഘടന rerr *r;

	വേണ്ടി (r = rerrs; r->code != 0; r++)
		എങ്കിൽ (strcmp(r->name, preg->re_endp) == 0)
			നിർത്തുക;
	എങ്കിൽ (r->code == 0)
		തിരികെ("0");

	(ശൂന്യം)snprintf(localbuf, localbufsize, "%d", r->code);
	തിരികെ(localbuf);
}
