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

#اگر_تعریف_نشده LLVM_SUPPORT_REGEX_IMPL_H
#تعریف_کن LLVM_SUPPORT_REGEX_IMPL_H

#ضمن <sys/types.h>
تعريفنوع off_t llvm_regoff_t;
تعريفنوع بنية {
  llvm_regoff_t rm_so;		/* start of match */
  llvm_regoff_t rm_eo;		/* end of match */
} llvm_regmatch_t;

تعريفنوع بنية llvm_regex {
  صحيح re_magic;
  size_t re_nsub;		/* number of parenthesized subexpressions */
  ثابت حرف *re_endp;	/* end pointer for REG_PEND */
  بنية re_guts *re_g;	/* none of your business :-) */
} llvm_regex_t;

/* llvm_regcomp() flags */
#تعریف_کن	REG_BASIC	0000
#تعریف_کن	REG_EXTENDED	0001
#تعریف_کن	REG_ICASE	0002
#تعریف_کن	REG_NOSUB	0004
#تعریف_کن	REG_NEWLINE	0010
#تعریف_کن	REG_NOSPEC	0020
#تعریف_کن	REG_PEND	0040
#تعریف_کن	REG_DUMP	0200

/* llvm_regerror() flags */
#تعریف_کن	REG_NOMATCH	 1
#تعریف_کن	REG_BADPAT	 2
#تعریف_کن	REG_ECOLLATE	 3
#تعریف_کن	REG_ECTYPE	 4
#تعریف_کن	REG_EESCAPE	 5
#تعریف_کن	REG_ESUBREG	 6
#تعریف_کن	REG_EBRACK	 7
#تعریف_کن	REG_EPAREN	 8
#تعریف_کن	REG_EBRACE	 9
#تعریف_کن	REG_BADBR	10
#تعریف_کن	REG_ERANGE	11
#تعریف_کن	REG_ESPACE	12
#تعریف_کن	REG_BADRPT	13
#تعریف_کن	REG_خالی	14
#تعریف_کن	REG_ASSERT	15
#تعریف_کن	REG_INVARG	16
#تعریف_کن	REG_ATOI	255	/* convert name to number (!) */
#تعریف_کن	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#تعریف_کن	REG_NOTBOL	00001
#تعریف_کن	REG_NOTEOL	00002
#تعریف_کن	REG_STARTEND	00004
#تعریف_کن	REG_TRACE	00400	/* tracing of execution */
#تعریف_کن	REG_LARGE	01000	/* force large representation */
#تعریف_کن	REG_BACKR	02000	/* force use of backref code */

#اگر_تعریف_شده __cplusplus
خارجي "C" {
#پایان_اگر

صحيح	llvm_regcomp(llvm_regex_t *, ثابت حرف *, صحيح);
size_t	llvm_regerror(صحيح, ثابت llvm_regex_t *, حرف *, size_t);
صحيح	llvm_regexec(ثابت llvm_regex_t *, ثابت حرف *, size_t,
                     llvm_regmatch_t [], صحيح);
فراغ	llvm_regfree(llvm_regex_t *);
size_t  llvm_strlcpy(حرف *dst, ثابت حرف *src, size_t siz);

#اگر_تعریف_شده __cplusplus
}
#پایان_اگر

#پایان_اگر /* LLVM_SUPPORT_REGEX_IMPL_H */
