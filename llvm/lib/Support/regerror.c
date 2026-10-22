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

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "regex_impl.h"

#अन्तर्भाव "regutils.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

स्थिर स्थिरांक अक्षर *regatoi(स्थिरांक llvm_regex_t *, अक्षर *, पूर्णांक);

स्थिर संरचना rerr {
	पूर्णांक code;
	स्थिरांक अक्षर *name;
	स्थिरांक अक्षर *explain;
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
माप_प्रकार
llvm_regerror(पूर्णांक errcode, स्थिरांक llvm_regex_t *preg, अक्षर *errbuf, माप_प्रकार errbuf_size)
{
	संरचना rerr *r;
	माप_प्रकार len;
	पूर्णांक target = errcode &~ REG_ITOA;
	स्थिरांक अक्षर *s;
	अक्षर convbuf[50];

	जर (errcode == REG_ATOI)
		s = regatoi(preg, convbuf, आकार convbuf);
	अन्यथा {
		साठी (r = rerrs; r->code != 0; r++)
			जर (r->code == target)
				थांबा;
	
		जर (errcode&REG_ITOA) {
			जर (r->code != 0) {
				प्रतिज्ञा(म_माप(r->name) < आकार(convbuf));
				(शून्य) llvm_strlcpy(convbuf, r->name, आकार convbuf);
			} अन्यथा
				(शून्य)snprintf(convbuf, आकार convbuf,
				    "REG_0x%x", target);
			s = convbuf;
		} अन्यथा
			s = r->explain;
	}

	len = म_माप(s) + 1;
	जर (errbuf_size > 0) {
		llvm_strlcpy(errbuf, s, errbuf_size);
	}

	परत(len);
}

/*
 - regatoi - internal routine to implement REG_ATOI
 */
स्थिर स्थिरांक अक्षर *
regatoi(स्थिरांक llvm_regex_t *preg, अक्षर *localbuf, पूर्णांक localbufsize)
{
	संरचना rerr *r;

	साठी (r = rerrs; r->code != 0; r++)
		जर (म_भेद(r->name, preg->re_endp) == 0)
			थांबा;
	जर (r->code == 0)
		परत("0");

	(शून्य)snprintf(localbuf, localbufsize, "%d", r->code);
	परत(localbuf);
}
