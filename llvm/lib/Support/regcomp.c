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

#ਅਨ੍ਤਰ੍ਭਾਵ "regex_impl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "regex2.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "regutils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"

/* character-class table */
ਸਥਿਰ ਬਣਤਰ cclass {
  ਅਚਲ ਅੱਖਰ *name;
  ਅਚਲ ਅੱਖਰ *chars;
  ਅਚਲ ਅੱਖਰ *multis;
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
    {ਨਿਰਾ, 0, ""}};

/* character-name table */
ਸਥਿਰ ਬਣਤਰ cname {
  ਅਚਲ ਅੱਖਰ *name;
  ਅੱਖਰ ਕੋਡ_2;
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
              {ਨਿਰਾ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ਬਣਤਰ parse {
  ਅਚਲ ਅੱਖਰ *ਅਗਲਾ; /* next character in RE */
  ਅਚਲ ਅੱਖਰ *ਸਿਰਾ;  /* end of string (-> NUL normally) */
  ਪੂਰਨ error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  ਪੂਰਨ ncsalloc;     /* number of csets allocated */
  ਬਣਤਰ re_guts *g;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ਸਥਿਰ ਖਾਲੀ p_ere(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ p_ere_exp(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ p_str(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ p_bre(ਬਣਤਰ parse *, ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ p_simp_re(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ p_ਗਿਣਤੀ(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ p_bracket(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ p_b_ਪਦ(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਖਾਲੀ p_b_cclass(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਖਾਲੀ p_b_eclass(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਅੱਖਰ p_b_ਚਿੰਨ੍ਹ(ਬਣਤਰ parse *);
ਸਥਿਰ ਅੱਖਰ p_b_coll_elem(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਅੱਖਰ othercase(ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ bothcases(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ uncased(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ ordinary(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ nonnewline(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ repeat(ਬਣਤਰ parse *, sopno, ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ seterr(ਬਣਤਰ parse *, ਪੂਰਨ);
ਸਥਿਰ cset *allocset(ਬਣਤਰ parse *);
ਸਥਿਰ ਖਾਲੀ freeset(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਪੂਰਨ freezeset(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਪੂਰਨ firstch(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਪੂਰਨ nch(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਖਾਲੀ mcadd(ਬਣਤਰ parse *, cset *, ਅਚਲ ਅੱਖਰ *);
ਸਥਿਰ ਖਾਲੀ mcinvert(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਖਾਲੀ mccase(ਬਣਤਰ parse *, cset *);
ਸਥਿਰ ਪੂਰਨ isinsets(ਬਣਤਰ re_guts *, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ samesets(ਬਣਤਰ re_guts *, ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਖਾਲੀ categorize(ਬਣਤਰ parse *, ਬਣਤਰ re_guts *);
ਸਥਿਰ sopno dupl(ਬਣਤਰ parse *, sopno, sopno);
ਸਥਿਰ ਖਾਲੀ doemit(ਬਣਤਰ parse *, sop, size_t);
ਸਥਿਰ ਖਾਲੀ doinsert(ਬਣਤਰ parse *, sop, size_t, sopno);
ਸਥਿਰ ਖਾਲੀ dofwd(ਬਣਤਰ parse *, sopno, sop);
ਸਥਿਰ ਖਾਲੀ enlarge(ਬਣਤਰ parse *, sopno);
ਸਥਿਰ ਖਾਲੀ stripsnug(ਬਣਤਰ parse *, ਬਣਤਰ re_guts *);
ਸਥਿਰ ਖਾਲੀ findmust(ਬਣਤਰ parse *, ਬਣਤਰ re_guts *);
ਸਥਿਰ sopno pluscount(ਬਣਤਰ parse *, ਬਣਤਰ re_guts *);

ਸਥਿਰ ਅੱਖਰ nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PEEK() (*p->ਅਗਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PEEK2() (*(p->ਅਗਲਾ + 1))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਹੋਰ() (p->ਸਿਰਾ - p->ਅਗਲਾ > 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MORE2() (p->ਸਿਰਾ - p->ਅਗਲਾ > 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEE(c) (ਹੋਰ() && PEEK() == (c))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EAT(c) ((SEE(c)) ? (ਅਗਲਾ_3(), 1) : 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_3() (p->ਅਗਲਾ++)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NEXT2() (p->ਅਗਲਾ += 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NEXTn(n) (p->ਅਗਲਾ += (n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GETNEXT() (*p->ਅਗਲਾ++)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETERROR(e) seterr(p, (e))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REQUIRE(co, e) (ਖਾਲੀ)((co) || SETERROR(e))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUSTSEE(c, e) (REQUIRE(ਹੋਰ() && PEEK() == (c), e))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUSTEAT(c, e) (REQUIRE(ਹੋਰ() && GETNEXT() == (c), e))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUSTNOTSEE(c, e) (REQUIRE(!ਹੋਰ() || PEEK() != (c), e))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_3(op, pos) doinsert(p, (sop)(op), ਇੱਥੇ() - (pos) + 1, pos)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHEAD(pos) dofwd(p, pos, ਇੱਥੇ() - (pos))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASTERN(sop, pos) EMIT(sop, ਇੱਥੇ() - pos)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਇੱਥੇ() (p->slen)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THERE() (p->slen - 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THERETHERE() (p->slen - 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DROP(n) (p->slen -= (n))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _POSIX2_RE_DUP_MAX
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DUPMAX _POSIX2_RE_DUP_MAX
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DUPMAX 255
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REGINFINITY (DUPMAX + 1)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਸਥਿਰ ਪੂਰਨ never = 0; /* for use in asserts; shuts lint up */
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ never 0 /* some <assert.h>s have bugs too */
#ਜੇ_ਅੰਤ

/*
 - llvm_regcomp - interface for parser and compilation
 */
ਪੂਰਨ /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ_2, ਪੂਰਨ cflags) {
  ਬਣਤਰ parse pa;
  ਬਣਤਰ re_guts *g;
  ਬਣਤਰ parse *p = &pa;
  ਪੂਰਨ i;
  size_t len;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ REDEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GOODFLAGS(f) (f)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GOODFLAGS(f) ((f) & ~REG_DUMP)
#ਜੇ_ਅੰਤ

  cflags = GOODFLAGS(cflags);
  ਜੇ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ਵਾਪਸ (REG_INVARG);

  ਜੇ (cflags & REG_PEND) {
    ਜੇ (preg->re_endp < ਨਮੂਨਾ_2)
      ਵਾਪਸ (REG_INVARG);
    len = preg->re_endp - ਨਮੂਨਾ_2;
  } ਨਹੀਂਤਾਂ {
    len = ਲੜੀ_ਲੰਬਾਈ(ਨਮੂਨਾ_2);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ਬਣਤਰ re_guts *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ re_guts) +
                               (NC - 1) * ਆਕਾਰ(cat_t));
  ਜੇ (g == ਨਿਰਾ)
    ਵਾਪਸ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(p->ssize, ਆਕਾਰ(sop));
  p->slen = 0;
  ਜੇ (p->strip == ਨਿਰਾ) {
    ਮੁਕਤ((ਅੱਖਰ *)g);
    ਵਾਪਸ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->ਅਗਲਾ = ਨਮੂਨਾ_2;
  p->ਸਿਰਾ = p->ਅਗਲਾ + len;
  p->error = 0;
  p->ncsalloc = 0;
  ਲਈ (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ਨਿਰਾ;
  g->setbits = ਨਿਰਾ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ਨਿਰਾ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (ਖਾਲੀ)memset((ਅੱਖਰ *)g->catspace, 0, NC * ਆਕਾਰ(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ਜੇ (cflags & REG_EXTENDED)
    p_ere(p, ਬਾਹਰਲਾ_3);
  ਨਹੀਂਤਾਂ ਜੇ (cflags & REG_NOSPEC)
    p_str(p);
  ਨਹੀਂਤਾਂ
    p_bre(p, ਬਾਹਰਲਾ_3, ਬਾਹਰਲਾ_3);
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  ਜੇ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#ਜੇ_ਅੰਤ

  /* win or lose, we're done */
  ਜੇ (p->error != 0) /* lose */
    llvm_regfree(preg);
  ਵਾਪਸ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ਸਥਿਰ ਖਾਲੀ p_ere(ਬਣਤਰ parse *p,
                  ਪੂਰਨ stop) /* character this ERE should end at */
{
  ਅੱਖਰ c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  ਪੂਰਨ ਪਹਿਲਾ_2 = 1; /* is this the first alternative? */

  ਲਈ (;;) {
    /* do a bunch of concatenated expressions */
    conc = ਇੱਥੇ();
    ਜਦੋਂ (ਹੋਰ() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(ਇੱਥੇ() != conc, REG_ਖਾਲੀ_ਥਾਵਾਂ); /* require nonempty */

    ਜੇ (!EAT('|'))
      ਰੋਕੋ; /* NOTE BREAK OUT */

    ਜੇ (ਪਹਿਲਾ_2) {
      ਪਾਓ_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      ਪਹਿਲਾ_2 = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = ਇੱਥੇ();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  ਜੇ (!ਪਹਿਲਾ_2) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(!ਹੋਰ() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ਸਥਿਰ ਖਾਲੀ p_ere_exp(ਬਣਤਰ parse *p) {
  ਅੱਖਰ c;
  sopno pos;
  ਪੂਰਨ ਗਿਣਤੀ_2;
  ਪੂਰਨ count2;
  ਪੂਰਨ backrefnum;
  sopno subno;
  ਪੂਰਨ wascaret = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਹੋਰ()); /* caller should have ensured this */
  c = GETNEXT();

  pos = ਇੱਥੇ();
  ਚੋਣ (c) {
  ਹਾਲਤ '(':
    REQUIRE(ਹੋਰ(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ਜੇ (subno < NPAREN)
      p->pbegin[subno] = ਇੱਥੇ();
    EMIT(OLPAREN, subno);
    ਜੇ (!SEE(')'))
      p_ere(p, ')');
    ਜੇ (subno < NPAREN) {
      p->pend[subno] = ਇੱਥੇ();
      ਪ੍ਰਤਿਜ੍ਞਾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ POSIX_MISTAKE
  ਹਾਲਤ ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    ਰੋਕੋ;
#ਜੇ_ਅੰਤ
  ਹਾਲਤ '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    ਰੋਕੋ;
  ਹਾਲਤ '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    ਰੋਕੋ;
  ਹਾਲਤ '|':
    SETERROR(REG_ਖਾਲੀ_ਥਾਵਾਂ);
    ਰੋਕੋ;
  ਹਾਲਤ '*':
  ਹਾਲਤ '+':
  ਹਾਲਤ '?':
    SETERROR(REG_BADRPT);
    ਰੋਕੋ;
  ਹਾਲਤ '.':
    ਜੇ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ਨਹੀਂਤਾਂ
      EMIT(OANY, 0);
    ਰੋਕੋ;
  ਹਾਲਤ '[':
    p_bracket(p);
    ਰੋਕੋ;
  ਹਾਲਤ '\\':
    REQUIRE(ਹੋਰ(), REG_EESCAPE);
    c = GETNEXT();
    ਜੇ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ਨਹੀਂਤਾਂ ਜੇ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ਜਦੋਂ (ਹੋਰ() && ਅੰਕ_ਹੈ(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ਨਹੀਂਤਾਂ ਜੇ (c == 'n') {
      ordinary(p, '\n');
      ਰੋਕੋ;
    } ਨਹੀਂਤਾਂ ਜੇ (c == 't') {
      ordinary(p, '\t');
      ਰੋਕੋ;
    } ਨਹੀਂਤਾਂ ਜੇ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      ਅੱਖਰ hexstr[3] = {0};
      ਅੱਖਰ *hexp;
      ਪੂਰਨ val;
      ਜੇ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ਜੇ (*hexp == '\0') {
          NEXT2();
          uncased(p, (ਅੱਖਰ)val);
          ਰੋਕੋ;
        }
      }
      ordinary(p, 'x');
      ਰੋਕੋ;
    } ਨਹੀਂਤਾਂ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      ਰੋਕੋ;
    }

    ਜੇ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      ਰੋਕੋ;
    }

    ਜੇ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      ਰੋਕੋ;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    ਪ੍ਰਤਿਜ੍ਞਾ(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    ਪ੍ਰਤਿਜ੍ਞਾ(p->pbegin[backrefnum] != 0);
    ਪ੍ਰਤਿਜ੍ਞਾ(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    ਪ੍ਰਤਿਜ੍ਞਾ(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (ਖਾਲੀ)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_ਪਿੱਠ, backrefnum);
    p->g->backrefs = 1;
    ਰੋਕੋ;
  ਹਾਲਤ '{': /* okay as ordinary except if digit follows */
    REQUIRE(!ਹੋਰ() || !ਅੰਕ_ਹੈ((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  ਮੂਲ:
    ordinary(p, c);
    ਰੋਕੋ;
  }

  ਜੇ (!ਹੋਰ())
    ਵਾਪਸ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ਜੇ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ਅੰਕ_ਹੈ((uch)PEEK2()))))
    ਵਾਪਸ; /* no repetition, we're done */
  ਅਗਲਾ_3();

  REQUIRE(!wascaret, REG_BADRPT);
  ਚੋਣ (c) {
  ਹਾਲਤ '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ਪਾਓ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ਪਾਓ_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    ਰੋਕੋ;
  ਹਾਲਤ '+':
    ਪਾਓ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ਰੋਕੋ;
  ਹਾਲਤ '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ਪਾਓ_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    ਰੋਕੋ;
  ਹਾਲਤ '{':
    ਗਿਣਤੀ_2 = p_ਗਿਣਤੀ(p);
    ਜੇ (EAT(',')) {
      ਜੇ (ਅੰਕ_ਹੈ((uch)PEEK())) {
        count2 = p_ਗਿਣਤੀ(p);
        REQUIRE(ਗਿਣਤੀ_2 <= count2, REG_BADBR);
      } ਨਹੀਂਤਾਂ /* single number with comma */
        count2 = REGINFINITY;
    } ਨਹੀਂਤਾਂ /* just a single number */
      count2 = ਗਿਣਤੀ_2;
    repeat(p, pos, ਗਿਣਤੀ_2, count2);
    ਜੇ (!EAT('}')) { /* error heuristics */
      ਜਦੋਂ (ਹੋਰ() && PEEK() != '}')
        ਅਗਲਾ_3();
      REQUIRE(ਹੋਰ(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    ਰੋਕੋ;
  }

  ਜੇ (!ਹੋਰ())
    ਵਾਪਸ;
  c = PEEK();
  ਜੇ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ਅੰਕ_ਹੈ((uch)PEEK2()))))
    ਵਾਪਸ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ਸਥਿਰ ਖਾਲੀ p_str(ਬਣਤਰ parse *p) {
  REQUIRE(ਹੋਰ(), REG_ਖਾਲੀ_ਥਾਵਾਂ);
  ਜਦੋਂ (ਹੋਰ())
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
ਸਥਿਰ ਖਾਲੀ p_bre(ਬਣਤਰ parse *p, ਪੂਰਨ end1, /* first terminating character */
                  ਪੂਰਨ end2)                  /* second terminating character */
{
  sopno ਸ਼ੁਰੂ_2 = ਇੱਥੇ();
  ਪੂਰਨ ਪਹਿਲਾ_2 = 1; /* first subexpression? */
  ਪੂਰਨ wasdollar = 0;

  ਜੇ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ਜਦੋਂ (ਹੋਰ() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, ਪਹਿਲਾ_2);
    ਪਹਿਲਾ_2 = 0;
  }
  ਜੇ (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(ਇੱਥੇ() != ਸ਼ੁਰੂ_2, REG_ਖਾਲੀ_ਥਾਵਾਂ); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
ਸਥਿਰ ਪੂਰਨ /* was the simple RE an unbackslashed $? */
p_simp_re(ਬਣਤਰ parse *p,
          ਪੂਰਨ starordinary) /* is a leading * an ordinary character? */
{
  ਪੂਰਨ c;
  ਪੂਰਨ ਗਿਣਤੀ_2;
  ਪੂਰਨ count2;
  sopno pos;
  ਪੂਰਨ i;
  sopno subno;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BACKSL (1 << CHAR_BIT)

  pos = ਇੱਥੇ(); /* repetition op, if any, covers from here */

  ਪ੍ਰਤਿਜ੍ਞਾ(ਹੋਰ()); /* caller should have ensured this */
  c = GETNEXT();
  ਜੇ (c == '\\') {
    REQUIRE(ਹੋਰ(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  ਚੋਣ (c) {
  ਹਾਲਤ '.':
    ਜੇ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ਨਹੀਂਤਾਂ
      EMIT(OANY, 0);
    ਰੋਕੋ;
  ਹਾਲਤ '[':
    p_bracket(p);
    ਰੋਕੋ;
  ਹਾਲਤ BACKSL | '{':
    SETERROR(REG_BADRPT);
    ਰੋਕੋ;
  ਹਾਲਤ BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ਜੇ (subno < NPAREN)
      p->pbegin[subno] = ਇੱਥੇ();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ਜੇ (ਹੋਰ() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ਜੇ (subno < NPAREN) {
      p->pend[subno] = ਇੱਥੇ();
      ਪ੍ਰਤਿਜ੍ਞਾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    ਰੋਕੋ;
  ਹਾਲਤ BACKSL | ')': /* should not get here -- must be user */
  ਹਾਲਤ BACKSL | '}':
    SETERROR(REG_EPAREN);
    ਰੋਕੋ;
  ਹਾਲਤ BACKSL | '1':
  ਹਾਲਤ BACKSL | '2':
  ਹਾਲਤ BACKSL | '3':
  ਹਾਲਤ BACKSL | '4':
  ਹਾਲਤ BACKSL | '5':
  ਹਾਲਤ BACKSL | '6':
  ਹਾਲਤ BACKSL | '7':
  ਹਾਲਤ BACKSL | '8':
  ਹਾਲਤ BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ਪ੍ਰਤਿਜ੍ਞਾ(i < NPAREN);
    ਜੇ (p->pend[i] != 0) {
      ਪ੍ਰਤਿਜ੍ਞਾ(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ਪ੍ਰਤਿਜ੍ਞਾ(p->pbegin[i] != 0);
      ਪ੍ਰਤਿਜ੍ਞਾ(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ਪ੍ਰਤਿਜ੍ਞਾ(OP(p->strip[p->pend[i]]) == ORPAREN);
      (ਖਾਲੀ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_ਪਿੱਠ, i);
    } ਨਹੀਂਤਾਂ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    ਰੋਕੋ;
  ਹਾਲਤ '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  ਮੂਲ:
    ordinary(p, (ਅੱਖਰ)c);
    ਰੋਕੋ;
  }

  ਜੇ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ਪਾਓ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ਪਾਓ_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ਨਹੀਂਤਾਂ ਜੇ (EATTWO('\\', '{')) {
    ਗਿਣਤੀ_2 = p_ਗਿਣਤੀ(p);
    ਜੇ (EAT(',')) {
      ਜੇ (ਹੋਰ() && ਅੰਕ_ਹੈ((uch)PEEK())) {
        count2 = p_ਗਿਣਤੀ(p);
        REQUIRE(ਗਿਣਤੀ_2 <= count2, REG_BADBR);
      } ਨਹੀਂਤਾਂ /* single number with comma */
        count2 = REGINFINITY;
    } ਨਹੀਂਤਾਂ /* just a single number */
      count2 = ਗਿਣਤੀ_2;
    repeat(p, pos, ਗਿਣਤੀ_2, count2);
    ਜੇ (!EATTWO('\\', '}')) { /* error heuristics */
      ਜਦੋਂ (ਹੋਰ() && !SEETWO('\\', '}'))
        ਅਗਲਾ_3();
      REQUIRE(ਹੋਰ(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ਨਹੀਂਤਾਂ ਜੇ (c == '$') /* $ (but not \$) ends it */
    ਵਾਪਸ (1);

  ਵਾਪਸ (0);
}

/*
 - p_count - parse a repetition count
 */
ਸਥਿਰ ਪੂਰਨ /* the value */
p_ਗਿਣਤੀ(ਬਣਤਰ parse *p) {
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਪੂਰਨ ndigits = 0;

  ਜਦੋਂ (ਹੋਰ() && ਅੰਕ_ਹੈ((uch)PEEK()) && ਗਿਣਤੀ_2 <= DUPMAX) {
    ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2 * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ਗਿਣਤੀ_2 <= DUPMAX, REG_BADBR);
  ਵਾਪਸ (ਗਿਣਤੀ_2);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ਸਥਿਰ ਖਾਲੀ p_bracket(ਬਣਤਰ parse *p) {
  cset *cs;
  ਪੂਰਨ invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ਜੇ (p->ਸਿਰਾ - p->ਅਗਲਾ > 5) {
    ਜੇ (ਲੜੀ_ਤੁਲਨਾ(p->ਅਗਲਾ, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ਵਾਪਸ;
    }
    ਜੇ (ਲੜੀ_ਤੁਲਨਾ(p->ਅਗਲਾ, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ਵਾਪਸ;
    }
  }

  ਜੇ ((cs = allocset(p)) == ਨਿਰਾ) {
    /* allocset did set error status in p */
    ਵਾਪਸ;
  }

  ਜੇ (EAT('^'))
    invert++; /* make note to invert set at end */
  ਜੇ (EAT(']'))
    CHadd(cs, ']');
  ਨਹੀਂਤਾਂ ਜੇ (EAT('-'))
    CHadd(cs, '-');
  ਜਦੋਂ (ਹੋਰ() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_ਪਦ(p, cs);
  ਜੇ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ਜੇ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ਵਾਪਸ;
  }

  ਜੇ (p->g->cflags & REG_ICASE) {
    ਪੂਰਨ i;
    ਪੂਰਨ ci;

    ਲਈ (i = p->g->csetsize - 1; i >= 0; i--)
      ਜੇ (CHIN(cs, i) && ਅੱਖਰ_ਹੈ(i)) {
        ci = othercase(i);
        ਜੇ (ci != i)
          CHadd(cs, ci);
      }
    ਜੇ (cs->multis != ਨਿਰਾ)
      mccase(p, cs);
  }
  ਜੇ (invert) {
    ਪੂਰਨ i;

    ਲਈ (i = p->g->csetsize - 1; i >= 0; i--)
      ਜੇ (CHIN(cs, i))
        CHsub(cs, i);
      ਨਹੀਂਤਾਂ
        CHadd(cs, i);
    ਜੇ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ਜੇ (cs->multis != ਨਿਰਾ)
      mcinvert(p, cs);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(cs->multis == ਨਿਰਾ); /* xxx */

  ਜੇ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ਨਹੀਂਤਾਂ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ਸਥਿਰ ਖਾਲੀ p_b_ਪਦ(ਬਣਤਰ parse *p, cset *cs) {
  ਅੱਖਰ c;
  ਅੱਖਰ ਸ਼ੁਰੂ_2, ਅੰਤ_2;
  ਪੂਰਨ i;

  /* classify what we've got */
  ਚੋਣ ((ਹੋਰ()) ? PEEK() : '\0') {
  ਹਾਲਤ '[':
    c = (MORE2()) ? PEEK2() : '\0';
    ਰੋਕੋ;
  ਹਾਲਤ '-':
    SETERROR(REG_ERANGE);
    ਵਾਪਸ; /* NOTE RETURN */
    ਰੋਕੋ;
  ਮੂਲ:
    c = '\0';
    ਰੋਕੋ;
  }

  ਚੋਣ (c) {
  ਹਾਲਤ ':': /* character class */
    NEXT2();
    REQUIRE(ਹੋਰ(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(ਹੋਰ(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    ਰੋਕੋ;
  ਹਾਲਤ '=': /* equivalence class */
    NEXT2();
    REQUIRE(ਹੋਰ(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(ਹੋਰ(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    ਰੋਕੋ;
  ਮੂਲ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    ਸ਼ੁਰੂ_2 = p_b_ਚਿੰਨ੍ਹ(p);
    ਜੇ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      ਅਗਲਾ_3();
      ਜੇ (EAT('-'))
        ਅੰਤ_2 = '-';
      ਨਹੀਂਤਾਂ
        ਅੰਤ_2 = p_b_ਚਿੰਨ੍ਹ(p);
    } ਨਹੀਂਤਾਂ {
      ਅੰਤ_2 = ਸ਼ੁਰੂ_2;
    }
    /* xxx what about signed chars here... */
    REQUIRE(ਸ਼ੁਰੂ_2 <= ਅੰਤ_2, REG_ERANGE);
    ਲਈ (i = ਸ਼ੁਰੂ_2; i <= ਅੰਤ_2; i++)
      CHadd(cs, i);
    ਰੋਕੋ;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ਸਥਿਰ ਖਾਲੀ p_b_cclass(ਬਣਤਰ parse *p, cset *cs) {
  ਅਚਲ ਅੱਖਰ *sp = p->ਅਗਲਾ;
  ਬਣਤਰ cclass *cp;
  size_t len;
  ਅਚਲ ਅੱਖਰ *u;
  ਅੱਖਰ c;

  ਜਦੋਂ (ਹੋਰ() && ਅੱਖਰ_ਹੈ((uch)PEEK()))
    ਅਗਲਾ_3();
  len = p->ਅਗਲਾ - sp;
  ਲਈ (cp = cclasses; cp->name != ਨਿਰਾ; cp++)
    ਜੇ (ਲੜੀ_ਤੁਲਨਾ(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      ਰੋਕੋ;
  ਜੇ (cp->name == ਨਿਰਾ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ਵਾਪਸ;
  }

  u = cp->chars;
  ਜਦੋਂ ((c = *u++) != '\0')
    CHadd(cs, c);
  ਲਈ (u = cp->multis; *u != '\0'; u += ਲੜੀ_ਲੰਬਾਈ(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ਸਥਿਰ ਖਾਲੀ p_b_eclass(ਬਣਤਰ parse *p, cset *cs) {
  ਅੱਖਰ c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ਸਥਿਰ ਅੱਖਰ /* value of symbol */
p_b_ਚਿੰਨ੍ਹ(ਬਣਤਰ parse *p) {
  ਅੱਖਰ ਮੁੱਲ_2;

  REQUIRE(ਹੋਰ(), REG_EBRACK);
  ਜੇ (!EATTWO('[', '.'))
    ਵਾਪਸ (GETNEXT());

  /* collating symbol */
  ਮੁੱਲ_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ਵਾਪਸ (ਮੁੱਲ_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ਸਥਿਰ ਅੱਖਰ                              /* value of collating element */
p_b_coll_elem(ਬਣਤਰ parse *p, ਪੂਰਨ endc) /* name ended by endc,']' */
{
  ਅਚਲ ਅੱਖਰ *sp = p->ਅਗਲਾ;
  ਬਣਤਰ cname *cp;
  size_t len;

  ਜਦੋਂ (ਹੋਰ() && !SEETWO(endc, ']'))
    ਅਗਲਾ_3();
  ਜੇ (!ਹੋਰ()) {
    SETERROR(REG_EBRACK);
    ਵਾਪਸ (0);
  }
  len = p->ਅਗਲਾ - sp;
  ਲਈ (cp = cnames; cp->name != ਨਿਰਾ; cp++)
    ਜੇ (ਲੜੀ_ਤੁਲਨਾ(cp->name, sp, len) == 0 && ਲੜੀ_ਲੰਬਾਈ(cp->name) == len)
      ਵਾਪਸ (cp->ਕੋਡ_2); /* known name */
  ਜੇ (len == 1)
    ਵਾਪਸ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ਵਾਪਸ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ਸਥਿਰ ਅੱਖਰ /* if no counterpart, return ch */
othercase(ਪੂਰਨ ch) {
  ch = (uch)ch;
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੱਖਰ_ਹੈ(ch));
  ਜੇ (isupper(ch))
    ਵਾਪਸ ((uch)tolower(ch));
  ਨਹੀਂਤਾਂ ਜੇ (islower(ch))
    ਵਾਪਸ ((uch)ਵੱਡਾ_ਅੱਖਰ(ch));
  ਨਹੀਂਤਾਂ /* peculiar, but could happen */
    ਵਾਪਸ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ਸਥਿਰ ਖਾਲੀ bothcases(ਬਣਤਰ parse *p, ਪੂਰਨ ch) {
  ਅਚਲ ਅੱਖਰ *oldnext = p->ਅਗਲਾ;
  ਅਚਲ ਅੱਖਰ *oldend = p->ਸਿਰਾ;
  ਅੱਖਰ bracket[3];

  ch = (uch)ch;
  ਪ੍ਰਤਿਜ੍ਞਾ(othercase(ch) != ch); /* p_bracket() would recurse */
  p->ਅਗਲਾ = bracket;
  p->ਸਿਰਾ = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ਪ੍ਰਤਿਜ੍ਞਾ(p->ਅਗਲਾ == bracket + 2);
  p->ਅਗਲਾ = oldnext;
  p->ਸਿਰਾ = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ਸਥਿਰ ਖਾਲੀ uncased(ਬਣਤਰ parse *p, ਪੂਰਨ ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ਜੇ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ਸਥਿਰ ਖਾਲੀ ordinary(ਬਣਤਰ parse *p, ਪੂਰਨ ch) {
  ਜੇ ((p->g->cflags & REG_ICASE) && ਅੱਖਰ_ਹੈ((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ਨਹੀਂਤਾਂ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ਸਥਿਰ ਖਾਲੀ nonnewline(ਬਣਤਰ parse *p) {
  ਅਚਲ ਅੱਖਰ *oldnext = p->ਅਗਲਾ;
  ਅਚਲ ਅੱਖਰ *oldend = p->ਸਿਰਾ;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ bracket[4] = {'^', '\n', ']', '\0'};

  p->ਅਗਲਾ = bracket;
  p->ਸਿਰਾ = bracket + 3;
  p_bracket(p);
  ਪ੍ਰਤਿਜ੍ਞਾ(p->ਅਗਲਾ == bracket + 3);
  p->ਅਗਲਾ = oldnext;
  p->ਸਿਰਾ = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ਸਥਿਰ ਖਾਲੀ repeat(ਬਣਤਰ parse *p,
                   sopno ਸ਼ੁਰੂ_2, /* operand from here to end of strip */
                   ਪੂਰਨ from,    /* repeated from this number */
                   ਪੂਰਨ ਮੰਜ਼ਿਲ_2)      /* to this number of times (maybe INFINITY) */
{
  sopno ਅੰਤ_2 = ਇੱਥੇ();
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INF 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REP(f, t) ((f) * 8 + (t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ਜੇ (p->error != 0) /* head off possible runaway recursion */
    ਵਾਪਸ;

  ਪ੍ਰਤਿਜ੍ਞਾ(from <= ਮੰਜ਼ਿਲ_2);

  ਚੋਣ (REP(MAP(from), MAP(ਮੰਜ਼ਿਲ_2))) {
  ਹਾਲਤ REP(0, 0):         /* must be user doing this */
    DROP(ਅੰਤ_2 - ਸ਼ੁਰੂ_2); /* drop the operand */
    ਰੋਕੋ;
  ਹਾਲਤ REP(0, 1):   /* as x{1,1}? */
  ਹਾਲਤ REP(0, N):   /* as x{1,n}? */
  ਹਾਲਤ REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ਪਾਓ_3(OCH_, ਸ਼ੁਰੂ_2); /* offset is wrong... */
    repeat(p, ਸ਼ੁਰੂ_2 + 1, 1, ਮੰਜ਼ਿਲ_2);
    ASTERN(OOR1, ਸ਼ੁਰੂ_2);
    AHEAD(ਸ਼ੁਰੂ_2); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    ਰੋਕੋ;
  ਹਾਲਤ REP(1, 1): /* trivial case */
    /*ਪੂਰਾ*/
    ਰੋਕੋ;
  ਹਾਲਤ REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ਪਾਓ_3(OCH_, ਸ਼ੁਰੂ_2);
    ASTERN(OOR1, ਸ਼ੁਰੂ_2);
    AHEAD(ਸ਼ੁਰੂ_2);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, ਸ਼ੁਰੂ_2 + 1, ਅੰਤ_2 + 1);
    ਪ੍ਰਤਿਜ੍ਞਾ(copy == ਅੰਤ_2 + 4);
    repeat(p, copy, 1, ਮੰਜ਼ਿਲ_2 - 1);
    ਰੋਕੋ;
  ਹਾਲਤ REP(1, INF): /* as x+ */
    ਪਾਓ_3(OPLUS_, ਸ਼ੁਰੂ_2);
    ASTERN(O_PLUS, ਸ਼ੁਰੂ_2);
    ਰੋਕੋ;
  ਹਾਲਤ REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, ਸ਼ੁਰੂ_2, ਅੰਤ_2);
    repeat(p, copy, from - 1, ਮੰਜ਼ਿਲ_2 - 1);
    ਰੋਕੋ;
  ਹਾਲਤ REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, ਸ਼ੁਰੂ_2, ਅੰਤ_2);
    repeat(p, copy, from - 1, ਮੰਜ਼ਿਲ_2);
    ਰੋਕੋ;
  ਮੂਲ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    ਰੋਕੋ;
  }
}

/*
 - seterr - set an error condition
 */
ਸਥਿਰ ਪੂਰਨ /* useless but makes type checking happy */
seterr(ਬਣਤਰ parse *p, ਪੂਰਨ e) {
  ਜੇ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->ਅਗਲਾ = nuls; /* try to bring things to a halt */
  p->ਸਿਰਾ = nuls;
  ਵਾਪਸ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ਸਥਿਰ cset *allocset(ਬਣਤਰ parse *p) {
  ਪੂਰਨ no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  ਪੂਰਨ i;

  ਜੇ (no >= p->ncsalloc) { /* need another column of space */
    ਖਾਲੀ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ਜੇ (nc > SIZE_MAX / ਆਕਾਰ(cset))
      ਜਾਓ nomem;
    ਪ੍ਰਤਿਜ੍ਞਾ(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((ਅੱਖਰ *)p->g->sets, nc * ਆਕਾਰ(cset));
    ਜੇ (ptr == ਨਿਰਾ)
      ਜਾਓ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((ਅੱਖਰ *)p->g->setbits, nbytes);
    ਜੇ (ptr == ਨਿਰਾ)
      ਜਾਓ nomem;
    p->g->setbits = ptr;

    ਲਈ (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (ਖਾਲੀ)memset((ਅੱਖਰ *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ਜੇ (p->g->sets == ਨਿਰਾ || p->g->setbits == ਨਿਰਾ)
    ਜਾਓ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->ਨਕਾਬ_2 = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ਨਿਰਾ;

  ਵਾਪਸ (cs);
nomem:
  ਮੁਕਤ(p->g->sets);
  p->g->sets = ਨਿਰਾ;
  ਮੁਕਤ(p->g->setbits);
  p->g->setbits = ਨਿਰਾ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ਵਾਪਸ (ਨਿਰਾ);
}

/*
 - freeset - free a now-unused set
 */
ਸਥਿਰ ਖਾਲੀ freeset(ਬਣਤਰ parse *p, cset *cs) {
  size_t i;
  cset *ਸਿਖਰ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  ਲਈ (i = 0; i < css; i++)
    CHsub(cs, i);
  ਜੇ (cs == ਸਿਖਰ_2 - 1) /* recover only the easy case */
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
ਸਥਿਰ ਪੂਰਨ /* set number */
freezeset(ਬਣਤਰ parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *ਸਿਖਰ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  ਲਈ (cs2 = &p->g->sets[0]; cs2 < ਸਿਖਰ_2; cs2++)
    ਜੇ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      ਲਈ (i = 0; i < css; i++)
        ਜੇ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          ਰੋਕੋ; /* no */
      ਜੇ (i == css)
        ਰੋਕੋ; /* yes */
    }

  ਜੇ (cs2 < ਸਿਖਰ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ਵਾਪਸ ((ਪੂਰਨ)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ਸਥਿਰ ਪੂਰਨ /* character; there is no "none" value */
firstch(ਬਣਤਰ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  ਲਈ (i = 0; i < css; i++)
    ਜੇ (CHIN(cs, i))
      ਵਾਪਸ ((ਅੱਖਰ)i);
  ਪ੍ਰਤਿਜ੍ਞਾ(never);
  ਵਾਪਸ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ਸਥਿਰ ਪੂਰਨ nch(ਬਣਤਰ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  ਪੂਰਨ n = 0;

  ਲਈ (i = 0; i < css; i++)
    ਜੇ (CHIN(cs, i))
      n++;
  ਵਾਪਸ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ਸਥਿਰ ਖਾਲੀ mcadd(ਬਣਤਰ parse *p, cset *cs, ਅਚਲ ਅੱਖਰ *cp) {
  size_t oldend = cs->smultis;
  ਖਾਲੀ *np;

  cs->smultis += ਲੜੀ_ਲੰਬਾਈ(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ਜੇ (np == ਨਿਰਾ) {
    ਜੇ (cs->multis)
      ਮੁਕਤ(cs->multis);
    cs->multis = ਨਿਰਾ;
    SETERROR(REG_ESPACE);
    ਵਾਪਸ;
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
ਸਥਿਰ ਖਾਲੀ mcinvert(ਬਣਤਰ parse *p, cset *cs) {
  ਪ੍ਰਤਿਜ੍ਞਾ(cs->multis == ਨਿਰਾ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ਸਥਿਰ ਖਾਲੀ mccase(ਬਣਤਰ parse *p, cset *cs) {
  ਪ੍ਰਤਿਜ੍ਞਾ(cs->multis == ਨਿਰਾ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ਸਥਿਰ ਪੂਰਨ /* predicate */
isinsets(ਬਣਤਰ re_guts *g, ਪੂਰਨ c) {
  uch *ਖਾਨਾ_2;
  ਪੂਰਨ i;
  ਪੂਰਨ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  ਬਿਨਾਨਿਸ਼ਾਨੀ uc = (uch)c;

  ਲਈ (i = 0, ਖਾਨਾ_2 = g->setbits; i < ncols; i++, ਖਾਨਾ_2 += g->csetsize)
    ਜੇ (ਖਾਨਾ_2[uc] != 0)
      ਵਾਪਸ (1);
  ਵਾਪਸ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ਸਥਿਰ ਪੂਰਨ /* predicate */
samesets(ਬਣਤਰ re_guts *g, ਪੂਰਨ c1, ਪੂਰਨ c2) {
  uch *ਖਾਨਾ_2;
  ਪੂਰਨ i;
  ਪੂਰਨ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  ਬਿਨਾਨਿਸ਼ਾਨੀ uc1 = (uch)c1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ uc2 = (uch)c2;

  ਲਈ (i = 0, ਖਾਨਾ_2 = g->setbits; i < ncols; i++, ਖਾਨਾ_2 += g->csetsize)
    ਜੇ (ਖਾਨਾ_2[uc1] != ਖਾਨਾ_2[uc2])
      ਵਾਪਸ (0);
  ਵਾਪਸ (1);
}

/*
 - categorize - sort out character categories
 */
ਸਥਿਰ ਖਾਲੀ categorize(ਬਣਤਰ parse *p, ਬਣਤਰ re_guts *g) {
  cat_t *cats = g->categories;
  ਪੂਰਨ c;
  ਪੂਰਨ c2;
  cat_t cat;

  /* avoid making error situations worse */
  ਜੇ (p->error != 0)
    ਵਾਪਸ;

  ਲਈ (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ਜੇ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      ਲਈ (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ਜੇ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ਸਥਿਰ sopno                       /* start of duplicate */
dupl(ਬਣਤਰ parse *p, sopno ਸ਼ੁਰੂ_2, /* from here */
     sopno ਅੰਤ_2)                 /* to this less one */
{
  sopno ret = ਇੱਥੇ();
  sopno len = ਅੰਤ_2 - ਸ਼ੁਰੂ_2;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਤ_2 >= ਸ਼ੁਰੂ_2);
  ਜੇ (len == 0)
    ਵਾਪਸ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ਪ੍ਰਤਿਜ੍ਞਾ(p->ssize >= p->slen + len);
  (ਖਾਲੀ)memmove((ਅੱਖਰ *)(p->strip + p->slen), (ਅੱਖਰ *)(p->strip + ਸ਼ੁਰੂ_2),
                (size_t)len * ਆਕਾਰ(sop));
  p->slen += len;
  ਵਾਪਸ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ਸਥਿਰ ਖਾਲੀ doemit(ਬਣਤਰ parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ਜੇ (p->error != 0)
    ਵਾਪਸ;

  /* deal with oversize operands ("can't happen", more or less) */
  ਪ੍ਰਤਿਜ੍ਞਾ(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ਜੇ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ਪ੍ਰਤਿਜ੍ਞਾ(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ਸਥਿਰ ਖਾਲੀ doinsert(ਬਣਤਰ parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  ਪੂਰਨ i;

  /* avoid making error situations worse */
  ਜੇ (p->error != 0)
    ਵਾਪਸ;

  sn = ਇੱਥੇ();
  EMIT(op, opnd); /* do checks, ensure space */
  ਪ੍ਰਤਿਜ੍ਞਾ(ਇੱਥੇ() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ਪ੍ਰਤਿਜ੍ਞਾ(pos > 0);
  ਲਈ (i = 1; i < NPAREN; i++) {
    ਜੇ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ਜੇ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((ਅੱਖਰ *)&p->strip[pos + 1], (ਅੱਖਰ *)&p->strip[pos],
          (ਇੱਥੇ() - pos - 1) * ਆਕਾਰ(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ਸਥਿਰ ਖਾਲੀ dofwd(ਬਣਤਰ parse *p, sopno pos, sop ਮੁੱਲ_2) {
  /* avoid making error situations worse */
  ਜੇ (p->error != 0)
    ਵਾਪਸ;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਮੁੱਲ_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | ਮੁੱਲ_2;
}

/*
 - enlarge - enlarge the strip
 */
ਸਥਿਰ ਖਾਲੀ enlarge(ਬਣਤਰ parse *p, sopno ਮਾਪ_2) {
  sop *sp;

  ਜੇ (p->ssize >= ਮਾਪ_2)
    ਵਾਪਸ;

  ਜੇ ((uintptr_t)ਮਾਪ_2 > SIZE_MAX / ਆਕਾਰ(sop)) {
    SETERROR(REG_ESPACE);
    ਵਾਪਸ;
  }

  sp = (sop *)realloc(p->strip, ਮਾਪ_2 * ਆਕਾਰ(sop));
  ਜੇ (sp == ਨਿਰਾ) {
    SETERROR(REG_ESPACE);
    ਵਾਪਸ;
  }
  p->strip = sp;
  p->ssize = ਮਾਪ_2;
}

/*
 - stripsnug - compact the strip
 */
ਸਥਿਰ ਖਾਲੀ stripsnug(ਬਣਤਰ parse *p, ਬਣਤਰ re_guts *g) {
  g->nstates = p->slen;
  ਜੇ ((uintptr_t)p->slen > SIZE_MAX / ਆਕਾਰ(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ਵਾਪਸ;
  }

  g->strip = (sop *)realloc((ਅੱਖਰ *)p->strip, p->slen * ਆਕਾਰ(sop));
  ਜੇ (g->strip == ਨਿਰਾ) {
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
ਸਥਿਰ ਖਾਲੀ findmust(ਬਣਤਰ parse *p, ਬਣਤਰ re_guts *g) {
  sop *ਜਾਂਚ;
  sop *ਸ਼ੁਰੂ_2 = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  ਅੱਖਰ *cp;
  sopno i;

  /* avoid making error situations worse */
  ਜੇ (p->error != 0)
    ਵਾਪਸ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  ਜਾਂਚ = g->strip + 1;
  ਕਰੋ {
    s = *ਜਾਂਚ++;
    ਚੋਣ (OP(s)) {
    ਹਾਲਤ OCHAR:        /* sequence member */
      ਜੇ (newlen == 0) /* new sequence */
        newstart = ਜਾਂਚ - 1;
      newlen++;
      ਰੋਕੋ;
    ਹਾਲਤ OPLUS_: /* things that don't break one */
    ਹਾਲਤ OLPAREN:
    ਹਾਲਤ ORPAREN:
      ਰੋਕੋ;
    ਹਾਲਤ OQUEST_: /* things that must be skipped */
    ਹਾਲਤ OCH_:
      ਜਾਂਚ--;
      ਕਰੋ {
        ਜਾਂਚ += OPND(s);
        s = *ਜਾਂਚ;
        /* assert() interferes w debug printouts */
        ਜੇ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ਵਾਪਸ;
        }
      } ਜਦੋਂ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    ਮੂਲ:                  /* things that break a sequence */
      ਜੇ (newlen > g->mlen) { /* ends one */
        ਸ਼ੁਰੂ_2 = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      ਰੋਕੋ;
    }
  } ਜਦੋਂ (OP(s) != OEND);

  ਜੇ (g->mlen == 0) /* there isn't one */
    ਵਾਪਸ;

  /* turn it into a character string */
  g->must = ਸਮ੍ਰਿਤੀ_ਦਿਓ((size_t)g->mlen + 1);
  ਜੇ (g->must == ਨਿਰਾ) { /* argh; just forget it */
    g->mlen = 0;
    ਵਾਪਸ;
  }
  cp = g->must;
  ਜਾਂਚ = ਸ਼ੁਰੂ_2;
  ਲਈ (i = g->mlen; i > 0; i--) {
    ਜਦੋਂ (OP(s = *ਜਾਂਚ++) != OCHAR)
      ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ(cp < g->must + g->mlen);
    *cp++ = (ਅੱਖਰ)OPND(s);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ਸਥਿਰ sopno /* nesting depth */
pluscount(ਬਣਤਰ parse *p, ਬਣਤਰ re_guts *g) {
  sop *ਜਾਂਚ;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ਜੇ (p->error != 0)
    ਵਾਪਸ (0); /* there may not be an OEND */

  ਜਾਂਚ = g->strip + 1;
  ਕਰੋ {
    s = *ਜਾਂਚ++;
    ਚੋਣ (OP(s)) {
    ਹਾਲਤ OPLUS_:
      plusnest++;
      ਰੋਕੋ;
    ਹਾਲਤ O_PLUS:
      ਜੇ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      ਰੋਕੋ;
    }
  } ਜਦੋਂ (OP(s) != OEND);
  ਜੇ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ਵਾਪਸ (maxnest);
}
