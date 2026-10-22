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

#समावेश <sys/types.h>
#समावेश <string.h>

#समावेश "regex_impl.h"
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
माप_प्रकार
llvm_strlcpy(अक्षर *dst, स्थिर अक्षर *src, माप_प्रकार siz)
{
	अक्षर *d = dst;
	स्थिर अक्षर *s = src;
	माप_प्रकार n = siz;

	/* Copy as many bytes as will fit */
	अगर (n != 0) {
		जबतक (--n != 0) {
			अगर ((*d++ = *s++) == '\0')
				अवरोध;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	अगर (n == 0) {
		अगर (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		जबतक (*s++)
			;
	}

	वापस(s - src - 1);	/* count does not include NUL */
}
