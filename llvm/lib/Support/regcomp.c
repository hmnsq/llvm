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
 *	@(#)regcomp.c	8.5 (Berkeley) 3/20/94
 */

#ಅನ್ತರ್ಭಾವ "regex_impl.h"
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "regex2.h"
#ಅನ್ತರ್ಭಾವ "regutils.h"

#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"

/* character-class table */
ಸ್ಥಿರ ರಚನೆ cclass {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *chars;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *multis;
} cclasses[] = {
    {"alnum", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\
0123456789",
     ""},
    {"alpha", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", ""},
    {"blank", " \t", ""},
    {"cntrl", "\007\b\t\n\v\f\r\1\2\3\4\5\6\16\17\20\21\22\23\24\
\25\26\27\30\31\32\33\34\35\36\37\177",
     ""},
    {"digit", "0123456789", ""},
    {"graph", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\
0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
     ""},
    {"lower", "abcdefghijklmnopqrstuvwxyz", ""},
    {"print", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\
0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ",
     ""},
    {"punct", "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", ""},
    {"space", "\t\n\v\f\r ", ""},
    {"upper", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", ""},
    {"xdigit", "0123456789ABCDEFabcdef", ""},
    {ಖಾಲಿ, 0, ""}};

/* character-name table */
ಸ್ಥಿರ ರಚನೆ cname {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  ಅಕ್ಷರ ಸಂಕೇತ_3;
} cnames[] = {{"NUL", '\0'},
              {"SOH", '\001'},
              {"STX", '\002'},
              {"ETX", '\003'},
              {"EOT", '\004'},
              {"ENQ", '\005'},
              {"ACK", '\006'},
              {"BEL", '\007'},
              {"alert", '\007'},
              {"BS", '\010'},
              {"backspace", '\b'},
              {"HT", '\011'},
              {"tab", '\t'},
              {"LF", '\012'},
              {"newline", '\n'},
              {"VT", '\013'},
              {"vertical-tab", '\v'},
              {"FF", '\014'},
              {"form-feed", '\f'},
              {"CR", '\015'},
              {"carriage-return", '\r'},
              {"SO", '\016'},
              {"SI", '\017'},
              {"DLE", '\020'},
              {"DC1", '\021'},
              {"DC2", '\022'},
              {"DC3", '\023'},
              {"DC4", '\024'},
              {"NAK", '\025'},
              {"SYN", '\026'},
              {"ETB", '\027'},
              {"CAN", '\030'},
              {"EM", '\031'},
              {"SUB", '\032'},
              {"ESC", '\033'},
              {"IS4", '\034'},
              {"FS", '\034'},
              {"IS3", '\035'},
              {"GS", '\035'},
              {"IS2", '\036'},
              {"RS", '\036'},
              {"IS1", '\037'},
              {"US", '\037'},
              {"space", ' '},
              {"exclamation-mark", '!'},
              {"quotation-mark", '"'},
              {"number-sign", '#'},
              {"dollar-sign", '$'},
              {"percent-sign", '%'},
              {"ampersand", '&'},
              {"apostrophe", '\''},
              {"left-parenthesis", '('},
              {"right-parenthesis", ')'},
              {"asterisk", '*'},
              {"plus-sign", '+'},
              {"comma", ','},
              {"hyphen", '-'},
              {"hyphen-minus", '-'},
              {"period", '.'},
              {"full-stop", '.'},
              {"slash", '/'},
              {"solidus", '/'},
              {"zero", '0'},
              {"one", '1'},
              {"two", '2'},
              {"three", '3'},
              {"four", '4'},
              {"five", '5'},
              {"six", '6'},
              {"seven", '7'},
              {"eight", '8'},
              {"nine", '9'},
              {"colon", ':'},
              {"semicolon", ';'},
              {"less-than-sign", '<'},
              {"equals-sign", '='},
              {"greater-than-sign", '>'},
              {"question-mark", '?'},
              {"commercial-at", '@'},
              {"left-square-bracket", '['},
              {"backslash", '\\'},
              {"reverse-solidus", '\\'},
              {"right-square-bracket", ']'},
              {"circumflex", '^'},
              {"circumflex-accent", '^'},
              {"underscore", '_'},
              {"low-line", '_'},
              {"grave-accent", '`'},
              {"left-brace", '{'},
              {"left-curly-bracket", '{'},
              {"vertical-line", '|'},
              {"right-brace", '}'},
              {"right-curly-bracket", '}'},
              {"tilde", '~'},
              {"DEL", '\177'},
              {ಖಾಲಿ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ರಚನೆ parse {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಂದಿನ; /* next character in RE */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ;  /* end of string (-> NUL normally) */
  ಪೂರ್ಣಾಂಕ error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  ಪೂರ್ಣಾಂಕ ncsalloc;     /* number of csets allocated */
  ರಚನೆ re_guts *g;
#ವ್ಯಾಖ್ಯಾನಿಸು NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ಸ್ಥಿರ ಶೂನ್ಯ p_ere(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ p_ere_exp(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_str(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_bre(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ p_simp_re(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ p_ಎಣಿಕೆ(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_bracket(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_ಪದ(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_cclass(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_eclass(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಅಕ್ಷರ p_b_ಚಿಹ್ನೆ(ರಚನೆ parse *);
ಸ್ಥಿರ ಅಕ್ಷರ p_b_coll_elem(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಅಕ್ಷರ othercase(ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ bothcases(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ uncased(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ ordinary(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ nonnewline(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ repeat(ರಚನೆ parse *, sopno, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ seterr(ರಚನೆ parse *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ cset *allocset(ರಚನೆ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ freeset(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ freezeset(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ firstch(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nch(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ mcadd(ರಚನೆ parse *, cset *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
ಸ್ಥಿರ ಶೂನ್ಯ mcinvert(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ mccase(ರಚನೆ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isinsets(ರಚನೆ re_guts *, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ samesets(ರಚನೆ re_guts *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಶೂನ್ಯ categorize(ರಚನೆ parse *, ರಚನೆ re_guts *);
ಸ್ಥಿರ sopno dupl(ರಚನೆ parse *, sopno, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ doemit(ರಚನೆ parse *, sop, size_t);
ಸ್ಥಿರ ಶೂನ್ಯ doinsert(ರಚನೆ parse *, sop, size_t, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ dofwd(ರಚನೆ parse *, sopno, sop);
ಸ್ಥಿರ ಶೂನ್ಯ enlarge(ರಚನೆ parse *, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ stripsnug(ರಚನೆ parse *, ರಚನೆ re_guts *);
ಸ್ಥಿರ ಶೂನ್ಯ findmust(ರಚನೆ parse *, ರಚನೆ re_guts *);
ಸ್ಥಿರ sopno pluscount(ರಚನೆ parse *, ರಚನೆ re_guts *);

ಸ್ಥಿರ ಅಕ್ಷರ nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#ವ್ಯಾಖ್ಯಾನಿಸು PEEK() (*p->ಮುಂದಿನ)
#ವ್ಯಾಖ್ಯಾನಿಸು PEEK2() (*(p->ಮುಂದಿನ + 1))
#ವ್ಯಾಖ್ಯಾನಿಸು ಇನ್ನಷ್ಟು() (p->ಕೊನೆ - p->ಮುಂದಿನ > 0)
#ವ್ಯಾಖ್ಯಾನಿಸು MORE2() (p->ಕೊನೆ - p->ಮುಂದಿನ > 1)
#ವ್ಯಾಖ್ಯಾನಿಸು SEE(c) (ಇನ್ನಷ್ಟು() && PEEK() == (c))
#ವ್ಯಾಖ್ಯಾನಿಸು SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#ವ್ಯಾಖ್ಯಾನಿಸು EAT(c) ((SEE(c)) ? (ಮುಂದಿನ_3(), 1) : 0)
#ವ್ಯಾಖ್ಯಾನಿಸು EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_3() (p->ಮುಂದಿನ++)
#ವ್ಯಾಖ್ಯಾನಿಸು NEXT2() (p->ಮುಂದಿನ += 2)
#ವ್ಯಾಖ್ಯಾನಿಸು NEXTn(n) (p->ಮುಂದಿನ += (n))
#ವ್ಯಾಖ್ಯಾನಿಸು GETNEXT() (*p->ಮುಂದಿನ++)
#ವ್ಯಾಖ್ಯಾನಿಸು SETERROR(e) seterr(p, (e))
#ವ್ಯಾಖ್ಯಾನಿಸು REQUIRE(co, e) (ಶೂನ್ಯ)((co) || SETERROR(e))
#ವ್ಯಾಖ್ಯಾನಿಸು MUSTSEE(c, e) (REQUIRE(ಇನ್ನಷ್ಟು() && PEEK() == (c), e))
#ವ್ಯಾಖ್ಯಾನಿಸು MUSTEAT(c, e) (REQUIRE(ಇನ್ನಷ್ಟು() && GETNEXT() == (c), e))
#ವ್ಯಾಖ್ಯಾನಿಸು MUSTNOTSEE(c, e) (REQUIRE(!ಇನ್ನಷ್ಟು() || PEEK() != (c), e))
#ವ್ಯಾಖ್ಯಾನಿಸು EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#ವ್ಯಾಖ್ಯಾನಿಸು ಸೇರಿಸು_3(op, pos) doinsert(p, (sop)(op), ಇಲ್ಲಿ() - (pos) + 1, pos)
#ವ್ಯಾಖ್ಯಾನಿಸು AHEAD(pos) dofwd(p, pos, ಇಲ್ಲಿ() - (pos))
#ವ್ಯಾಖ್ಯಾನಿಸು ASTERN(sop, pos) EMIT(sop, ಇಲ್ಲಿ() - pos)
#ವ್ಯಾಖ್ಯಾನಿಸು ಇಲ್ಲಿ() (p->slen)
#ವ್ಯಾಖ್ಯಾನಿಸು THERE() (p->slen - 1)
#ವ್ಯಾಖ್ಯಾನಿಸು THERETHERE() (p->slen - 2)
#ವ್ಯಾಖ್ಯಾನಿಸು DROP(n) (p->slen -= (n))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _POSIX2_RE_DUP_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು DUPMAX _POSIX2_RE_DUP_MAX
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DUPMAX 255
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು REGINFINITY (DUPMAX + 1)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ never = 0; /* for use in asserts; shuts lint up */
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು never 0 /* some <assert.h>s have bugs too */
#ಆದರೆ_ಅಂತ್ಯ

/*
 - llvm_regcomp - interface for parser and compilation
 */
ಪೂರ್ಣಾಂಕ /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ_2, ಪೂರ್ಣಾಂಕ cflags) {
  ರಚನೆ parse pa;
  ರಚನೆ re_guts *g;
  ರಚನೆ parse *p = &pa;
  ಪೂರ್ಣಾಂಕ i;
  size_t len;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ REDEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು GOODFLAGS(f) (f)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು GOODFLAGS(f) ((f) & ~REG_DUMP)
#ಆದರೆ_ಅಂತ್ಯ

  cflags = GOODFLAGS(cflags);
  ಆದರೆ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ಹಿಂತಿರುಗಿ (REG_INVARG);

  ಆದರೆ (cflags & REG_PEND) {
    ಆದರೆ (preg->re_endp < ನಮೂನೆ_2)
      ಹಿಂತಿರುಗಿ (REG_INVARG);
    len = preg->re_endp - ನಮೂನೆ_2;
  } ಇಲ್ಲದಿದ್ದರೆ {
    len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ನಮೂನೆ_2);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ರಚನೆ re_guts *)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ re_guts) +
                               (NC - 1) * ಗಾತ್ರ(cat_t));
  ಆದರೆ (g == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(p->ssize, ಗಾತ್ರ(sop));
  p->slen = 0;
  ಆದರೆ (p->strip == ಖಾಲಿ) {
    ಬಿಡುಗಡೆ((ಅಕ್ಷರ *)g);
    ಹಿಂತಿರುಗಿ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->ಮುಂದಿನ = ನಮೂನೆ_2;
  p->ಕೊನೆ = p->ಮುಂದಿನ + len;
  p->error = 0;
  p->ncsalloc = 0;
  ಗಾಗಿ (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ಖಾಲಿ;
  g->setbits = ಖಾಲಿ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ಖಾಲಿ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (ಶೂನ್ಯ)memset((ಅಕ್ಷರ *)g->catspace, 0, NC * ಗಾತ್ರ(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ಆದರೆ (cflags & REG_EXTENDED)
    p_ere(p, ಹೊರಹೋಗು_3);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cflags & REG_NOSPEC)
    p_str(p);
  ಇಲ್ಲದಿದ್ದರೆ
    p_bre(p, ಹೊರಹೋಗು_3, ಹೊರಹೋಗು_3);
  EMIT(OEND, 0);
  g->laststate = THERE();

  /* tidy up loose ends and fill things in */
  categorize(p, g);
  stripsnug(p, g);
  findmust(p, g);
  g->nplus = pluscount(p, g);
  g->magic = MAGIC2;
  preg->re_nsub = g->nsub;
  preg->re_g = g;
  preg->re_magic = MAGIC1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  ಆದರೆ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#ಆದರೆ_ಅಂತ್ಯ

  /* win or lose, we're done */
  ಆದರೆ (p->error != 0) /* lose */
    llvm_regfree(preg);
  ಹಿಂತಿರುಗಿ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_ere(ರಚನೆ parse *p,
                  ಪೂರ್ಣಾಂಕ stop) /* character this ERE should end at */
{
  ಅಕ್ಷರ c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  ಪೂರ್ಣಾಂಕ ಮೊದಲ_2 = 1; /* is this the first alternative? */

  ಗಾಗಿ (;;) {
    /* do a bunch of concatenated expressions */
    conc = ಇಲ್ಲಿ();
    ಆಗ (ಇನ್ನಷ್ಟು() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(ಇಲ್ಲಿ() != conc, REG_ಖಾಲಿಗಳು); /* require nonempty */

    ಆದರೆ (!EAT('|'))
      ನಿಲ್ಲಿಸು; /* NOTE BREAK OUT */

    ಆದರೆ (ಮೊದಲ_2) {
      ಸೇರಿಸು_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      ಮೊದಲ_2 = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = ಇಲ್ಲಿ();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  ಆದರೆ (!ಮೊದಲ_2) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ಪ್ರತಿಜ್ಞಾ(!ಇನ್ನಷ್ಟು() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_ere_exp(ರಚನೆ parse *p) {
  ಅಕ್ಷರ c;
  sopno pos;
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
  ಪೂರ್ಣಾಂಕ count2;
  ಪೂರ್ಣಾಂಕ backrefnum;
  sopno subno;
  ಪೂರ್ಣಾಂಕ wascaret = 0;

  ಪ್ರತಿಜ್ಞಾ(ಇನ್ನಷ್ಟು()); /* caller should have ensured this */
  c = GETNEXT();

  pos = ಇಲ್ಲಿ();
  ಆಯ್ಕೆ (c) {
  ಸ್ಥಿತಿ '(':
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ಆದರೆ (subno < NPAREN)
      p->pbegin[subno] = ಇಲ್ಲಿ();
    EMIT(OLPAREN, subno);
    ಆದರೆ (!SEE(')'))
      p_ere(p, ')');
    ಆದರೆ (subno < NPAREN) {
      p->pend[subno] = ಇಲ್ಲಿ();
      ಪ್ರತಿಜ್ಞಾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ POSIX_MISTAKE
  ಸ್ಥಿತಿ ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
  ಸ್ಥಿತಿ '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '|':
    SETERROR(REG_ಖಾಲಿಗಳು);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '*':
  ಸ್ಥಿತಿ '+':
  ಸ್ಥಿತಿ '?':
    SETERROR(REG_BADRPT);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '.':
    ಆದರೆ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ಇಲ್ಲದಿದ್ದರೆ
      EMIT(OANY, 0);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '[':
    p_bracket(p);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '\\':
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EESCAPE);
    c = GETNEXT();
    ಆದರೆ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ಆಗ (ಇನ್ನಷ್ಟು() && ಅಂಕಿಯೇ(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == 'n') {
      ordinary(p, '\n');
      ನಿಲ್ಲಿಸು;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == 't') {
      ordinary(p, '\t');
      ನಿಲ್ಲಿಸು;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      ಅಕ್ಷರ hexstr[3] = {0};
      ಅಕ್ಷರ *hexp;
      ಪೂರ್ಣಾಂಕ val;
      ಆದರೆ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ಆದರೆ (*hexp == '\0') {
          NEXT2();
          uncased(p, (ಅಕ್ಷರ)val);
          ನಿಲ್ಲಿಸು;
        }
      }
      ordinary(p, 'x');
      ನಿಲ್ಲಿಸು;
    } ಇಲ್ಲದಿದ್ದರೆ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      ನಿಲ್ಲಿಸು;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    ಪ್ರತಿಜ್ಞಾ(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    ಪ್ರತಿಜ್ಞಾ(p->pbegin[backrefnum] != 0);
    ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (ಶೂನ್ಯ)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_ಹಿಂಭಾಗ, backrefnum);
    p->g->backrefs = 1;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '{': /* okay as ordinary except if digit follows */
    REQUIRE(!ಇನ್ನಷ್ಟು() || !ಅಂಕಿಯೇ((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ordinary(p, c);
    ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (!ಇನ್ನಷ್ಟು())
    ಹಿಂತಿರುಗಿ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ಆದರೆ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ಅಂಕಿಯೇ((uch)PEEK2()))))
    ಹಿಂತಿರುಗಿ; /* no repetition, we're done */
  ಮುಂದಿನ_3();

  REQUIRE(!wascaret, REG_BADRPT);
  ಆಯ್ಕೆ (c) {
  ಸ್ಥಿತಿ '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ಸೇರಿಸು_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '+':
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '{':
    ಎಣಿಕೆ_2 = p_ಎಣಿಕೆ(p);
    ಆದರೆ (EAT(',')) {
      ಆದರೆ (ಅಂಕಿಯೇ((uch)PEEK())) {
        count2 = p_ಎಣಿಕೆ(p);
        REQUIRE(ಎಣಿಕೆ_2 <= count2, REG_BADBR);
      } ಇಲ್ಲದಿದ್ದರೆ /* single number with comma */
        count2 = REGINFINITY;
    } ಇಲ್ಲದಿದ್ದರೆ /* just a single number */
      count2 = ಎಣಿಕೆ_2;
    repeat(p, pos, ಎಣಿಕೆ_2, count2);
    ಆದರೆ (!EAT('}')) { /* error heuristics */
      ಆಗ (ಇನ್ನಷ್ಟು() && PEEK() != '}')
        ಮುಂದಿನ_3();
      REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (!ಇನ್ನಷ್ಟು())
    ಹಿಂತಿರುಗಿ;
  c = PEEK();
  ಆದರೆ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ಅಂಕಿಯೇ((uch)PEEK2()))))
    ಹಿಂತಿರುಗಿ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_str(ರಚನೆ parse *p) {
  REQUIRE(ಇನ್ನಷ್ಟು(), REG_ಖಾಲಿಗಳು);
  ಆಗ (ಇನ್ನಷ್ಟು())
    ordinary(p, GETNEXT());
}

/*
 - p_bre - BRE parser top level, anchoring and concatenation
 * Giving end1 as OUT essentially eliminates the end1/end2 check.
 *
 * This implementation is a bit of a kludge, in that a trailing $ is first
 * taken as an ordinary character and then revised to be an anchor.  The
 * only undesirable side effect is that '$' gets included as a character
 * category in such cases.  This is fairly harmless; not worth fixing.
 * The amount of lookahead needed to avoid this kludge is excessive.
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_bre(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ end1, /* first terminating character */
                  ಪೂರ್ಣಾಂಕ end2)                  /* second terminating character */
{
  sopno ಆರಂಭ_2 = ಇಲ್ಲಿ();
  ಪೂರ್ಣಾಂಕ ಮೊದಲ_2 = 1; /* first subexpression? */
  ಪೂರ್ಣಾಂಕ wasdollar = 0;

  ಆದರೆ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ಆಗ (ಇನ್ನಷ್ಟು() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, ಮೊದಲ_2);
    ಮೊದಲ_2 = 0;
  }
  ಆದರೆ (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(ಇಲ್ಲಿ() != ಆರಂಭ_2, REG_ಖಾಲಿಗಳು); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* was the simple RE an unbackslashed $? */
p_simp_re(ರಚನೆ parse *p,
          ಪೂರ್ಣಾಂಕ starordinary) /* is a leading * an ordinary character? */
{
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
  ಪೂರ್ಣಾಂಕ count2;
  sopno pos;
  ಪೂರ್ಣಾಂಕ i;
  sopno subno;
#ವ್ಯಾಖ್ಯಾನಿಸು BACKSL (1 << CHAR_BIT)

  pos = ಇಲ್ಲಿ(); /* repetition op, if any, covers from here */

  ಪ್ರತಿಜ್ಞಾ(ಇನ್ನಷ್ಟು()); /* caller should have ensured this */
  c = GETNEXT();
  ಆದರೆ (c == '\\') {
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  ಆಯ್ಕೆ (c) {
  ಸ್ಥಿತಿ '.':
    ಆದರೆ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ಇಲ್ಲದಿದ್ದರೆ
      EMIT(OANY, 0);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '[':
    p_bracket(p);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ BACKSL | '{':
    SETERROR(REG_BADRPT);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ಆದರೆ (subno < NPAREN)
      p->pbegin[subno] = ಇಲ್ಲಿ();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ಆದರೆ (ಇನ್ನಷ್ಟು() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ಆದರೆ (subno < NPAREN) {
      p->pend[subno] = ಇಲ್ಲಿ();
      ಪ್ರತಿಜ್ಞಾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ BACKSL | ')': /* should not get here -- must be user */
  ಸ್ಥಿತಿ BACKSL | '}':
    SETERROR(REG_EPAREN);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ BACKSL | '1':
  ಸ್ಥಿತಿ BACKSL | '2':
  ಸ್ಥಿತಿ BACKSL | '3':
  ಸ್ಥಿತಿ BACKSL | '4':
  ಸ್ಥಿತಿ BACKSL | '5':
  ಸ್ಥಿತಿ BACKSL | '6':
  ಸ್ಥಿತಿ BACKSL | '7':
  ಸ್ಥಿತಿ BACKSL | '8':
  ಸ್ಥಿತಿ BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ಪ್ರತಿಜ್ಞಾ(i < NPAREN);
    ಆದರೆ (p->pend[i] != 0) {
      ಪ್ರತಿಜ್ಞಾ(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ಪ್ರತಿಜ್ಞಾ(p->pbegin[i] != 0);
      ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pend[i]]) == ORPAREN);
      (ಶೂನ್ಯ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_ಹಿಂಭಾಗ, i);
    } ಇಲ್ಲದಿದ್ದರೆ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ordinary(p, (ಅಕ್ಷರ)c);
    ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ಸೇರಿಸು_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (EATTWO('\\', '{')) {
    ಎಣಿಕೆ_2 = p_ಎಣಿಕೆ(p);
    ಆದರೆ (EAT(',')) {
      ಆದರೆ (ಇನ್ನಷ್ಟು() && ಅಂಕಿಯೇ((uch)PEEK())) {
        count2 = p_ಎಣಿಕೆ(p);
        REQUIRE(ಎಣಿಕೆ_2 <= count2, REG_BADBR);
      } ಇಲ್ಲದಿದ್ದರೆ /* single number with comma */
        count2 = REGINFINITY;
    } ಇಲ್ಲದಿದ್ದರೆ /* just a single number */
      count2 = ಎಣಿಕೆ_2;
    repeat(p, pos, ಎಣಿಕೆ_2, count2);
    ಆದರೆ (!EATTWO('\\', '}')) { /* error heuristics */
      ಆಗ (ಇನ್ನಷ್ಟು() && !SEETWO('\\', '}'))
        ಮುಂದಿನ_3();
      REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c == '$') /* $ (but not \$) ends it */
    ಹಿಂತಿರುಗಿ (1);

  ಹಿಂತಿರುಗಿ (0);
}

/*
 - p_count - parse a repetition count
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* the value */
p_ಎಣಿಕೆ(ರಚನೆ parse *p) {
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಪೂರ್ಣಾಂಕ ndigits = 0;

  ಆಗ (ಇನ್ನಷ್ಟು() && ಅಂಕಿಯೇ((uch)PEEK()) && ಎಣಿಕೆ_2 <= DUPMAX) {
    ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2 * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ಎಣಿಕೆ_2 <= DUPMAX, REG_BADBR);
  ಹಿಂತಿರುಗಿ (ಎಣಿಕೆ_2);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_bracket(ರಚನೆ parse *p) {
  cset *cs;
  ಪೂರ್ಣಾಂಕ invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ಆದರೆ (p->ಕೊನೆ - p->ಮುಂದಿನ > 5) {
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p->ಮುಂದಿನ, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p->ಮುಂದಿನ, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ಹಿಂತಿರುಗಿ;
    }
  }

  ಆದರೆ ((cs = allocset(p)) == ಖಾಲಿ) {
    /* allocset did set error status in p */
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (EAT('^'))
    invert++; /* make note to invert set at end */
  ಆದರೆ (EAT(']'))
    CHadd(cs, ']');
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (EAT('-'))
    CHadd(cs, '-');
  ಆಗ (ಇನ್ನಷ್ಟು() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_ಪದ(p, cs);
  ಆದರೆ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ಆದರೆ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (p->g->cflags & REG_ICASE) {
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ ci;

    ಗಾಗಿ (i = p->g->csetsize - 1; i >= 0; i--)
      ಆದರೆ (CHIN(cs, i) && ಅಕ್ಷರವೇ(i)) {
        ci = othercase(i);
        ಆದರೆ (ci != i)
          CHadd(cs, ci);
      }
    ಆದರೆ (cs->multis != ಖಾಲಿ)
      mccase(p, cs);
  }
  ಆದರೆ (invert) {
    ಪೂರ್ಣಾಂಕ i;

    ಗಾಗಿ (i = p->g->csetsize - 1; i >= 0; i--)
      ಆದರೆ (CHIN(cs, i))
        CHsub(cs, i);
      ಇಲ್ಲದಿದ್ದರೆ
        CHadd(cs, i);
    ಆದರೆ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ಆದರೆ (cs->multis != ಖಾಲಿ)
      mcinvert(p, cs);
  }

  ಪ್ರತಿಜ್ಞಾ(cs->multis == ಖಾಲಿ); /* xxx */

  ಆದರೆ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ಇಲ್ಲದಿದ್ದರೆ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_ಪದ(ರಚನೆ parse *p, cset *cs) {
  ಅಕ್ಷರ c;
  ಅಕ್ಷರ ಆರಂಭ_2, ಮುಕ್ತಾಯ;
  ಪೂರ್ಣಾಂಕ i;

  /* classify what we've got */
  ಆಯ್ಕೆ ((ಇನ್ನಷ್ಟು()) ? PEEK() : '\0') {
  ಸ್ಥಿತಿ '[':
    c = (MORE2()) ? PEEK2() : '\0';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '-':
    SETERROR(REG_ERANGE);
    ಹಿಂತಿರುಗಿ; /* NOTE RETURN */
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    c = '\0';
    ನಿಲ್ಲಿಸು;
  }

  ಆಯ್ಕೆ (c) {
  ಸ್ಥಿತಿ ':': /* character class */
    NEXT2();
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ '=': /* equivalence class */
    NEXT2();
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    ಆರಂಭ_2 = p_b_ಚಿಹ್ನೆ(p);
    ಆದರೆ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      ಮುಂದಿನ_3();
      ಆದರೆ (EAT('-'))
        ಮುಕ್ತಾಯ = '-';
      ಇಲ್ಲದಿದ್ದರೆ
        ಮುಕ್ತಾಯ = p_b_ಚಿಹ್ನೆ(p);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಮುಕ್ತಾಯ = ಆರಂಭ_2;
    }
    /* xxx what about signed chars here... */
    REQUIRE(ಆರಂಭ_2 <= ಮುಕ್ತಾಯ, REG_ERANGE);
    ಗಾಗಿ (i = ಆರಂಭ_2; i <= ಮುಕ್ತಾಯ; i++)
      CHadd(cs, i);
    ನಿಲ್ಲಿಸು;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_cclass(ರಚನೆ parse *p, cset *cs) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sp = p->ಮುಂದಿನ;
  ರಚನೆ cclass *cp;
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *u;
  ಅಕ್ಷರ c;

  ಆಗ (ಇನ್ನಷ್ಟು() && ಅಕ್ಷರವೇ((uch)PEEK()))
    ಮುಂದಿನ_3();
  len = p->ಮುಂದಿನ - sp;
  ಗಾಗಿ (cp = cclasses; cp->name != ಖಾಲಿ; cp++)
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      ನಿಲ್ಲಿಸು;
  ಆದರೆ (cp->name == ಖಾಲಿ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ಹಿಂತಿರುಗಿ;
  }

  u = cp->chars;
  ಆಗ ((c = *u++) != '\0')
    CHadd(cs, c);
  ಗಾಗಿ (u = cp->multis; *u != '\0'; u += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_eclass(ರಚನೆ parse *p, cset *cs) {
  ಅಕ್ಷರ c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ಸ್ಥಿರ ಅಕ್ಷರ /* value of symbol */
p_b_ಚಿಹ್ನೆ(ರಚನೆ parse *p) {
  ಅಕ್ಷರ ಮೌಲ್ಯ_2;

  REQUIRE(ಇನ್ನಷ್ಟು(), REG_EBRACK);
  ಆದರೆ (!EATTWO('[', '.'))
    ಹಿಂತಿರುಗಿ (GETNEXT());

  /* collating symbol */
  ಮೌಲ್ಯ_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ಹಿಂತಿರುಗಿ (ಮೌಲ್ಯ_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ಸ್ಥಿರ ಅಕ್ಷರ                              /* value of collating element */
p_b_coll_elem(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ endc) /* name ended by endc,']' */
{
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sp = p->ಮುಂದಿನ;
  ರಚನೆ cname *cp;
  size_t len;

  ಆಗ (ಇನ್ನಷ್ಟು() && !SEETWO(endc, ']'))
    ಮುಂದಿನ_3();
  ಆದರೆ (!ಇನ್ನಷ್ಟು()) {
    SETERROR(REG_EBRACK);
    ಹಿಂತಿರುಗಿ (0);
  }
  len = p->ಮುಂದಿನ - sp;
  ಗಾಗಿ (cp = cnames; cp->name != ಖಾಲಿ; cp++)
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(cp->name, sp, len) == 0 && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cp->name) == len)
      ಹಿಂತಿರುಗಿ (cp->ಸಂಕೇತ_3); /* known name */
  ಆದರೆ (len == 1)
    ಹಿಂತಿರುಗಿ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ಹಿಂತಿರುಗಿ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ಸ್ಥಿರ ಅಕ್ಷರ /* if no counterpart, return ch */
othercase(ಪೂರ್ಣಾಂಕ ch) {
  ch = (uch)ch;
  ಪ್ರತಿಜ್ಞಾ(ಅಕ್ಷರವೇ(ch));
  ಆದರೆ (isupper(ch))
    ಹಿಂತಿರುಗಿ ((uch)tolower(ch));
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (islower(ch))
    ಹಿಂತಿರುಗಿ ((uch)ದೊಡ್ಡ_ಅಕ್ಷರ(ch));
  ಇಲ್ಲದಿದ್ದರೆ /* peculiar, but could happen */
    ಹಿಂತಿರುಗಿ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ಸ್ಥಿರ ಶೂನ್ಯ bothcases(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ ch) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oldnext = p->ಮುಂದಿನ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oldend = p->ಕೊನೆ;
  ಅಕ್ಷರ bracket[3];

  ch = (uch)ch;
  ಪ್ರತಿಜ್ಞಾ(othercase(ch) != ch); /* p_bracket() would recurse */
  p->ಮುಂದಿನ = bracket;
  p->ಕೊನೆ = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ಪ್ರತಿಜ್ಞಾ(p->ಮುಂದಿನ == bracket + 2);
  p->ಮುಂದಿನ = oldnext;
  p->ಕೊನೆ = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ಸ್ಥಿರ ಶೂನ್ಯ uncased(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ಆದರೆ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ಸ್ಥಿರ ಶೂನ್ಯ ordinary(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ ch) {
  ಆದರೆ ((p->g->cflags & REG_ICASE) && ಅಕ್ಷರವೇ((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ಇಲ್ಲದಿದ್ದರೆ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ಸ್ಥಿರ ಶೂನ್ಯ nonnewline(ರಚನೆ parse *p) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oldnext = p->ಮುಂದಿನ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oldend = p->ಕೊನೆ;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ bracket[4] = {'^', '\n', ']', '\0'};

  p->ಮುಂದಿನ = bracket;
  p->ಕೊನೆ = bracket + 3;
  p_bracket(p);
  ಪ್ರತಿಜ್ಞಾ(p->ಮುಂದಿನ == bracket + 3);
  p->ಮುಂದಿನ = oldnext;
  p->ಕೊನೆ = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ಸ್ಥಿರ ಶೂನ್ಯ repeat(ರಚನೆ parse *p,
                   sopno ಆರಂಭ_2, /* operand from here to end of strip */
                   ಪೂರ್ಣಾಂಕ from,    /* repeated from this number */
                   ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_2)      /* to this number of times (maybe INFINITY) */
{
  sopno ಮುಕ್ತಾಯ = ಇಲ್ಲಿ();
#ವ್ಯಾಖ್ಯಾನಿಸು N 2
#ವ್ಯಾಖ್ಯಾನಿಸು INF 3
#ವ್ಯಾಖ್ಯಾನಿಸು REP(f, t) ((f) * 8 + (t))
#ವ್ಯಾಖ್ಯಾನಿಸು MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ಆದರೆ (p->error != 0) /* head off possible runaway recursion */
    ಹಿಂತಿರುಗಿ;

  ಪ್ರತಿಜ್ಞಾ(from <= ಗಮ್ಯ_2);

  ಆಯ್ಕೆ (REP(MAP(from), MAP(ಗಮ್ಯ_2))) {
  ಸ್ಥಿತಿ REP(0, 0):         /* must be user doing this */
    DROP(ಮುಕ್ತಾಯ - ಆರಂಭ_2); /* drop the operand */
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(0, 1):   /* as x{1,1}? */
  ಸ್ಥಿತಿ REP(0, N):   /* as x{1,n}? */
  ಸ್ಥಿತಿ REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, ಆರಂಭ_2); /* offset is wrong... */
    repeat(p, ಆರಂಭ_2 + 1, 1, ಗಮ್ಯ_2);
    ASTERN(OOR1, ಆರಂಭ_2);
    AHEAD(ಆರಂಭ_2); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(1, 1): /* trivial case */
    /*ಮುಗಿದ*/
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, ಆರಂಭ_2);
    ASTERN(OOR1, ಆರಂಭ_2);
    AHEAD(ಆರಂಭ_2);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, ಆರಂಭ_2 + 1, ಮುಕ್ತಾಯ + 1);
    ಪ್ರತಿಜ್ಞಾ(copy == ಮುಕ್ತಾಯ + 4);
    repeat(p, copy, 1, ಗಮ್ಯ_2 - 1);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(1, INF): /* as x+ */
    ಸೇರಿಸು_3(OPLUS_, ಆರಂಭ_2);
    ASTERN(O_PLUS, ಆರಂಭ_2);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, ಆರಂಭ_2, ಮುಕ್ತಾಯ);
    repeat(p, copy, from - 1, ಗಮ್ಯ_2 - 1);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, ಆರಂಭ_2, ಮುಕ್ತಾಯ);
    repeat(p, copy, from - 1, ಗಮ್ಯ_2);
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    ನಿಲ್ಲಿಸು;
  }
}

/*
 - seterr - set an error condition
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* useless but makes type checking happy */
seterr(ರಚನೆ parse *p, ಪೂರ್ಣಾಂಕ e) {
  ಆದರೆ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->ಮುಂದಿನ = nuls; /* try to bring things to a halt */
  p->ಕೊನೆ = nuls;
  ಹಿಂತಿರುಗಿ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ಸ್ಥಿರ cset *allocset(ರಚನೆ parse *p) {
  ಪೂರ್ಣಾಂಕ no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (no >= p->ncsalloc) { /* need another column of space */
    ಶೂನ್ಯ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ಆದರೆ (nc > SIZE_MAX / ಗಾತ್ರ(cset))
      ಹೋಗು nomem;
    ಪ್ರತಿಜ್ಞಾ(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((ಅಕ್ಷರ *)p->g->sets, nc * ಗಾತ್ರ(cset));
    ಆದರೆ (ptr == ಖಾಲಿ)
      ಹೋಗು nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((ಅಕ್ಷರ *)p->g->setbits, nbytes);
    ಆದರೆ (ptr == ಖಾಲಿ)
      ಹೋಗು nomem;
    p->g->setbits = ptr;

    ಗಾಗಿ (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (ಶೂನ್ಯ)memset((ಅಕ್ಷರ *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ಆದರೆ (p->g->sets == ಖಾಲಿ || p->g->setbits == ಖಾಲಿ)
    ಹೋಗು nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->ಮುಖವಾಡ_2 = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ಖಾಲಿ;

  ಹಿಂತಿರುಗಿ (cs);
nomem:
  ಬಿಡುಗಡೆ(p->g->sets);
  p->g->sets = ಖಾಲಿ;
  ಬಿಡುಗಡೆ(p->g->setbits);
  p->g->setbits = ಖಾಲಿ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ಹಿಂತಿರುಗಿ (ಖಾಲಿ);
}

/*
 - freeset - free a now-unused set
 */
ಸ್ಥಿರ ಶೂನ್ಯ freeset(ರಚನೆ parse *p, cset *cs) {
  size_t i;
  cset *ತುದಿ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  ಗಾಗಿ (i = 0; i < css; i++)
    CHsub(cs, i);
  ಆದರೆ (cs == ತುದಿ_2 - 1) /* recover only the easy case */
    p->g->ncsets--;
}

/*
 - freezeset - final processing on a set of characters
 *
 * The main task here is merging identical sets.  This is usually a waste
 * of time (although the hash code minimizes the overhead), but can win
 * big if REG_ICASE is being used.  REG_ICASE, by the way, is why the hash
 * is done using addition rather than xor -- all ASCII [aA] sets xor to
 * the same value!
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* set number */
freezeset(ರಚನೆ parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *ತುದಿ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  ಗಾಗಿ (cs2 = &p->g->sets[0]; cs2 < ತುದಿ_2; cs2++)
    ಆದರೆ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      ಗಾಗಿ (i = 0; i < css; i++)
        ಆದರೆ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          ನಿಲ್ಲಿಸು; /* no */
      ಆದರೆ (i == css)
        ನಿಲ್ಲಿಸು; /* yes */
    }

  ಆದರೆ (cs2 < ತುದಿ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ಹಿಂತಿರುಗಿ ((ಪೂರ್ಣಾಂಕ)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* character; there is no "none" value */
firstch(ರಚನೆ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  ಗಾಗಿ (i = 0; i < css; i++)
    ಆದರೆ (CHIN(cs, i))
      ಹಿಂತಿರುಗಿ ((ಅಕ್ಷರ)i);
  ಪ್ರತಿಜ್ಞಾ(never);
  ಹಿಂತಿರುಗಿ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nch(ರಚನೆ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  ಪೂರ್ಣಾಂಕ n = 0;

  ಗಾಗಿ (i = 0; i < css; i++)
    ಆದರೆ (CHIN(cs, i))
      n++;
  ಹಿಂತಿರುಗಿ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ಸ್ಥಿರ ಶೂನ್ಯ mcadd(ರಚನೆ parse *p, cset *cs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp) {
  size_t oldend = cs->smultis;
  ಶೂನ್ಯ *np;

  cs->smultis += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ಆದರೆ (np == ಖಾಲಿ) {
    ಆದರೆ (cs->multis)
      ಬಿಡುಗಡೆ(cs->multis);
    cs->multis = ಖಾಲಿ;
    SETERROR(REG_ESPACE);
    ಹಿಂತಿರುಗಿ;
  }
  cs->multis = np;

  llvm_strlcpy(cs->multis + oldend - 1, cp, cs->smultis - oldend + 1);
}

/*
 - mcinvert - invert the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ಸ್ಥಿರ ಶೂನ್ಯ mcinvert(ರಚನೆ parse *p, cset *cs) {
  ಪ್ರತಿಜ್ಞಾ(cs->multis == ಖಾಲಿ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ಸ್ಥಿರ ಶೂನ್ಯ mccase(ರಚನೆ parse *p, cset *cs) {
  ಪ್ರತಿಜ್ಞಾ(cs->multis == ಖಾಲಿ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* predicate */
isinsets(ರಚನೆ re_guts *g, ಪೂರ್ಣಾಂಕ c) {
  uch *ಲಂಬಸಾಲು_2;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  ಚಿಹ್ನೆರಹಿತ uc = (uch)c;

  ಗಾಗಿ (i = 0, ಲಂಬಸಾಲು_2 = g->setbits; i < ncols; i++, ಲಂಬಸಾಲು_2 += g->csetsize)
    ಆದರೆ (ಲಂಬಸಾಲು_2[uc] != 0)
      ಹಿಂತಿರುಗಿ (1);
  ಹಿಂತಿರುಗಿ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ /* predicate */
samesets(ರಚನೆ re_guts *g, ಪೂರ್ಣಾಂಕ c1, ಪೂರ್ಣಾಂಕ c2) {
  uch *ಲಂಬಸಾಲು_2;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  ಚಿಹ್ನೆರಹಿತ uc1 = (uch)c1;
  ಚಿಹ್ನೆರಹಿತ uc2 = (uch)c2;

  ಗಾಗಿ (i = 0, ಲಂಬಸಾಲು_2 = g->setbits; i < ncols; i++, ಲಂಬಸಾಲು_2 += g->csetsize)
    ಆದರೆ (ಲಂಬಸಾಲು_2[uc1] != ಲಂಬಸಾಲು_2[uc2])
      ಹಿಂತಿರುಗಿ (0);
  ಹಿಂತಿರುಗಿ (1);
}

/*
 - categorize - sort out character categories
 */
ಸ್ಥಿರ ಶೂನ್ಯ categorize(ರಚನೆ parse *p, ರಚನೆ re_guts *g) {
  cat_t *cats = g->categories;
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ c2;
  cat_t cat;

  /* avoid making error situations worse */
  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ;

  ಗಾಗಿ (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ಆದರೆ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      ಗಾಗಿ (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ಆದರೆ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ಸ್ಥಿರ sopno                       /* start of duplicate */
dupl(ರಚನೆ parse *p, sopno ಆರಂಭ_2, /* from here */
     sopno ಮುಕ್ತಾಯ)                 /* to this less one */
{
  sopno ret = ಇಲ್ಲಿ();
  sopno len = ಮುಕ್ತಾಯ - ಆರಂಭ_2;

  ಪ್ರತಿಜ್ಞಾ(ಮುಕ್ತಾಯ >= ಆರಂಭ_2);
  ಆದರೆ (len == 0)
    ಹಿಂತಿರುಗಿ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ಪ್ರತಿಜ್ಞಾ(p->ssize >= p->slen + len);
  (ಶೂನ್ಯ)memmove((ಅಕ್ಷರ *)(p->strip + p->slen), (ಅಕ್ಷರ *)(p->strip + ಆರಂಭ_2),
                (size_t)len * ಗಾತ್ರ(sop));
  p->slen += len;
  ಹಿಂತಿರುಗಿ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ಸ್ಥಿರ ಶೂನ್ಯ doemit(ರಚನೆ parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ;

  /* deal with oversize operands ("can't happen", more or less) */
  ಪ್ರತಿಜ್ಞಾ(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ಆದರೆ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ಪ್ರತಿಜ್ಞಾ(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ doinsert(ರಚನೆ parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  ಪೂರ್ಣಾಂಕ i;

  /* avoid making error situations worse */
  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ;

  sn = ಇಲ್ಲಿ();
  EMIT(op, opnd); /* do checks, ensure space */
  ಪ್ರತಿಜ್ಞಾ(ಇಲ್ಲಿ() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ಪ್ರತಿಜ್ಞಾ(pos > 0);
  ಗಾಗಿ (i = 1; i < NPAREN; i++) {
    ಆದರೆ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ಆದರೆ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((ಅಕ್ಷರ *)&p->strip[pos + 1], (ಅಕ್ಷರ *)&p->strip[pos],
          (ಇಲ್ಲಿ() - pos - 1) * ಗಾತ್ರ(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ಸ್ಥಿರ ಶೂನ್ಯ dofwd(ರಚನೆ parse *p, sopno pos, sop ಮೌಲ್ಯ_2) {
  /* avoid making error situations worse */
  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ;

  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | ಮೌಲ್ಯ_2;
}

/*
 - enlarge - enlarge the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ enlarge(ರಚನೆ parse *p, sopno ಗಾತ್ರ_3) {
  sop *sp;

  ಆದರೆ (p->ssize >= ಗಾತ್ರ_3)
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ ((uintptr_t)ಗಾತ್ರ_3 > SIZE_MAX / ಗಾತ್ರ(sop)) {
    SETERROR(REG_ESPACE);
    ಹಿಂತಿರುಗಿ;
  }

  sp = (sop *)realloc(p->strip, ಗಾತ್ರ_3 * ಗಾತ್ರ(sop));
  ಆದರೆ (sp == ಖಾಲಿ) {
    SETERROR(REG_ESPACE);
    ಹಿಂತಿರುಗಿ;
  }
  p->strip = sp;
  p->ssize = ಗಾತ್ರ_3;
}

/*
 - stripsnug - compact the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ stripsnug(ರಚನೆ parse *p, ರಚನೆ re_guts *g) {
  g->nstates = p->slen;
  ಆದರೆ ((uintptr_t)p->slen > SIZE_MAX / ಗಾತ್ರ(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ಹಿಂತಿರುಗಿ;
  }

  g->strip = (sop *)realloc((ಅಕ್ಷರ *)p->strip, p->slen * ಗಾತ್ರ(sop));
  ಆದರೆ (g->strip == ಖಾಲಿ) {
    SETERROR(REG_ESPACE);
    g->strip = p->strip;
  }
}

/*
 - findmust - fill in must and mlen with longest mandatory literal string
 *
 * This algorithm could do fancy things like analyzing the operands of |
 * for common subsequences.  Someday.  This code is simple and finds most
 * of the interesting cases.
 *
 * Note that must and mlen got initialized during setup.
 */
ಸ್ಥಿರ ಶೂನ್ಯ findmust(ರಚನೆ parse *p, ರಚನೆ re_guts *g) {
  sop *ಪರಿಶೀಲನೆ;
  sop *ಆರಂಭ_2 = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  ಅಕ್ಷರ *cp;
  sopno i;

  /* avoid making error situations worse */
  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  ಪರಿಶೀಲನೆ = g->strip + 1;
  ಮಾಡು {
    s = *ಪರಿಶೀಲನೆ++;
    ಆಯ್ಕೆ (OP(s)) {
    ಸ್ಥಿತಿ OCHAR:        /* sequence member */
      ಆದರೆ (newlen == 0) /* new sequence */
        newstart = ಪರಿಶೀಲನೆ - 1;
      newlen++;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OPLUS_: /* things that don't break one */
    ಸ್ಥಿತಿ OLPAREN:
    ಸ್ಥಿತಿ ORPAREN:
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OQUEST_: /* things that must be skipped */
    ಸ್ಥಿತಿ OCH_:
      ಪರಿಶೀಲನೆ--;
      ಮಾಡು {
        ಪರಿಶೀಲನೆ += OPND(s);
        s = *ಪರಿಶೀಲನೆ;
        /* assert() interferes w debug printouts */
        ಆದರೆ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ಹಿಂತಿರುಗಿ;
        }
      } ಆಗ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    ಪೂರ್ವನಿಯೋಜಿತ:                  /* things that break a sequence */
      ಆದರೆ (newlen > g->mlen) { /* ends one */
        ಆರಂಭ_2 = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      ನಿಲ್ಲಿಸು;
    }
  } ಆಗ (OP(s) != OEND);

  ಆದರೆ (g->mlen == 0) /* there isn't one */
    ಹಿಂತಿರುಗಿ;

  /* turn it into a character string */
  g->must = ಸ್ಮರಣೆ_ಕೊಡು((size_t)g->mlen + 1);
  ಆದರೆ (g->must == ಖಾಲಿ) { /* argh; just forget it */
    g->mlen = 0;
    ಹಿಂತಿರುಗಿ;
  }
  cp = g->must;
  ಪರಿಶೀಲನೆ = ಆರಂಭ_2;
  ಗಾಗಿ (i = g->mlen; i > 0; i--) {
    ಆಗ (OP(s = *ಪರಿಶೀಲನೆ++) != OCHAR)
      ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ(cp < g->must + g->mlen);
    *cp++ = (ಅಕ್ಷರ)OPND(s);
  }
  ಪ್ರತಿಜ್ಞಾ(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ಸ್ಥಿರ sopno /* nesting depth */
pluscount(ರಚನೆ parse *p, ರಚನೆ re_guts *g) {
  sop *ಪರಿಶೀಲನೆ;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ಆದರೆ (p->error != 0)
    ಹಿಂತಿರುಗಿ (0); /* there may not be an OEND */

  ಪರಿಶೀಲನೆ = g->strip + 1;
  ಮಾಡು {
    s = *ಪರಿಶೀಲನೆ++;
    ಆಯ್ಕೆ (OP(s)) {
    ಸ್ಥಿತಿ OPLUS_:
      plusnest++;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ O_PLUS:
      ಆದರೆ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      ನಿಲ್ಲಿಸು;
    }
  } ಆಗ (OP(s) != OEND);
  ಆದರೆ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ಹಿಂತಿರುಗಿ (maxnest);
}
