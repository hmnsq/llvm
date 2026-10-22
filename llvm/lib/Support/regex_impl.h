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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LLVM_SUPPORT_REGEX_IMPL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LLVM_SUPPORT_REGEX_IMPL_H

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
ਕਿਸਮਪਰਿਭਾਸ਼ਾ off_t llvm_regoff_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  llvm_regoff_t rm_so;		/* start of match */
  llvm_regoff_t rm_eo;		/* end of match */
} llvm_regmatch_t;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ llvm_regex {
  ਪੂਰਨ re_magic;
  size_t re_nsub;		/* number of parenthesized subexpressions */
  ਅਚਲ ਅੱਖਰ *re_endp;	/* end pointer for REG_PEND */
  ਬਣਤਰ re_guts *re_g;	/* none of your business :-) */
} llvm_regex_t;

/* llvm_regcomp() flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_BASIC	0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_EXTENDED	0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ICASE	0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NOSUB	0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NEWLINE	0010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NOSPEC	0020
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_PEND	0040
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_DUMP	0200

/* llvm_regerror() flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NOMATCH	 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_BADPAT	 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ECOLLATE	 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ECTYPE	 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_EESCAPE	 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ESUBREG	 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_EBRACK	 7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_EPAREN	 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_EBRACE	 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_BADBR	10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ERANGE	11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ESPACE	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_BADRPT	13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ਖਾਲੀ_ਥਾਵਾਂ	14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ASSERT	15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_INVARG	16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ATOI	255	/* convert name to number (!) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NOTBOL	00001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_NOTEOL	00002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_STARTEND	00004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_TRACE	00400	/* tracing of execution */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_LARGE	01000	/* force large representation */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	REG_BACKR	02000	/* force use of backref code */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

ਪੂਰਨ	llvm_regcomp(llvm_regex_t *, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ);
size_t	llvm_regerror(ਪੂਰਨ, ਅਚਲ llvm_regex_t *, ਅੱਖਰ *, size_t);
ਪੂਰਨ	llvm_regexec(ਅਚਲ llvm_regex_t *, ਅਚਲ ਅੱਖਰ *, size_t,
                     llvm_regmatch_t [], ਪੂਰਨ);
ਖਾਲੀ	llvm_regfree(llvm_regex_t *);
size_t  llvm_strlcpy(ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src, size_t siz);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* LLVM_SUPPORT_REGEX_IMPL_H */
