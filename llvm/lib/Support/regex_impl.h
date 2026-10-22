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

#నిర్వచితం_కాకపోతే LLVM_SUPPORT_REGEX_IMPL_H
#నిర్వచించు LLVM_SUPPORT_REGEX_IMPL_H

#అన్తర్భావ <sys/types.h>
రకనిర్వచనం off_t llvm_regoff_t;
రకనిర్వచనం నిర్మాణం {
  llvm_regoff_t rm_so;		/* start of match */
  llvm_regoff_t rm_eo;		/* end of match */
} llvm_regmatch_t;

రకనిర్వచనం నిర్మాణం llvm_regex {
  పూర్ణాంకం re_magic;
  size_t re_nsub;		/* number of parenthesized subexpressions */
  స్థిరరాశి అక్షరం *re_endp;	/* end pointer for REG_PEND */
  నిర్మాణం re_guts *re_g;	/* none of your business :-) */
} llvm_regex_t;

/* llvm_regcomp() flags */
#నిర్వచించు	REG_BASIC	0000
#నిర్వచించు	REG_EXTENDED	0001
#నిర్వచించు	REG_ICASE	0002
#నిర్వచించు	REG_NOSUB	0004
#నిర్వచించు	REG_NEWLINE	0010
#నిర్వచించు	REG_NOSPEC	0020
#నిర్వచించు	REG_PEND	0040
#నిర్వచించు	REG_DUMP	0200

/* llvm_regerror() flags */
#నిర్వచించు	REG_NOMATCH	 1
#నిర్వచించు	REG_BADPAT	 2
#నిర్వచించు	REG_ECOLLATE	 3
#నిర్వచించు	REG_ECTYPE	 4
#నిర్వచించు	REG_EESCAPE	 5
#నిర్వచించు	REG_ESUBREG	 6
#నిర్వచించు	REG_EBRACK	 7
#నిర్వచించు	REG_EPAREN	 8
#నిర్వచించు	REG_EBRACE	 9
#నిర్వచించు	REG_BADBR	10
#నిర్వచించు	REG_ERANGE	11
#నిర్వచించు	REG_ESPACE	12
#నిర్వచించు	REG_BADRPT	13
#నిర్వచించు	REG_ఖాళీలు	14
#నిర్వచించు	REG_ASSERT	15
#నిర్వచించు	REG_INVARG	16
#నిర్వచించు	REG_ATOI	255	/* convert name to number (!) */
#నిర్వచించు	REG_ITOA	0400	/* convert number to name (!) */

/* llvm_regexec() flags */
#నిర్వచించు	REG_NOTBOL	00001
#నిర్వచించు	REG_NOTEOL	00002
#నిర్వచించు	REG_STARTEND	00004
#నిర్వచించు	REG_TRACE	00400	/* tracing of execution */
#నిర్వచించు	REG_LARGE	01000	/* force large representation */
#నిర్వచించు	REG_BACKR	02000	/* force use of backref code */

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

పూర్ణాంకం	llvm_regcomp(llvm_regex_t *, స్థిరరాశి అక్షరం *, పూర్ణాంకం);
size_t	llvm_regerror(పూర్ణాంకం, స్థిరరాశి llvm_regex_t *, అక్షరం *, size_t);
పూర్ణాంకం	llvm_regexec(స్థిరరాశి llvm_regex_t *, స్థిరరాశి అక్షరం *, size_t,
                     llvm_regmatch_t [], పూర్ణాంకం);
శూన్య	llvm_regfree(llvm_regex_t *);
size_t  llvm_strlcpy(అక్షరం *dst, స్థిరరాశి అక్షరం *src, size_t siz);

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* LLVM_SUPPORT_REGEX_IMPL_H */
