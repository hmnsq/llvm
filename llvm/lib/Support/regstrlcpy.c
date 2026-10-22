/*
 * This code is derived from OpenBSD's libc, original license follows:
 *
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "regex_impl.h"
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
llvm_strlcpy(ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src, size_t siz)
{
	ଅକ୍ଷର *d = dst;
	ଅଚଳ ଅକ୍ଷର *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	ଯଦି (n != 0) {
		ଯେତେବେଳେ (--n != 0) {
			ଯଦି ((*d++ = *s++) == '\0')
				ଭାଙ୍ଗ;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	ଯଦି (n == 0) {
		ଯଦି (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		ଯେତେବେଳେ (*s++)
			;
	}

	ଫେରନ୍ତୁ(s - src - 1);	/* count does not include NUL */
}
