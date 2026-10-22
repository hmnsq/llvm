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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LLVM_SUPPORT_REGEX_IMPL_H
#ವ್ಯಾಖ್ಯಾನಿಸು LLVM_SUPPORT_REGEX_IMPL_H

#ಅನ್ತರ್ಭಾವ <sys/types.h>
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ off_t llvm_regoff_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  llvm_regoff_t rm_so;		/* start of match */
  llvm_regoff_t rm_eo;		/* end of match */
} llvm_regmatch_t;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ llvm_regex {
  ಪೂರ್ಣಾಂಕ re_magic;
  size_t re_nsub;		/* number of parenthesized subexpressions */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *re_endp;	/* end pointer for REG_PEND */
  ರಚನೆ re_guts *re_g;	/* none of your business :-) */
} llvm_regex_t;

/* llvm_regcomp() flags */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_BASIC	0000
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_EXTENDED	0001
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ICASE	0002
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NOSUB	0004
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NEWLINE	0010
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NOSPEC	0020
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_PEND	0040
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_DUMP	0200

/* llvm_regerror() flags */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NOMATCH	 1
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_BADPAT	 2
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ECOLLATE	 3
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ECTYPE	 4
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_EESCAPE	 5
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ESUBREG	 6
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_EBRACK	 7
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_EPAREN	 8
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_EBRACE	 9
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_BADBR	10
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ERANGE	11
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ESPACE	12
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_BADRPT	13
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ಖಾಲಿಗಳು	14
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ASSERT	15
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_INVARG	16
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ATOI	255	/* convert name to number (!) */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NOTBOL	00001
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_NOTEOL	00002
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_STARTEND	00004
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_TRACE	00400	/* tracing of execution */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_LARGE	01000	/* force large representation */
#ವ್ಯಾಖ್ಯಾನಿಸು	REG_BACKR	02000	/* force use of backref code */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ	llvm_regcomp(llvm_regex_t *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);
size_t	llvm_regerror(ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ llvm_regex_t *, ಅಕ್ಷರ *, size_t);
ಪೂರ್ಣಾಂಕ	llvm_regexec(ಸ್ಥಿರಾಂಕ llvm_regex_t *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, size_t,
                     llvm_regmatch_t [], ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ	llvm_regfree(llvm_regex_t *);
size_t  llvm_strlcpy(ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t siz);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* LLVM_SUPPORT_REGEX_IMPL_H */
