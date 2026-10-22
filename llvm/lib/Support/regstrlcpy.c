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

#ضمن <sys/types.h>
#ضمن <string.h>

#ضمن "regex_impl.h"
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
llvm_strlcpy(حرف *dst, ثابت حرف *src, size_t siz)
{
	حرف *d = dst;
	ثابت حرف *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	אם (n != 0) {
		כלעוד (--n != 0) {
			אם ((*d++ = *s++) == '\0')
				اكسر;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	אם (n == 0) {
		אם (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		כלעוד (*s++)
			;
	}

	החזר(s - src - 1);	/* count does not include NUL */
}
