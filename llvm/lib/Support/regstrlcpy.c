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

#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "regex_impl.h"
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
llvm_strlcpy(ৱর্ণ *dst, ধ্রুৱ ৱর্ণ *src, size_t siz)
{
	ৱর্ণ *d = dst;
	ধ্রুৱ ৱর্ণ *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	যদি (n != 0) {
		যেতিয়ালৈ (--n != 0) {
			যদি ((*d++ = *s++) == '\0')
				ভঙ্গ;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	যদি (n == 0) {
		যদি (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		যেতিয়ালৈ (*s++)
			;
	}

	ঘূৰাই(s - src - 1);	/* count does not include NUL */
}
