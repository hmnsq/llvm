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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ವ್ಯಾಖ್ಯಾನಿಸು snprintf _snprintf
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *regatoi(ಸ್ಥಿರಾಂಕ llvm_regex_t *, ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);

ಸ್ಥಿರ ರಚನೆ rerr {
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_3;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *explain;
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
	{ REG_ಖಾಲಿಗಳು,	"REG_EMPTY",	"empty (sub)expression" },
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
llvm_regerror(ಪೂರ್ಣಾಂಕ errcode, ಸ್ಥಿರಾಂಕ llvm_regex_t *preg, ಅಕ್ಷರ *errbuf, size_t errbuf_ಗಾತ್ರ)
{
	ರಚನೆ rerr *r;
	size_t len;
	ಪೂರ್ಣಾಂಕ ಗುರಿ = errcode &~ REG_ITOA;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
	ಅಕ್ಷರ convbuf[50];

	ಆದರೆ (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, ಗಾತ್ರ convbuf);
	ಇಲ್ಲದಿದ್ದರೆ {
		ಗಾಗಿ (r = rerrs; r->ಸಂಕೇತ_3 != 0; r++)
			ಆದರೆ (r->ಸಂಕೇತ_3 == ಗುರಿ)
				ನಿಲ್ಲಿಸು;
	
		ಆದರೆ (errcode&REG_ITOA) {
			ಆದರೆ (r->ಸಂಕೇತ_3 != 0) {
				ಪ್ರತಿಜ್ಞಾ(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(r->name) < ಗಾತ್ರ(convbuf));
				(ಶೂನ್ಯ) llvm_strlcpy(convbuf, r->name, ಗಾತ್ರ convbuf);
			} ಇಲ್ಲದಿದ್ದರೆ
				(ಶೂನ್ಯ)snprintf(convbuf, ಗಾತ್ರ convbuf,
				    "REG_0x%x", ಗುರಿ);
			s = convbuf;
		} ಇಲ್ಲದಿದ್ದರೆ
			s = r->explain;
	}

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) + 1;
	ಆದರೆ (errbuf_ಗಾತ್ರ > 0) {
		llvm_strlcpy(errbuf, s, errbuf_ಗಾತ್ರ);
	}

	ಹಿಂತಿರುಗಿ(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
regatoi(ಸ್ಥಿರಾಂಕ llvm_regex_t *preg, ಅಕ್ಷರ *localbuf, ಪೂರ್ಣಾಂಕ localbufsize)
{
	ರಚನೆ rerr *r;

	ಗಾಗಿ (r = rerrs; r->ಸಂಕೇತ_3 != 0; r++)
		ಆದರೆ (strcmp(r->name, preg->re_endp) == 0)
			ನಿಲ್ಲಿಸು;
	ಆದರೆ (r->ಸಂಕೇತ_3 == 0)
		ಹಿಂತಿರುಗಿ("0");

	(ಶೂನ್ಯ)snprintf(localbuf, localbufsize, "%d", r->ಸಂಕೇತ_3);
	ಹಿಂತಿರುಗಿ(localbuf);
}
