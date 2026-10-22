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
ಸ್ಥಿರ ಸಂರಚನಾ cclass {
  ಧ್ರುವ ವರ್ಣ *name;
  ಧ್ರುವ ವರ್ಣ *chars;
  ಧ್ರುವ ವರ್ಣ *multis;
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
    {ರಿಕ್ತ, 0, ""}};

/* character-name table */
ಸ್ಥಿರ ಸಂರಚನಾ cname {
  ಧ್ರುವ ವರ್ಣ *name;
  ವರ್ಣ code;
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
              {ರಿಕ್ತ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ಸಂರಚನಾ parse {
  ಧ್ರುವ ವರ್ಣ *next; /* next character in RE */
  ಧ್ರುವ ವರ್ಣ *end;  /* end of string (-> NUL normally) */
  ಪೂರ್ಣಾಙ್ಕ error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  ಪೂರ್ಣಾಙ್ಕ ncsalloc;     /* number of csets allocated */
  ಸಂರಚನಾ re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ಸ್ಥಿರ ಶೂನ್ಯ p_ere(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ p_ere_exp(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_str(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_bre(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ p_simp_re(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ p_ಎಣಿಕೆ(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_bracket(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_term(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_cclass(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ p_b_eclass(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ವರ್ಣ p_b_ಚಿಹ್ನೆ(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ವರ್ಣ p_b_coll_elem(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ವರ್ಣ othercase(ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ bothcases(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ uncased(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ ordinary(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ nonnewline(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ repeat(ಸಂರಚನಾ parse *, sopno, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ seterr(ಸಂರಚನಾ parse *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ cset *allocset(ಸಂರಚನಾ parse *);
ಸ್ಥಿರ ಶೂನ್ಯ freeset(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ freezeset(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ firstch(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nch(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ mcadd(ಸಂರಚನಾ parse *, cset *, ಧ್ರುವ ವರ್ಣ *);
ಸ್ಥಿರ ಶೂನ್ಯ mcinvert(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಶೂನ್ಯ mccase(ಸಂರಚನಾ parse *, cset *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isinsets(ಸಂರಚನಾ re_guts *, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ samesets(ಸಂರಚನಾ re_guts *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಶೂನ್ಯ categorize(ಸಂರಚನಾ parse *, ಸಂರಚನಾ re_guts *);
ಸ್ಥಿರ sopno dupl(ಸಂರಚನಾ parse *, sopno, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ doemit(ಸಂರಚನಾ parse *, sop, size_t);
ಸ್ಥಿರ ಶೂನ್ಯ doinsert(ಸಂರಚನಾ parse *, sop, size_t, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ dofwd(ಸಂರಚನಾ parse *, sopno, sop);
ಸ್ಥಿರ ಶೂನ್ಯ enlarge(ಸಂರಚನಾ parse *, sopno);
ಸ್ಥಿರ ಶೂನ್ಯ stripsnug(ಸಂರಚನಾ parse *, ಸಂರಚನಾ re_guts *);
ಸ್ಥಿರ ಶೂನ್ಯ findmust(ಸಂರಚನಾ parse *, ಸಂರಚನಾ re_guts *);
ಸ್ಥಿರ sopno pluscount(ಸಂರಚನಾ parse *, ಸಂರಚನಾ re_guts *);

ಸ್ಥಿರ ವರ್ಣ nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#define PEEK() (*p->next)
#define PEEK2() (*(p->next + 1))
#define MORE() (p->end - p->next > 0)
#define MORE2() (p->end - p->next > 1)
#define SEE(c) (MORE() && PEEK() == (c))
#define SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#define EAT(c) ((SEE(c)) ? (NEXT(), 1) : 0)
#define EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#define NEXT() (p->next++)
#define NEXT2() (p->next += 2)
#define NEXTn(n) (p->next += (n))
#define GETNEXT() (*p->next++)
#define SETERROR(e) seterr(p, (e))
#define REQUIRE(co, e) (ಶೂನ್ಯ)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define ಸೇರಿಸು_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#ಅನ್ಯಥಾ
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ never = 0; /* for use in asserts; shuts lint up */
#ಅನ್ಯಥಾ
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
ಪೂರ್ಣಾಙ್ಕ /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ಧ್ರುವ ವರ್ಣ *pattern, ಪೂರ್ಣಾಙ್ಕ cflags) {
  ಸಂರಚನಾ parse pa;
  ಸಂರಚನಾ re_guts *g;
  ಸಂರಚನಾ parse *p = &pa;
  ಪೂರ್ಣಾಙ್ಕ i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#ಅನ್ಯಥಾ
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  ಯದಿ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ಪ್ರತ್ಯಾಗಮ (REG_INVARG);

  ಯದಿ (cflags & REG_PEND) {
    ಯದಿ (preg->re_endp < pattern)
      ಪ್ರತ್ಯಾಗಮ (REG_INVARG);
    len = preg->re_endp - pattern;
  } ಅನ್ಯಥಾ {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ಸಂರಚನಾ re_guts *)malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ re_guts) +
                               (NC - 1) * ಪರಿಮಾಣ(cat_t));
  ಯದಿ (g == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, ಪರಿಮಾಣ(sop));
  p->slen = 0;
  ಯದಿ (p->strip == ರಿಕ್ತ) {
    ಮುಕ್ತ((ವರ್ಣ *)g);
    ಪ್ರತ್ಯಾಗಮ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  ಪ್ರತಿ (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ರಿಕ್ತ;
  g->setbits = ರಿಕ್ತ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ರಿಕ್ತ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (ಶೂನ್ಯ)memset((ವರ್ಣ *)g->catspace, 0, NC * ಪರಿಮಾಣ(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ಯದಿ (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  ಅನ್ಯಥಾ ಯದಿ (cflags & REG_NOSPEC)
    p_str(p);
  ಅನ್ಯಥಾ
    p_bre(p, OUT, OUT);
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
#ifndef REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  ಯದಿ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  ಯದಿ (p->error != 0) /* lose */
    llvm_regfree(preg);
  ಪ್ರತ್ಯಾಗಮ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_ere(ಸಂರಚನಾ parse *p,
                  ಪೂರ್ಣಾಙ್ಕ stop) /* character this ERE should end at */
{
  ವರ್ಣ c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  ಪೂರ್ಣಾಙ್ಕ first = 1; /* is this the first alternative? */

  ಪ್ರತಿ (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    ಯಾವತ್ (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    ಯದಿ (!EAT('|'))
      ಭಙ್ಗ; /* NOTE BREAK OUT */

    ಯದಿ (first) {
      ಸೇರಿಸು_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      first = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = HERE();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  ಯದಿ (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ಪ್ರತಿಜ್ಞಾ(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_ere_exp(ಸಂರಚನಾ parse *p) {
  ವರ್ಣ c;
  sopno pos;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
  ಪೂರ್ಣಾಙ್ಕ count2;
  ಪೂರ್ಣಾಙ್ಕ backrefnum;
  sopno subno;
  ಪೂರ್ಣಾಙ್ಕ wascaret = 0;

  ಪ್ರತಿಜ್ಞಾ(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  ವಿಕಲ್ಪ (c) {
  ಪಕ್ಷ '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ಯದಿ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    ಯದಿ (!SEE(')'))
      p_ere(p, ')');
    ಯದಿ (subno < NPAREN) {
      p->pend[subno] = HERE();
      ಪ್ರತಿಜ್ಞಾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    ಭಙ್ಗ;
#ifndef POSIX_MISTAKE
  ಪಕ್ಷ ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    ಭಙ್ಗ;
#endif
  ಪಕ್ಷ '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    ಭಙ್ಗ;
  ಪಕ್ಷ '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    ಭಙ್ಗ;
  ಪಕ್ಷ '|':
    SETERROR(REG_EMPTY);
    ಭಙ್ಗ;
  ಪಕ್ಷ '*':
  ಪಕ್ಷ '+':
  ಪಕ್ಷ '?':
    SETERROR(REG_BADRPT);
    ಭಙ್ಗ;
  ಪಕ್ಷ '.':
    ಯದಿ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ಅನ್ಯಥಾ
      EMIT(OANY, 0);
    ಭಙ್ಗ;
  ಪಕ್ಷ '[':
    p_bracket(p);
    ಭಙ್ಗ;
  ಪಕ್ಷ '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    ಯದಿ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ಅನ್ಯಥಾ ಯದಿ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ಯಾವತ್ (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ಅನ್ಯಥಾ ಯದಿ (c == 'n') {
      ordinary(p, '\n');
      ಭಙ್ಗ;
    } ಅನ್ಯಥಾ ಯದಿ (c == 't') {
      ordinary(p, '\t');
      ಭಙ್ಗ;
    } ಅನ್ಯಥಾ ಯದಿ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      ವರ್ಣ hexstr[3] = {0};
      ವರ್ಣ *hexp;
      ಪೂರ್ಣಾಙ್ಕ val;
      ಯದಿ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ಯದಿ (*hexp == '\0') {
          NEXT2();
          uncased(p, (ವರ್ಣ)val);
          ಭಙ್ಗ;
        }
      }
      ordinary(p, 'x');
      ಭಙ್ಗ;
    } ಅನ್ಯಥಾ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      ಭಙ್ಗ;
    }

    ಯದಿ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      ಭಙ್ಗ;
    }

    ಯದಿ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      ಭಙ್ಗ;
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
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    ಭಙ್ಗ;
  ಪಕ್ಷ '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  ಸ್ವಭಾವ:
    ordinary(p, c);
    ಭಙ್ಗ;
  }

  ಯದಿ (!MORE())
    ಪ್ರತ್ಯಾಗಮ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ಯದಿ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ಪ್ರತ್ಯಾಗಮ; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  ವಿಕಲ್ಪ (c) {
  ಪಕ್ಷ '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ಸೇರಿಸು_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    ಭಙ್ಗ;
  ಪಕ್ಷ '+':
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ಭಙ್ಗ;
  ಪಕ್ಷ '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    ಭಙ್ಗ;
  ಪಕ್ಷ '{':
    ಎಣಿಕೆ = p_ಎಣಿಕೆ(p);
    ಯದಿ (EAT(',')) {
      ಯದಿ (isdigit((uch)PEEK())) {
        count2 = p_ಎಣಿಕೆ(p);
        REQUIRE(ಎಣಿಕೆ <= count2, REG_BADBR);
      } ಅನ್ಯಥಾ /* single number with comma */
        count2 = REGINFINITY;
    } ಅನ್ಯಥಾ /* just a single number */
      count2 = ಎಣಿಕೆ;
    repeat(p, pos, ಎಣಿಕೆ, count2);
    ಯದಿ (!EAT('}')) { /* error heuristics */
      ಯಾವತ್ (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    ಭಙ್ಗ;
  }

  ಯದಿ (!MORE())
    ಪ್ರತ್ಯಾಗಮ;
  c = PEEK();
  ಯದಿ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ಪ್ರತ್ಯಾಗಮ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_str(ಸಂರಚನಾ parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  ಯಾವತ್ (MORE())
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
ಸ್ಥಿರ ಶೂನ್ಯ p_bre(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ end1, /* first terminating character */
                  ಪೂರ್ಣಾಙ್ಕ end2)                  /* second terminating character */
{
  sopno start = HERE();
  ಪೂರ್ಣಾಙ್ಕ first = 1; /* first subexpression? */
  ಪೂರ್ಣಾಙ್ಕ wasdollar = 0;

  ಯದಿ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ಯಾವತ್ (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  ಯದಿ (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(HERE() != start, REG_EMPTY); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* was the simple RE an unbackslashed $? */
p_simp_re(ಸಂರಚನಾ parse *p,
          ಪೂರ್ಣಾಙ್ಕ starordinary) /* is a leading * an ordinary character? */
{
  ಪೂರ್ಣಾಙ್ಕ c;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
  ಪೂರ್ಣಾಙ್ಕ count2;
  sopno pos;
  ಪೂರ್ಣಾಙ್ಕ i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  ಪ್ರತಿಜ್ಞಾ(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  ಯದಿ (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  ವಿಕಲ್ಪ (c) {
  ಪಕ್ಷ '.':
    ಯದಿ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ಅನ್ಯಥಾ
      EMIT(OANY, 0);
    ಭಙ್ಗ;
  ಪಕ್ಷ '[':
    p_bracket(p);
    ಭಙ್ಗ;
  ಪಕ್ಷ BACKSL | '{':
    SETERROR(REG_BADRPT);
    ಭಙ್ಗ;
  ಪಕ್ಷ BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ಯದಿ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ಯದಿ (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ಯದಿ (subno < NPAREN) {
      p->pend[subno] = HERE();
      ಪ್ರತಿಜ್ಞಾ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    ಭಙ್ಗ;
  ಪಕ್ಷ BACKSL | ')': /* should not get here -- must be user */
  ಪಕ್ಷ BACKSL | '}':
    SETERROR(REG_EPAREN);
    ಭಙ್ಗ;
  ಪಕ್ಷ BACKSL | '1':
  ಪಕ್ಷ BACKSL | '2':
  ಪಕ್ಷ BACKSL | '3':
  ಪಕ್ಷ BACKSL | '4':
  ಪಕ್ಷ BACKSL | '5':
  ಪಕ್ಷ BACKSL | '6':
  ಪಕ್ಷ BACKSL | '7':
  ಪಕ್ಷ BACKSL | '8':
  ಪಕ್ಷ BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ಪ್ರತಿಜ್ಞಾ(i < NPAREN);
    ಯದಿ (p->pend[i] != 0) {
      ಪ್ರತಿಜ್ಞಾ(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ಪ್ರತಿಜ್ಞಾ(p->pbegin[i] != 0);
      ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ಪ್ರತಿಜ್ಞಾ(OP(p->strip[p->pend[i]]) == ORPAREN);
      (ಶೂನ್ಯ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } ಅನ್ಯಥಾ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    ಭಙ್ಗ;
  ಪಕ್ಷ '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  ಸ್ವಭಾವ:
    ordinary(p, (ವರ್ಣ)c);
    ಭಙ್ಗ;
  }

  ಯದಿ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ಸೇರಿಸು_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ಸೇರಿಸು_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ಅನ್ಯಥಾ ಯದಿ (EATTWO('\\', '{')) {
    ಎಣಿಕೆ = p_ಎಣಿಕೆ(p);
    ಯದಿ (EAT(',')) {
      ಯದಿ (MORE() && isdigit((uch)PEEK())) {
        count2 = p_ಎಣಿಕೆ(p);
        REQUIRE(ಎಣಿಕೆ <= count2, REG_BADBR);
      } ಅನ್ಯಥಾ /* single number with comma */
        count2 = REGINFINITY;
    } ಅನ್ಯಥಾ /* just a single number */
      count2 = ಎಣಿಕೆ;
    repeat(p, pos, ಎಣಿಕೆ, count2);
    ಯದಿ (!EATTWO('\\', '}')) { /* error heuristics */
      ಯಾವತ್ (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ಅನ್ಯಥಾ ಯದಿ (c == '$') /* $ (but not \$) ends it */
    ಪ್ರತ್ಯಾಗಮ (1);

  ಪ್ರತ್ಯಾಗಮ (0);
}

/*
 - p_count - parse a repetition count
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* the value */
p_ಎಣಿಕೆ(ಸಂರಚನಾ parse *p) {
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಙ್ಕ ndigits = 0;

  ಯಾವತ್ (MORE() && isdigit((uch)PEEK()) && ಎಣಿಕೆ <= DUPMAX) {
    ಎಣಿಕೆ = ಎಣಿಕೆ * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ಎಣಿಕೆ <= DUPMAX, REG_BADBR);
  ಪ್ರತ್ಯಾಗಮ (ಎಣಿಕೆ);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_bracket(ಸಂರಚನಾ parse *p) {
  cset *cs;
  ಪೂರ್ಣಾಙ್ಕ invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ಯದಿ (p->end - p->next > 5) {
    ಯದಿ (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }

  ಯದಿ ((cs = allocset(p)) == ರಿಕ್ತ) {
    /* allocset did set error status in p */
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (EAT('^'))
    invert++; /* make note to invert set at end */
  ಯದಿ (EAT(']'))
    CHadd(cs, ']');
  ಅನ್ಯಥಾ ಯದಿ (EAT('-'))
    CHadd(cs, '-');
  ಯಾವತ್ (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  ಯದಿ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ಯದಿ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (p->g->cflags & REG_ICASE) {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ ci;

    ಪ್ರತಿ (i = p->g->csetsize - 1; i >= 0; i--)
      ಯದಿ (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        ಯದಿ (ci != i)
          CHadd(cs, ci);
      }
    ಯದಿ (cs->multis != ರಿಕ್ತ)
      mccase(p, cs);
  }
  ಯದಿ (invert) {
    ಪೂರ್ಣಾಙ್ಕ i;

    ಪ್ರತಿ (i = p->g->csetsize - 1; i >= 0; i--)
      ಯದಿ (CHIN(cs, i))
        CHsub(cs, i);
      ಅನ್ಯಥಾ
        CHadd(cs, i);
    ಯದಿ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ಯದಿ (cs->multis != ರಿಕ್ತ)
      mcinvert(p, cs);
  }

  ಪ್ರತಿಜ್ಞಾ(cs->multis == ರಿಕ್ತ); /* xxx */

  ಯದಿ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ಅನ್ಯಥಾ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_term(ಸಂರಚನಾ parse *p, cset *cs) {
  ವರ್ಣ c;
  ವರ್ಣ start, finish;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* classify what we've got */
  ವಿಕಲ್ಪ ((MORE()) ? PEEK() : '\0') {
  ಪಕ್ಷ '[':
    c = (MORE2()) ? PEEK2() : '\0';
    ಭಙ್ಗ;
  ಪಕ್ಷ '-':
    SETERROR(REG_ERANGE);
    ಪ್ರತ್ಯಾಗಮ; /* NOTE RETURN */
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    c = '\0';
    ಭಙ್ಗ;
  }

  ವಿಕಲ್ಪ (c) {
  ಪಕ್ಷ ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    ಭಙ್ಗ;
  ಪಕ್ಷ '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    ಭಙ್ಗ;
  ಸ್ವಭಾವ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_ಚಿಹ್ನೆ(p);
    ಯದಿ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      ಯದಿ (EAT('-'))
        finish = '-';
      ಅನ್ಯಥಾ
        finish = p_b_ಚಿಹ್ನೆ(p);
    } ಅನ್ಯಥಾ {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    ಪ್ರತಿ (i = start; i <= finish; i++)
      CHadd(cs, i);
    ಭಙ್ಗ;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_cclass(ಸಂರಚನಾ parse *p, cset *cs) {
  ಧ್ರುವ ವರ್ಣ *sp = p->next;
  ಸಂರಚನಾ cclass *cp;
  size_t len;
  ಧ್ರುವ ವರ್ಣ *u;
  ವರ್ಣ c;

  ಯಾವತ್ (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  ಪ್ರತಿ (cp = cclasses; cp->name != ರಿಕ್ತ; cp++)
    ಯದಿ (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      ಭಙ್ಗ;
  ಯದಿ (cp->name == ರಿಕ್ತ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ಪ್ರತ್ಯಾಗಮ;
  }

  u = cp->chars;
  ಯಾವತ್ ((c = *u++) != '\0')
    CHadd(cs, c);
  ಪ್ರತಿ (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ಸ್ಥಿರ ಶೂನ್ಯ p_b_eclass(ಸಂರಚನಾ parse *p, cset *cs) {
  ವರ್ಣ c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ಸ್ಥಿರ ವರ್ಣ /* value of symbol */
p_b_ಚಿಹ್ನೆ(ಸಂರಚನಾ parse *p) {
  ವರ್ಣ ಮೌಲ್ಯ_2;

  REQUIRE(MORE(), REG_EBRACK);
  ಯದಿ (!EATTWO('[', '.'))
    ಪ್ರತ್ಯಾಗಮ (GETNEXT());

  /* collating symbol */
  ಮೌಲ್ಯ_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ಪ್ರತ್ಯಾಗಮ (ಮೌಲ್ಯ_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ಸ್ಥಿರ ವರ್ಣ                              /* value of collating element */
p_b_coll_elem(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ endc) /* name ended by endc,']' */
{
  ಧ್ರುವ ವರ್ಣ *sp = p->next;
  ಸಂರಚನಾ cname *cp;
  size_t len;

  ಯಾವತ್ (MORE() && !SEETWO(endc, ']'))
    NEXT();
  ಯದಿ (!MORE()) {
    SETERROR(REG_EBRACK);
    ಪ್ರತ್ಯಾಗಮ (0);
  }
  len = p->next - sp;
  ಪ್ರತಿ (cp = cnames; cp->name != ರಿಕ್ತ; cp++)
    ಯದಿ (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      ಪ್ರತ್ಯಾಗಮ (cp->code); /* known name */
  ಯದಿ (len == 1)
    ಪ್ರತ್ಯಾಗಮ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ಪ್ರತ್ಯಾಗಮ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ಸ್ಥಿರ ವರ್ಣ /* if no counterpart, return ch */
othercase(ಪೂರ್ಣಾಙ್ಕ ch) {
  ch = (uch)ch;
  ಪ್ರತಿಜ್ಞಾ(isalpha(ch));
  ಯದಿ (isupper(ch))
    ಪ್ರತ್ಯಾಗಮ ((uch)tolower(ch));
  ಅನ್ಯಥಾ ಯದಿ (islower(ch))
    ಪ್ರತ್ಯಾಗಮ ((uch)toupper(ch));
  ಅನ್ಯಥಾ /* peculiar, but could happen */
    ಪ್ರತ್ಯಾಗಮ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ಸ್ಥಿರ ಶೂನ್ಯ bothcases(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ ch) {
  ಧ್ರುವ ವರ್ಣ *oldnext = p->next;
  ಧ್ರುವ ವರ್ಣ *oldend = p->end;
  ವರ್ಣ bracket[3];

  ch = (uch)ch;
  ಪ್ರತಿಜ್ಞಾ(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ಪ್ರತಿಜ್ಞಾ(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ಸ್ಥಿರ ಶೂನ್ಯ uncased(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ಯದಿ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ಸ್ಥಿರ ಶೂನ್ಯ ordinary(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ ch) {
  ಯದಿ ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ಅನ್ಯಥಾ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ಸ್ಥಿರ ಶೂನ್ಯ nonnewline(ಸಂರಚನಾ parse *p) {
  ಧ್ರುವ ವರ್ಣ *oldnext = p->next;
  ಧ್ರುವ ವರ್ಣ *oldend = p->end;
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  ಪ್ರತಿಜ್ಞಾ(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ಸ್ಥಿರ ಶೂನ್ಯ repeat(ಸಂರಚನಾ parse *p,
                   sopno start, /* operand from here to end of strip */
                   ಪೂರ್ಣಾಙ್ಕ from,    /* repeated from this number */
                   ಪೂರ್ಣಾಙ್ಕ to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ಯದಿ (p->error != 0) /* head off possible runaway recursion */
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿಜ್ಞಾ(from <= to);

  ವಿಕಲ್ಪ (REP(MAP(from), MAP(to))) {
  ಪಕ್ಷ REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(0, 1):   /* as x{1,1}? */
  ಪಕ್ಷ REP(0, N):   /* as x{1,n}? */
  ಪಕ್ಷ REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(1, 1): /* trivial case */
    /* done */
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ಸೇರಿಸು_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    ಪ್ರತಿಜ್ಞಾ(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(1, INF): /* as x+ */
    ಸೇರಿಸು_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    ಭಙ್ಗ;
  ಪಕ್ಷ REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    ಭಙ್ಗ;
  }
}

/*
 - seterr - set an error condition
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* useless but makes type checking happy */
seterr(ಸಂರಚನಾ parse *p, ಪೂರ್ಣಾಙ್ಕ e) {
  ಯದಿ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  ಪ್ರತ್ಯಾಗಮ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ಸ್ಥಿರ cset *allocset(ಸಂರಚನಾ parse *p) {
  ಪೂರ್ಣಾಙ್ಕ no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (no >= p->ncsalloc) { /* need another column of space */
    ಶೂನ್ಯ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ಯದಿ (nc > SIZE_MAX / ಪರಿಮಾಣ(cset))
      ಗಚ್ಛ nomem;
    ಪ್ರತಿಜ್ಞಾ(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((ವರ್ಣ *)p->g->sets, nc * ಪರಿಮಾಣ(cset));
    ಯದಿ (ptr == ರಿಕ್ತ)
      ಗಚ್ಛ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((ವರ್ಣ *)p->g->setbits, nbytes);
    ಯದಿ (ptr == ರಿಕ್ತ)
      ಗಚ್ಛ nomem;
    p->g->setbits = ptr;

    ಪ್ರತಿ (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (ಶೂನ್ಯ)memset((ವರ್ಣ *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ಯದಿ (p->g->sets == ರಿಕ್ತ || p->g->setbits == ರಿಕ್ತ)
    ಗಚ್ಛ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ರಿಕ್ತ;

  ಪ್ರತ್ಯಾಗಮ (cs);
nomem:
  ಮುಕ್ತ(p->g->sets);
  p->g->sets = ರಿಕ್ತ;
  ಮುಕ್ತ(p->g->setbits);
  p->g->setbits = ರಿಕ್ತ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ಪ್ರತ್ಯಾಗಮ (ರಿಕ್ತ);
}

/*
 - freeset - free a now-unused set
 */
ಸ್ಥಿರ ಶೂನ್ಯ freeset(ಸಂರಚನಾ parse *p, cset *cs) {
  size_t i;
  cset *ತುದಿ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  ಪ್ರತಿ (i = 0; i < css; i++)
    CHsub(cs, i);
  ಯದಿ (cs == ತುದಿ_2 - 1) /* recover only the easy case */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* set number */
freezeset(ಸಂರಚನಾ parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *ತುದಿ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  ಪ್ರತಿ (cs2 = &p->g->sets[0]; cs2 < ತುದಿ_2; cs2++)
    ಯದಿ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      ಪ್ರತಿ (i = 0; i < css; i++)
        ಯದಿ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          ಭಙ್ಗ; /* no */
      ಯದಿ (i == css)
        ಭಙ್ಗ; /* yes */
    }

  ಯದಿ (cs2 < ತುದಿ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ಪ್ರತ್ಯಾಗಮ ((ಪೂರ್ಣಾಙ್ಕ)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* character; there is no "none" value */
firstch(ಸಂರಚನಾ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  ಪ್ರತಿ (i = 0; i < css; i++)
    ಯದಿ (CHIN(cs, i))
      ಪ್ರತ್ಯಾಗಮ ((ವರ್ಣ)i);
  ಪ್ರತಿಜ್ಞಾ(never);
  ಪ್ರತ್ಯಾಗಮ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nch(ಸಂರಚನಾ parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  ಪೂರ್ಣಾಙ್ಕ n = 0;

  ಪ್ರತಿ (i = 0; i < css; i++)
    ಯದಿ (CHIN(cs, i))
      n++;
  ಪ್ರತ್ಯಾಗಮ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ಸ್ಥಿರ ಶೂನ್ಯ mcadd(ಸಂರಚನಾ parse *p, cset *cs, ಧ್ರುವ ವರ್ಣ *cp) {
  size_t oldend = cs->smultis;
  ಶೂನ್ಯ *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ಯದಿ (np == ರಿಕ್ತ) {
    ಯದಿ (cs->multis)
      ಮುಕ್ತ(cs->multis);
    cs->multis = ರಿಕ್ತ;
    SETERROR(REG_ESPACE);
    ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಶೂನ್ಯ mcinvert(ಸಂರಚನಾ parse *p, cset *cs) {
  ಪ್ರತಿಜ್ಞಾ(cs->multis == ರಿಕ್ತ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ಸ್ಥಿರ ಶೂನ್ಯ mccase(ಸಂರಚನಾ parse *p, cset *cs) {
  ಪ್ರತಿಜ್ಞಾ(cs->multis == ರಿಕ್ತ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* predicate */
isinsets(ಸಂರಚನಾ re_guts *g, ಪೂರ್ಣಾಙ್ಕ c) {
  uch *col;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  ಪ್ರತಿ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ಯದಿ (col[uc] != 0)
      ಪ್ರತ್ಯಾಗಮ (1);
  ಪ್ರತ್ಯಾಗಮ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ /* predicate */
samesets(ಸಂರಚನಾ re_guts *g, ಪೂರ್ಣಾಙ್ಕ c1, ಪೂರ್ಣಾಙ್ಕ c2) {
  uch *col;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  ಪ್ರತಿ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ಯದಿ (col[uc1] != col[uc2])
      ಪ್ರತ್ಯಾಗಮ (0);
  ಪ್ರತ್ಯಾಗಮ (1);
}

/*
 - categorize - sort out character categories
 */
ಸ್ಥಿರ ಶೂನ್ಯ categorize(ಸಂರಚನಾ parse *p, ಸಂರಚನಾ re_guts *g) {
  cat_t *cats = g->categories;
  ಪೂರ್ಣಾಙ್ಕ c;
  ಪೂರ್ಣಾಙ್ಕ c2;
  cat_t cat;

  /* avoid making error situations worse */
  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿ (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ಯದಿ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      ಪ್ರತಿ (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ಯದಿ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ಸ್ಥಿರ sopno                       /* start of duplicate */
dupl(ಸಂರಚನಾ parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  ಪ್ರತಿಜ್ಞಾ(finish >= start);
  ಯದಿ (len == 0)
    ಪ್ರತ್ಯಾಗಮ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ಪ್ರತಿಜ್ಞಾ(p->ssize >= p->slen + len);
  (ಶೂನ್ಯ)memmove((ವರ್ಣ *)(p->strip + p->slen), (ವರ್ಣ *)(p->strip + start),
                (size_t)len * ಪರಿಮಾಣ(sop));
  p->slen += len;
  ಪ್ರತ್ಯಾಗಮ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ಸ್ಥಿರ ಶೂನ್ಯ doemit(ಸಂರಚನಾ parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ;

  /* deal with oversize operands ("can't happen", more or less) */
  ಪ್ರತಿಜ್ಞಾ(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ಯದಿ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ಪ್ರತಿಜ್ಞಾ(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ doinsert(ಸಂರಚನಾ parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* avoid making error situations worse */
  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  ಪ್ರತಿಜ್ಞಾ(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ಪ್ರತಿಜ್ಞಾ(pos > 0);
  ಪ್ರತಿ (i = 1; i < NPAREN; i++) {
    ಯದಿ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ಯದಿ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((ವರ್ಣ *)&p->strip[pos + 1], (ವರ್ಣ *)&p->strip[pos],
          (HERE() - pos - 1) * ಪರಿಮಾಣ(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ಸ್ಥಿರ ಶೂನ್ಯ dofwd(ಸಂರಚನಾ parse *p, sopno pos, sop ಮೌಲ್ಯ_2) {
  /* avoid making error situations worse */
  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | ಮೌಲ್ಯ_2;
}

/*
 - enlarge - enlarge the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ enlarge(ಸಂರಚನಾ parse *p, sopno size) {
  sop *sp;

  ಯದಿ (p->ssize >= size)
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ ((uintptr_t)size > SIZE_MAX / ಪರಿಮಾಣ(sop)) {
    SETERROR(REG_ESPACE);
    ಪ್ರತ್ಯಾಗಮ;
  }

  sp = (sop *)realloc(p->strip, size * ಪರಿಮಾಣ(sop));
  ಯದಿ (sp == ರಿಕ್ತ) {
    SETERROR(REG_ESPACE);
    ಪ್ರತ್ಯಾಗಮ;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
ಸ್ಥಿರ ಶೂನ್ಯ stripsnug(ಸಂರಚನಾ parse *p, ಸಂರಚನಾ re_guts *g) {
  g->nstates = p->slen;
  ಯದಿ ((uintptr_t)p->slen > SIZE_MAX / ಪರಿಮಾಣ(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ಪ್ರತ್ಯಾಗಮ;
  }

  g->strip = (sop *)realloc((ವರ್ಣ *)p->strip, p->slen * ಪರಿಮಾಣ(sop));
  ಯದಿ (g->strip == ರಿಕ್ತ) {
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
ಸ್ಥಿರ ಶೂನ್ಯ findmust(ಸಂರಚನಾ parse *p, ಸಂರಚನಾ re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  ವರ್ಣ *cp;
  sopno i;

  /* avoid making error situations worse */
  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  ಕುರು {
    s = *scan++;
    ವಿಕಲ್ಪ (OP(s)) {
    ಪಕ್ಷ OCHAR:        /* sequence member */
      ಯದಿ (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      ಭಙ್ಗ;
    ಪಕ್ಷ OPLUS_: /* things that don't break one */
    ಪಕ್ಷ OLPAREN:
    ಪಕ್ಷ ORPAREN:
      ಭಙ್ಗ;
    ಪಕ್ಷ OQUEST_: /* things that must be skipped */
    ಪಕ್ಷ OCH_:
      scan--;
      ಕುರು {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        ಯದಿ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ಪ್ರತ್ಯಾಗಮ;
        }
      } ಯಾವತ್ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    ಸ್ವಭಾವ:                  /* things that break a sequence */
      ಯದಿ (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      ಭಙ್ಗ;
    }
  } ಯಾವತ್ (OP(s) != OEND);

  ಯದಿ (g->mlen == 0) /* there isn't one */
    ಪ್ರತ್ಯಾಗಮ;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  ಯದಿ (g->must == ರಿಕ್ತ) { /* argh; just forget it */
    g->mlen = 0;
    ಪ್ರತ್ಯಾಗಮ;
  }
  cp = g->must;
  scan = start;
  ಪ್ರತಿ (i = g->mlen; i > 0; i--) {
    ಯಾವತ್ (OP(s = *scan++) != OCHAR)
      ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ(cp < g->must + g->mlen);
    *cp++ = (ವರ್ಣ)OPND(s);
  }
  ಪ್ರತಿಜ್ಞಾ(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ಸ್ಥಿರ sopno /* nesting depth */
pluscount(ಸಂರಚನಾ parse *p, ಸಂರಚನಾ re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ಯದಿ (p->error != 0)
    ಪ್ರತ್ಯಾಗಮ (0); /* there may not be an OEND */

  scan = g->strip + 1;
  ಕುರು {
    s = *scan++;
    ವಿಕಲ್ಪ (OP(s)) {
    ಪಕ್ಷ OPLUS_:
      plusnest++;
      ಭಙ್ಗ;
    ಪಕ್ಷ O_PLUS:
      ಯದಿ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      ಭಙ್ಗ;
    }
  } ಯಾವತ್ (OP(s) != OEND);
  ಯದಿ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ಪ್ರತ್ಯಾಗಮ (maxnest);
}
