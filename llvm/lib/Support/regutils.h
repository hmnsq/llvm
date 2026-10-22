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
 *	@(#)utils.h	8.3 (Berkeley) 3/20/94
 */

#جيڪڏهن_وصف_نه_ٿيل LLVM_SUPPORT_REGUTILS_H
#وصف_ڪريو LLVM_SUPPORT_REGUTILS_H

/* utility definitions */
#وصف_ڪريو	NC		(CHAR_MAX - CHAR_MIN + 1)
تعريفنوع بنانشان حرف uch;

/* switch off assertions (if not already off) if no REDEBUG */
#جيڪڏهن_وصف_نه_ٿيل REDEBUG
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
#وصف_ڪريو	NDEBUG	/* no assertions please */
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#ضمن <assert.h>

/* for old systems with bcopy() but no memmove() */
#جيڪڏهن_وصف_ٿيل USEBCOPY
#وصف_ڪريو	memmove(d, s, c)	bcopy(s, d, c)
#جيڪڏهن_پورو

#جيڪڏهن_پورو
