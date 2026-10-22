/*-
 * This code is derived from OpenBSD's libc/regex, original license follows:
 *
 * Copyright (c) 1992 Henry Spencer.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Henry Spencer of the University of Toronto.
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
 *	@(#)regex.h	8.1 (Berkeley) 6/2/93
 */

#अगर_अघोषित LLVM_SUPPORT_REGEX_IMPL_H
#घोषणा LLVM_SUPPORT_REGEX_IMPL_H

#समावेश <sys/types.h>
प्रकार off_t llvm_regoff_t;
प्रकार काष्ठा {
  llvm_regoff_t rm_so;		/* start of match */
  llvm_regoff_t rm_eo;		/* end of match */
} llvm_regmatch_t;

प्रकार काष्ठा llvm_regex {
  पूर्णांक re_magic;
  माप_प्रकार re_nsub;		/* number of parenthesized subexpressions */
  स्थिर अक्षर *re_endp;	/* end pointer for REG_PEND */
  काष्ठा re_guts *re_g;	/* none of your business :-) */
} llvm_regex_t;

/* llvm_regcomp() flags */
#घोषणा	REG_BASIC	0000
#घोषणा	REG_EXTENDED	0001
#घोषणा	REG_ICASE	0002
#घोषणा	REG_NOSUB	0004
#घोषणा	REG_NEWLINE	0010
#घोषणा	REG_NOSPEC	0020
#घोषणा	REG_PEND	0040
#घोषणा	REG_DUMP	0200

/* llvm_regerror() flags */
#घोषणा	REG_NOMATCH	 1
#घोषणा	REG_BADPAT	 2
#घोषणा	REG_ECOLLATE	 3
#घोषणा	REG_ECTYPE	 4
#घोषणा	REG_EESCAPE	 5
#घोषणा	REG_ESUBREG	 6
#घोषणा	REG_EBRACK	 7
#घोषणा	REG_EPAREN	 8
#घोषणा	REG_EBRACE	 9
#घोषणा	REG_BADBR	10
#घोषणा	REG_ERANGE	11
#घोषणा	REG_ESPACE	12
#घोषणा	REG_BADRPT	13
#घोषणा	REG_EMPTY	14
#घोषणा	REG_ASSERT	15
#घोषणा	REG_INVARG	16
#घोषणा	REG_ATOI	255	/* convert name to number (!) */
#घोषणा	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#घोषणा	REG_NOTBOL	00001
#घोषणा	REG_NOTEOL	00002
#घोषणा	REG_STARTEND	00004
#घोषणा	REG_TRACE	00400	/* tracing of execution */
#घोषणा	REG_LARGE	01000	/* force large representation */
#घोषणा	REG_BACKR	02000	/* force use of backref code */

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

पूर्णांक	llvm_regcomp(llvm_regex_t *, स्थिर अक्षर *, पूर्णांक);
माप_प्रकार	llvm_regerror(पूर्णांक, स्थिर llvm_regex_t *, अक्षर *, माप_प्रकार);
पूर्णांक	llvm_regexec(स्थिर llvm_regex_t *, स्थिर अक्षर *, माप_प्रकार,
                     llvm_regmatch_t [], पूर्णांक);
व्योम	llvm_regfree(llvm_regex_t *);
माप_प्रकार  llvm_strlcpy(अक्षर *dst, स्थिर अक्षर *src, माप_प्रकार siz);

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर

#पूर्ण_अगर /* LLVM_SUPPORT_REGEX_IMPL_H */
