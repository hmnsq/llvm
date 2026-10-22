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

#جيڪڏهن_وصف_نه_ٿيل LLVM_SUPPORT_REGEX_IMPL_H
#وصف_ڪريو LLVM_SUPPORT_REGEX_IMPL_H

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
#وصف_ڪريو	REG_BASIC	0000
#وصف_ڪريو	REG_EXTENDED	0001
#وصف_ڪريو	REG_ICASE	0002
#وصف_ڪريو	REG_NOSUB	0004
#وصف_ڪريو	REG_NEWLINE	0010
#وصف_ڪريو	REG_NOSPEC	0020
#وصف_ڪريو	REG_PEND	0040
#وصف_ڪريو	REG_DUMP	0200

/* llvm_regerror() flags */
#وصف_ڪريو	REG_NOMATCH	 1
#وصف_ڪريو	REG_BADPAT	 2
#وصف_ڪريو	REG_ECOLLATE	 3
#وصف_ڪريو	REG_ECTYPE	 4
#وصف_ڪريو	REG_EESCAPE	 5
#وصف_ڪريو	REG_ESUBREG	 6
#وصف_ڪريو	REG_EBRACK	 7
#وصف_ڪريو	REG_EPAREN	 8
#وصف_ڪريو	REG_EBRACE	 9
#وصف_ڪريو	REG_BADBR	10
#وصف_ڪريو	REG_ERANGE	11
#وصف_ڪريو	REG_ESPACE	12
#وصف_ڪريو	REG_BADRPT	13
#وصف_ڪريو	REG_خالي_جايون	14
#وصف_ڪريو	REG_ASSERT	15
#وصف_ڪريو	REG_INVARG	16
#وصف_ڪريو	REG_ATOI	255	/* convert name to number (!) */
#وصف_ڪريو	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#وصف_ڪريو	REG_NOTBOL	00001
#وصف_ڪريو	REG_NOTEOL	00002
#وصف_ڪريو	REG_STARTEND	00004
#وصف_ڪريو	REG_TRACE	00400	/* tracing of execution */
#وصف_ڪريو	REG_LARGE	01000	/* force large representation */
#وصف_ڪريو	REG_BACKR	02000	/* force use of backref code */

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو

صحيح	llvm_regcomp(llvm_regex_t *, ثابت حرف *, صحيح);
size_t	llvm_regerror(صحيح, ثابت llvm_regex_t *, حرف *, size_t);
صحيح	llvm_regexec(ثابت llvm_regex_t *, ثابت حرف *, size_t,
                     llvm_regmatch_t [], صحيح);
فراغ	llvm_regfree(llvm_regex_t *);
size_t  llvm_strlcpy(حرف *dst, ثابت حرف *src, size_t siz);

#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* LLVM_SUPPORT_REGEX_IMPL_H */
