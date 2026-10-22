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

#അന്തര്ഭാവ "regex_impl.h"
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "regex2.h"
#അന്തര്ഭാവ "regutils.h"

#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"

/* character-class table */
സ്ഥിതം ഘടന cclass {
  സ്ഥിരം അക്ഷരം *name;
  സ്ഥിരം അക്ഷരം *chars;
  സ്ഥിരം അക്ഷരം *multis;
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
    {ശൂന്യ, 0, ""}};

/* character-name table */
സ്ഥിതം ഘടന cname {
  സ്ഥിരം അക്ഷരം *name;
  അക്ഷരം code;
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
              {ശൂന്യ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ഘടന parse {
  സ്ഥിരം അക്ഷരം *next; /* next character in RE */
  സ്ഥിരം അക്ഷരം *end;  /* end of string (-> NUL normally) */
  പൂർണ്ണസംഖ്യ error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  പൂർണ്ണസംഖ്യ ncsalloc;     /* number of csets allocated */
  ഘടന re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

സ്ഥിതം ശൂന്യം p_ere(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം p_ere_exp(ഘടന parse *);
സ്ഥിതം ശൂന്യം p_str(ഘടന parse *);
സ്ഥിതം ശൂന്യം p_bre(ഘടന parse *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ p_simp_re(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ p_എണ്ണം(ഘടന parse *);
സ്ഥിതം ശൂന്യം p_bracket(ഘടന parse *);
സ്ഥിതം ശൂന്യം p_b_term(ഘടന parse *, cset *);
സ്ഥിതം ശൂന്യം p_b_cclass(ഘടന parse *, cset *);
സ്ഥിതം ശൂന്യം p_b_eclass(ഘടന parse *, cset *);
സ്ഥിതം അക്ഷരം p_b_ചിഹ്നം(ഘടന parse *);
സ്ഥിതം അക്ഷരം p_b_coll_elem(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം അക്ഷരം othercase(പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം bothcases(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം uncased(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം ordinary(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം nonnewline(ഘടന parse *);
സ്ഥിതം ശൂന്യം repeat(ഘടന parse *, sopno, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ seterr(ഘടന parse *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം cset *allocset(ഘടന parse *);
സ്ഥിതം ശൂന്യം freeset(ഘടന parse *, cset *);
സ്ഥിതം പൂർണ്ണസംഖ്യ freezeset(ഘടന parse *, cset *);
സ്ഥിതം പൂർണ്ണസംഖ്യ firstch(ഘടന parse *, cset *);
സ്ഥിതം പൂർണ്ണസംഖ്യ nch(ഘടന parse *, cset *);
സ്ഥിതം ശൂന്യം mcadd(ഘടന parse *, cset *, സ്ഥിരം അക്ഷരം *);
സ്ഥിതം ശൂന്യം mcinvert(ഘടന parse *, cset *);
സ്ഥിതം ശൂന്യം mccase(ഘടന parse *, cset *);
സ്ഥിതം പൂർണ്ണസംഖ്യ isinsets(ഘടന re_guts *, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ samesets(ഘടന re_guts *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം ശൂന്യം categorize(ഘടന parse *, ഘടന re_guts *);
സ്ഥിതം sopno dupl(ഘടന parse *, sopno, sopno);
സ്ഥിതം ശൂന്യം doemit(ഘടന parse *, sop, size_t);
സ്ഥിതം ശൂന്യം doinsert(ഘടന parse *, sop, size_t, sopno);
സ്ഥിതം ശൂന്യം dofwd(ഘടന parse *, sopno, sop);
സ്ഥിതം ശൂന്യം enlarge(ഘടന parse *, sopno);
സ്ഥിതം ശൂന്യം stripsnug(ഘടന parse *, ഘടന re_guts *);
സ്ഥിതം ശൂന്യം findmust(ഘടന parse *, ഘടന re_guts *);
സ്ഥിതം sopno pluscount(ഘടന parse *, ഘടന re_guts *);

സ്ഥിതം അക്ഷരം nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (ശൂന്യം)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define തിരുകുക_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#അല്ലെങ്കിൽ
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
സ്ഥിതം പൂർണ്ണസംഖ്യ never = 0; /* for use in asserts; shuts lint up */
#അല്ലെങ്കിൽ
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
പൂർണ്ണസംഖ്യ /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, സ്ഥിരം അക്ഷരം *pattern, പൂർണ്ണസംഖ്യ cflags) {
  ഘടന parse pa;
  ഘടന re_guts *g;
  ഘടന parse *p = &pa;
  പൂർണ്ണസംഖ്യ i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#അല്ലെങ്കിൽ
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  എങ്കിൽ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    തിരികെ (REG_INVARG);

  എങ്കിൽ (cflags & REG_PEND) {
    എങ്കിൽ (preg->re_endp < pattern)
      തിരികെ (REG_INVARG);
    len = preg->re_endp - pattern;
  } അല്ലെങ്കിൽ {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ഘടന re_guts *)malloc(വലുപ്പം(ഘടന re_guts) +
                               (NC - 1) * വലുപ്പം(cat_t));
  എങ്കിൽ (g == ശൂന്യ)
    തിരികെ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, വലുപ്പം(sop));
  p->slen = 0;
  എങ്കിൽ (p->strip == ശൂന്യ) {
    സ്വതന്ത്രം((അക്ഷരം *)g);
    തിരികെ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  വേണ്ടി (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ശൂന്യ;
  g->setbits = ശൂന്യ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ശൂന്യ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (ശൂന്യം)memset((അക്ഷരം *)g->catspace, 0, NC * വലുപ്പം(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  എങ്കിൽ (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  അല്ലെങ്കിൽ എങ്കിൽ (cflags & REG_NOSPEC)
    p_str(p);
  അല്ലെങ്കിൽ
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
  എങ്കിൽ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  എങ്കിൽ (p->error != 0) /* lose */
    llvm_regfree(preg);
  തിരികെ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
സ്ഥിതം ശൂന്യം p_ere(ഘടന parse *p,
                  പൂർണ്ണസംഖ്യ stop) /* character this ERE should end at */
{
  അക്ഷരം c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  പൂർണ്ണസംഖ്യ first = 1; /* is this the first alternative? */

  വേണ്ടി (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    ആകുമ്പോൾ (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    എങ്കിൽ (!EAT('|'))
      നിർത്തുക; /* NOTE BREAK OUT */

    എങ്കിൽ (first) {
      തിരുകുക_3(OCH_, conc); /* offset is wrong */
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

  എങ്കിൽ (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  പ്രതിജ്ഞാ(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
സ്ഥിതം ശൂന്യം p_ere_exp(ഘടന parse *p) {
  അക്ഷരം c;
  sopno pos;
  പൂർണ്ണസംഖ്യ എണ്ണം;
  പൂർണ്ണസംഖ്യ count2;
  പൂർണ്ണസംഖ്യ backrefnum;
  sopno subno;
  പൂർണ്ണസംഖ്യ wascaret = 0;

  പ്രതിജ്ഞാ(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  തിരഞ്ഞെടുപ്പ് (c) {
  അവസ്ഥ '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    എങ്കിൽ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    എങ്കിൽ (!SEE(')'))
      p_ere(p, ')');
    എങ്കിൽ (subno < NPAREN) {
      p->pend[subno] = HERE();
      പ്രതിജ്ഞാ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    നിർത്തുക;
#ifndef POSIX_MISTAKE
  അവസ്ഥ ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    നിർത്തുക;
#endif
  അവസ്ഥ '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    നിർത്തുക;
  അവസ്ഥ '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    നിർത്തുക;
  അവസ്ഥ '|':
    SETERROR(REG_EMPTY);
    നിർത്തുക;
  അവസ്ഥ '*':
  അവസ്ഥ '+':
  അവസ്ഥ '?':
    SETERROR(REG_BADRPT);
    നിർത്തുക;
  അവസ്ഥ '.':
    എങ്കിൽ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    അല്ലെങ്കിൽ
      EMIT(OANY, 0);
    നിർത്തുക;
  അവസ്ഥ '[':
    p_bracket(p);
    നിർത്തുക;
  അവസ്ഥ '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    എങ്കിൽ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } അല്ലെങ്കിൽ എങ്കിൽ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ആകുമ്പോൾ (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } അല്ലെങ്കിൽ എങ്കിൽ (c == 'n') {
      ordinary(p, '\n');
      നിർത്തുക;
    } അല്ലെങ്കിൽ എങ്കിൽ (c == 't') {
      ordinary(p, '\t');
      നിർത്തുക;
    } അല്ലെങ്കിൽ എങ്കിൽ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      അക്ഷരം hexstr[3] = {0};
      അക്ഷരം *hexp;
      പൂർണ്ണസംഖ്യ val;
      എങ്കിൽ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        എങ്കിൽ (*hexp == '\0') {
          NEXT2();
          uncased(p, (അക്ഷരം)val);
          നിർത്തുക;
        }
      }
      ordinary(p, 'x');
      നിർത്തുക;
    } അല്ലെങ്കിൽ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      നിർത്തുക;
    }

    എങ്കിൽ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      നിർത്തുക;
    }

    എങ്കിൽ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      നിർത്തുക;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    പ്രതിജ്ഞാ(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    പ്രതിജ്ഞാ(p->pbegin[backrefnum] != 0);
    പ്രതിജ്ഞാ(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    പ്രതിജ്ഞാ(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (ശൂന്യം)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    നിർത്തുക;
  അവസ്ഥ '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  സ്ഥിരസ്ഥിതി:
    ordinary(p, c);
    നിർത്തുക;
  }

  എങ്കിൽ (!MORE())
    തിരികെ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  എങ്കിൽ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    തിരികെ; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  തിരഞ്ഞെടുപ്പ് (c) {
  അവസ്ഥ '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    തിരുകുക_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    തിരുകുക_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    നിർത്തുക;
  അവസ്ഥ '+':
    തിരുകുക_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    നിർത്തുക;
  അവസ്ഥ '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    തിരുകുക_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    നിർത്തുക;
  അവസ്ഥ '{':
    എണ്ണം = p_എണ്ണം(p);
    എങ്കിൽ (EAT(',')) {
      എങ്കിൽ (isdigit((uch)PEEK())) {
        count2 = p_എണ്ണം(p);
        REQUIRE(എണ്ണം <= count2, REG_BADBR);
      } അല്ലെങ്കിൽ /* single number with comma */
        count2 = REGINFINITY;
    } അല്ലെങ്കിൽ /* just a single number */
      count2 = എണ്ണം;
    repeat(p, pos, എണ്ണം, count2);
    എങ്കിൽ (!EAT('}')) { /* error heuristics */
      ആകുമ്പോൾ (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    നിർത്തുക;
  }

  എങ്കിൽ (!MORE())
    തിരികെ;
  c = PEEK();
  എങ്കിൽ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    തിരികെ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
സ്ഥിതം ശൂന്യം p_str(ഘടന parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  ആകുമ്പോൾ (MORE())
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
സ്ഥിതം ശൂന്യം p_bre(ഘടന parse *p, പൂർണ്ണസംഖ്യ end1, /* first terminating character */
                  പൂർണ്ണസംഖ്യ end2)                  /* second terminating character */
{
  sopno start = HERE();
  പൂർണ്ണസംഖ്യ first = 1; /* first subexpression? */
  പൂർണ്ണസംഖ്യ wasdollar = 0;

  എങ്കിൽ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ആകുമ്പോൾ (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  എങ്കിൽ (wasdollar) { /* oops, that was a trailing anchor */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ /* was the simple RE an unbackslashed $? */
p_simp_re(ഘടന parse *p,
          പൂർണ്ണസംഖ്യ starordinary) /* is a leading * an ordinary character? */
{
  പൂർണ്ണസംഖ്യ c;
  പൂർണ്ണസംഖ്യ എണ്ണം;
  പൂർണ്ണസംഖ്യ count2;
  sopno pos;
  പൂർണ്ണസംഖ്യ i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  പ്രതിജ്ഞാ(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  എങ്കിൽ (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  തിരഞ്ഞെടുപ്പ് (c) {
  അവസ്ഥ '.':
    എങ്കിൽ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    അല്ലെങ്കിൽ
      EMIT(OANY, 0);
    നിർത്തുക;
  അവസ്ഥ '[':
    p_bracket(p);
    നിർത്തുക;
  അവസ്ഥ BACKSL | '{':
    SETERROR(REG_BADRPT);
    നിർത്തുക;
  അവസ്ഥ BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    എങ്കിൽ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    എങ്കിൽ (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    എങ്കിൽ (subno < NPAREN) {
      p->pend[subno] = HERE();
      പ്രതിജ്ഞാ(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    നിർത്തുക;
  അവസ്ഥ BACKSL | ')': /* should not get here -- must be user */
  അവസ്ഥ BACKSL | '}':
    SETERROR(REG_EPAREN);
    നിർത്തുക;
  അവസ്ഥ BACKSL | '1':
  അവസ്ഥ BACKSL | '2':
  അവസ്ഥ BACKSL | '3':
  അവസ്ഥ BACKSL | '4':
  അവസ്ഥ BACKSL | '5':
  അവസ്ഥ BACKSL | '6':
  അവസ്ഥ BACKSL | '7':
  അവസ്ഥ BACKSL | '8':
  അവസ്ഥ BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    പ്രതിജ്ഞാ(i < NPAREN);
    എങ്കിൽ (p->pend[i] != 0) {
      പ്രതിജ്ഞാ(i <= p->g->nsub);
      EMIT(OBACK_, i);
      പ്രതിജ്ഞാ(p->pbegin[i] != 0);
      പ്രതിജ്ഞാ(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      പ്രതിജ്ഞാ(OP(p->strip[p->pend[i]]) == ORPAREN);
      (ശൂന്യം)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } അല്ലെങ്കിൽ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    നിർത്തുക;
  അവസ്ഥ '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  സ്ഥിരസ്ഥിതി:
    ordinary(p, (അക്ഷരം)c);
    നിർത്തുക;
  }

  എങ്കിൽ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    തിരുകുക_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    തിരുകുക_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } അല്ലെങ്കിൽ എങ്കിൽ (EATTWO('\\', '{')) {
    എണ്ണം = p_എണ്ണം(p);
    എങ്കിൽ (EAT(',')) {
      എങ്കിൽ (MORE() && isdigit((uch)PEEK())) {
        count2 = p_എണ്ണം(p);
        REQUIRE(എണ്ണം <= count2, REG_BADBR);
      } അല്ലെങ്കിൽ /* single number with comma */
        count2 = REGINFINITY;
    } അല്ലെങ്കിൽ /* just a single number */
      count2 = എണ്ണം;
    repeat(p, pos, എണ്ണം, count2);
    എങ്കിൽ (!EATTWO('\\', '}')) { /* error heuristics */
      ആകുമ്പോൾ (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (c == '$') /* $ (but not \$) ends it */
    തിരികെ (1);

  തിരികെ (0);
}

/*
 - p_count - parse a repetition count
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ /* the value */
p_എണ്ണം(ഘടന parse *p) {
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ ndigits = 0;

  ആകുമ്പോൾ (MORE() && isdigit((uch)PEEK()) && എണ്ണം <= DUPMAX) {
    എണ്ണം = എണ്ണം * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && എണ്ണം <= DUPMAX, REG_BADBR);
  തിരികെ (എണ്ണം);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
സ്ഥിതം ശൂന്യം p_bracket(ഘടന parse *p) {
  cset *cs;
  പൂർണ്ണസംഖ്യ invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  എങ്കിൽ (p->end - p->next > 5) {
    എങ്കിൽ (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      തിരികെ;
    }
    എങ്കിൽ (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      തിരികെ;
    }
  }

  എങ്കിൽ ((cs = allocset(p)) == ശൂന്യ) {
    /* allocset did set error status in p */
    തിരികെ;
  }

  എങ്കിൽ (EAT('^'))
    invert++; /* make note to invert set at end */
  എങ്കിൽ (EAT(']'))
    CHadd(cs, ']');
  അല്ലെങ്കിൽ എങ്കിൽ (EAT('-'))
    CHadd(cs, '-');
  ആകുമ്പോൾ (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  എങ്കിൽ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  എങ്കിൽ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    തിരികെ;
  }

  എങ്കിൽ (p->g->cflags & REG_ICASE) {
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ ci;

    വേണ്ടി (i = p->g->csetsize - 1; i >= 0; i--)
      എങ്കിൽ (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        എങ്കിൽ (ci != i)
          CHadd(cs, ci);
      }
    എങ്കിൽ (cs->multis != ശൂന്യ)
      mccase(p, cs);
  }
  എങ്കിൽ (invert) {
    പൂർണ്ണസംഖ്യ i;

    വേണ്ടി (i = p->g->csetsize - 1; i >= 0; i--)
      എങ്കിൽ (CHIN(cs, i))
        CHsub(cs, i);
      അല്ലെങ്കിൽ
        CHadd(cs, i);
    എങ്കിൽ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    എങ്കിൽ (cs->multis != ശൂന്യ)
      mcinvert(p, cs);
  }

  പ്രതിജ്ഞാ(cs->multis == ശൂന്യ); /* xxx */

  എങ്കിൽ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } അല്ലെങ്കിൽ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
സ്ഥിതം ശൂന്യം p_b_term(ഘടന parse *p, cset *cs) {
  അക്ഷരം c;
  അക്ഷരം start, finish;
  പൂർണ്ണസംഖ്യ i;

  /* classify what we've got */
  തിരഞ്ഞെടുപ്പ് ((MORE()) ? PEEK() : '\0') {
  അവസ്ഥ '[':
    c = (MORE2()) ? PEEK2() : '\0';
    നിർത്തുക;
  അവസ്ഥ '-':
    SETERROR(REG_ERANGE);
    തിരികെ; /* NOTE RETURN */
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    c = '\0';
    നിർത്തുക;
  }

  തിരഞ്ഞെടുപ്പ് (c) {
  അവസ്ഥ ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    നിർത്തുക;
  അവസ്ഥ '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_ചിഹ്നം(p);
    എങ്കിൽ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      എങ്കിൽ (EAT('-'))
        finish = '-';
      അല്ലെങ്കിൽ
        finish = p_b_ചിഹ്നം(p);
    } അല്ലെങ്കിൽ {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    വേണ്ടി (i = start; i <= finish; i++)
      CHadd(cs, i);
    നിർത്തുക;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
സ്ഥിതം ശൂന്യം p_b_cclass(ഘടന parse *p, cset *cs) {
  സ്ഥിരം അക്ഷരം *sp = p->next;
  ഘടന cclass *cp;
  size_t len;
  സ്ഥിരം അക്ഷരം *u;
  അക്ഷരം c;

  ആകുമ്പോൾ (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  വേണ്ടി (cp = cclasses; cp->name != ശൂന്യ; cp++)
    എങ്കിൽ (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      നിർത്തുക;
  എങ്കിൽ (cp->name == ശൂന്യ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    തിരികെ;
  }

  u = cp->chars;
  ആകുമ്പോൾ ((c = *u++) != '\0')
    CHadd(cs, c);
  വേണ്ടി (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
സ്ഥിതം ശൂന്യം p_b_eclass(ഘടന parse *p, cset *cs) {
  അക്ഷരം c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
സ്ഥിതം അക്ഷരം /* value of symbol */
p_b_ചിഹ്നം(ഘടന parse *p) {
  അക്ഷരം മൂല്യം_2;

  REQUIRE(MORE(), REG_EBRACK);
  എങ്കിൽ (!EATTWO('[', '.'))
    തിരികെ (GETNEXT());

  /* collating symbol */
  മൂല്യം_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  തിരികെ (മൂല്യം_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
സ്ഥിതം അക്ഷരം                              /* value of collating element */
p_b_coll_elem(ഘടന parse *p, പൂർണ്ണസംഖ്യ endc) /* name ended by endc,']' */
{
  സ്ഥിരം അക്ഷരം *sp = p->next;
  ഘടന cname *cp;
  size_t len;

  ആകുമ്പോൾ (MORE() && !SEETWO(endc, ']'))
    NEXT();
  എങ്കിൽ (!MORE()) {
    SETERROR(REG_EBRACK);
    തിരികെ (0);
  }
  len = p->next - sp;
  വേണ്ടി (cp = cnames; cp->name != ശൂന്യ; cp++)
    എങ്കിൽ (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      തിരികെ (cp->code); /* known name */
  എങ്കിൽ (len == 1)
    തിരികെ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  തിരികെ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
സ്ഥിതം അക്ഷരം /* if no counterpart, return ch */
othercase(പൂർണ്ണസംഖ്യ ch) {
  ch = (uch)ch;
  പ്രതിജ്ഞാ(isalpha(ch));
  എങ്കിൽ (isupper(ch))
    തിരികെ ((uch)tolower(ch));
  അല്ലെങ്കിൽ എങ്കിൽ (islower(ch))
    തിരികെ ((uch)toupper(ch));
  അല്ലെങ്കിൽ /* peculiar, but could happen */
    തിരികെ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
സ്ഥിതം ശൂന്യം bothcases(ഘടന parse *p, പൂർണ്ണസംഖ്യ ch) {
  സ്ഥിരം അക്ഷരം *oldnext = p->next;
  സ്ഥിരം അക്ഷരം *oldend = p->end;
  അക്ഷരം bracket[3];

  ch = (uch)ch;
  പ്രതിജ്ഞാ(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  പ്രതിജ്ഞാ(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
സ്ഥിതം ശൂന്യം uncased(ഘടന parse *p, പൂർണ്ണസംഖ്യ ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  എങ്കിൽ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
സ്ഥിതം ശൂന്യം ordinary(ഘടന parse *p, പൂർണ്ണസംഖ്യ ch) {
  എങ്കിൽ ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  അല്ലെങ്കിൽ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
സ്ഥിതം ശൂന്യം nonnewline(ഘടന parse *p) {
  സ്ഥിരം അക്ഷരം *oldnext = p->next;
  സ്ഥിരം അക്ഷരം *oldend = p->end;
  സ്ഥിതം സ്ഥിരം അക്ഷരം bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  പ്രതിജ്ഞാ(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
സ്ഥിതം ശൂന്യം repeat(ഘടന parse *p,
                   sopno start, /* operand from here to end of strip */
                   പൂർണ്ണസംഖ്യ from,    /* repeated from this number */
                   പൂർണ്ണസംഖ്യ to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  എങ്കിൽ (p->error != 0) /* head off possible runaway recursion */
    തിരികെ;

  പ്രതിജ്ഞാ(from <= to);

  തിരഞ്ഞെടുപ്പ് (REP(MAP(from), MAP(to))) {
  അവസ്ഥ REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    നിർത്തുക;
  അവസ്ഥ REP(0, 1):   /* as x{1,1}? */
  അവസ്ഥ REP(0, N):   /* as x{1,n}? */
  അവസ്ഥ REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    തിരുകുക_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    നിർത്തുക;
  അവസ്ഥ REP(1, 1): /* trivial case */
    /* done */
    നിർത്തുക;
  അവസ്ഥ REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    തിരുകുക_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    പ്രതിജ്ഞാ(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    നിർത്തുക;
  അവസ്ഥ REP(1, INF): /* as x+ */
    തിരുകുക_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    നിർത്തുക;
  അവസ്ഥ REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    നിർത്തുക;
  അവസ്ഥ REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    നിർത്തുക;
  }
}

/*
 - seterr - set an error condition
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ /* useless but makes type checking happy */
seterr(ഘടന parse *p, പൂർണ്ണസംഖ്യ e) {
  എങ്കിൽ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  തിരികെ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
സ്ഥിതം cset *allocset(ഘടന parse *p) {
  പൂർണ്ണസംഖ്യ no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (no >= p->ncsalloc) { /* need another column of space */
    ശൂന്യം *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    എങ്കിൽ (nc > SIZE_MAX / വലുപ്പം(cset))
      പോകുക nomem;
    പ്രതിജ്ഞാ(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((അക്ഷരം *)p->g->sets, nc * വലുപ്പം(cset));
    എങ്കിൽ (ptr == ശൂന്യ)
      പോകുക nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((അക്ഷരം *)p->g->setbits, nbytes);
    എങ്കിൽ (ptr == ശൂന്യ)
      പോകുക nomem;
    p->g->setbits = ptr;

    വേണ്ടി (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (ശൂന്യം)memset((അക്ഷരം *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  എങ്കിൽ (p->g->sets == ശൂന്യ || p->g->setbits == ശൂന്യ)
    പോകുക nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ശൂന്യ;

  തിരികെ (cs);
nomem:
  സ്വതന്ത്രം(p->g->sets);
  p->g->sets = ശൂന്യ;
  സ്വതന്ത്രം(p->g->setbits);
  p->g->setbits = ശൂന്യ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  തിരികെ (ശൂന്യ);
}

/*
 - freeset - free a now-unused set
 */
സ്ഥിതം ശൂന്യം freeset(ഘടന parse *p, cset *cs) {
  size_t i;
  cset *മുകൾ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  വേണ്ടി (i = 0; i < css; i++)
    CHsub(cs, i);
  എങ്കിൽ (cs == മുകൾ_2 - 1) /* recover only the easy case */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ /* set number */
freezeset(ഘടന parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *മുകൾ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  വേണ്ടി (cs2 = &p->g->sets[0]; cs2 < മുകൾ_2; cs2++)
    എങ്കിൽ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      വേണ്ടി (i = 0; i < css; i++)
        എങ്കിൽ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          നിർത്തുക; /* no */
      എങ്കിൽ (i == css)
        നിർത്തുക; /* yes */
    }

  എങ്കിൽ (cs2 < മുകൾ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  തിരികെ ((പൂർണ്ണസംഖ്യ)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ /* character; there is no "none" value */
firstch(ഘടന parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  വേണ്ടി (i = 0; i < css; i++)
    എങ്കിൽ (CHIN(cs, i))
      തിരികെ ((അക്ഷരം)i);
  പ്രതിജ്ഞാ(never);
  തിരികെ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ nch(ഘടന parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  പൂർണ്ണസംഖ്യ n = 0;

  വേണ്ടി (i = 0; i < css; i++)
    എങ്കിൽ (CHIN(cs, i))
      n++;
  തിരികെ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
സ്ഥിതം ശൂന്യം mcadd(ഘടന parse *p, cset *cs, സ്ഥിരം അക്ഷരം *cp) {
  size_t oldend = cs->smultis;
  ശൂന്യം *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  എങ്കിൽ (np == ശൂന്യ) {
    എങ്കിൽ (cs->multis)
      സ്വതന്ത്രം(cs->multis);
    cs->multis = ശൂന്യ;
    SETERROR(REG_ESPACE);
    തിരികെ;
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
സ്ഥിതം ശൂന്യം mcinvert(ഘടന parse *p, cset *cs) {
  പ്രതിജ്ഞാ(cs->multis == ശൂന്യ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
സ്ഥിതം ശൂന്യം mccase(ഘടന parse *p, cset *cs) {
  പ്രതിജ്ഞാ(cs->multis == ശൂന്യ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ /* predicate */
isinsets(ഘടന re_guts *g, പൂർണ്ണസംഖ്യ c) {
  uch *col;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  വേണ്ടി (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    എങ്കിൽ (col[uc] != 0)
      തിരികെ (1);
  തിരികെ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ /* predicate */
samesets(ഘടന re_guts *g, പൂർണ്ണസംഖ്യ c1, പൂർണ്ണസംഖ്യ c2) {
  uch *col;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  വേണ്ടി (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    എങ്കിൽ (col[uc1] != col[uc2])
      തിരികെ (0);
  തിരികെ (1);
}

/*
 - categorize - sort out character categories
 */
സ്ഥിതം ശൂന്യം categorize(ഘടന parse *p, ഘടന re_guts *g) {
  cat_t *cats = g->categories;
  പൂർണ്ണസംഖ്യ c;
  പൂർണ്ണസംഖ്യ c2;
  cat_t cat;

  /* avoid making error situations worse */
  എങ്കിൽ (p->error != 0)
    തിരികെ;

  വേണ്ടി (c = CHAR_MIN; c <= CHAR_MAX; c++)
    എങ്കിൽ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      വേണ്ടി (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        എങ്കിൽ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
സ്ഥിതം sopno                       /* start of duplicate */
dupl(ഘടന parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  പ്രതിജ്ഞാ(finish >= start);
  എങ്കിൽ (len == 0)
    തിരികെ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  പ്രതിജ്ഞാ(p->ssize >= p->slen + len);
  (ശൂന്യം)memmove((അക്ഷരം *)(p->strip + p->slen), (അക്ഷരം *)(p->strip + start),
                (size_t)len * വലുപ്പം(sop));
  p->slen += len;
  തിരികെ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
സ്ഥിതം ശൂന്യം doemit(ഘടന parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  എങ്കിൽ (p->error != 0)
    തിരികെ;

  /* deal with oversize operands ("can't happen", more or less) */
  പ്രതിജ്ഞാ(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  എങ്കിൽ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  പ്രതിജ്ഞാ(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
സ്ഥിതം ശൂന്യം doinsert(ഘടന parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  പൂർണ്ണസംഖ്യ i;

  /* avoid making error situations worse */
  എങ്കിൽ (p->error != 0)
    തിരികെ;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  പ്രതിജ്ഞാ(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  പ്രതിജ്ഞാ(pos > 0);
  വേണ്ടി (i = 1; i < NPAREN; i++) {
    എങ്കിൽ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    എങ്കിൽ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((അക്ഷരം *)&p->strip[pos + 1], (അക്ഷരം *)&p->strip[pos],
          (HERE() - pos - 1) * വലുപ്പം(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
സ്ഥിതം ശൂന്യം dofwd(ഘടന parse *p, sopno pos, sop മൂല്യം_2) {
  /* avoid making error situations worse */
  എങ്കിൽ (p->error != 0)
    തിരികെ;

  പ്രതിജ്ഞാ(മൂല്യം_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | മൂല്യം_2;
}

/*
 - enlarge - enlarge the strip
 */
സ്ഥിതം ശൂന്യം enlarge(ഘടന parse *p, sopno size) {
  sop *sp;

  എങ്കിൽ (p->ssize >= size)
    തിരികെ;

  എങ്കിൽ ((uintptr_t)size > SIZE_MAX / വലുപ്പം(sop)) {
    SETERROR(REG_ESPACE);
    തിരികെ;
  }

  sp = (sop *)realloc(p->strip, size * വലുപ്പം(sop));
  എങ്കിൽ (sp == ശൂന്യ) {
    SETERROR(REG_ESPACE);
    തിരികെ;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
സ്ഥിതം ശൂന്യം stripsnug(ഘടന parse *p, ഘടന re_guts *g) {
  g->nstates = p->slen;
  എങ്കിൽ ((uintptr_t)p->slen > SIZE_MAX / വലുപ്പം(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    തിരികെ;
  }

  g->strip = (sop *)realloc((അക്ഷരം *)p->strip, p->slen * വലുപ്പം(sop));
  എങ്കിൽ (g->strip == ശൂന്യ) {
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
സ്ഥിതം ശൂന്യം findmust(ഘടന parse *p, ഘടന re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  അക്ഷരം *cp;
  sopno i;

  /* avoid making error situations worse */
  എങ്കിൽ (p->error != 0)
    തിരികെ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  ചെയ്യുക {
    s = *scan++;
    തിരഞ്ഞെടുപ്പ് (OP(s)) {
    അവസ്ഥ OCHAR:        /* sequence member */
      എങ്കിൽ (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      നിർത്തുക;
    അവസ്ഥ OPLUS_: /* things that don't break one */
    അവസ്ഥ OLPAREN:
    അവസ്ഥ ORPAREN:
      നിർത്തുക;
    അവസ്ഥ OQUEST_: /* things that must be skipped */
    അവസ്ഥ OCH_:
      scan--;
      ചെയ്യുക {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        എങ്കിൽ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          തിരികെ;
        }
      } ആകുമ്പോൾ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    സ്ഥിരസ്ഥിതി:                  /* things that break a sequence */
      എങ്കിൽ (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      നിർത്തുക;
    }
  } ആകുമ്പോൾ (OP(s) != OEND);

  എങ്കിൽ (g->mlen == 0) /* there isn't one */
    തിരികെ;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  എങ്കിൽ (g->must == ശൂന്യ) { /* argh; just forget it */
    g->mlen = 0;
    തിരികെ;
  }
  cp = g->must;
  scan = start;
  വേണ്ടി (i = g->mlen; i > 0; i--) {
    ആകുമ്പോൾ (OP(s = *scan++) != OCHAR)
      തുടരുക;
    പ്രതിജ്ഞാ(cp < g->must + g->mlen);
    *cp++ = (അക്ഷരം)OPND(s);
  }
  പ്രതിജ്ഞാ(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
സ്ഥിതം sopno /* nesting depth */
pluscount(ഘടന parse *p, ഘടന re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  എങ്കിൽ (p->error != 0)
    തിരികെ (0); /* there may not be an OEND */

  scan = g->strip + 1;
  ചെയ്യുക {
    s = *scan++;
    തിരഞ്ഞെടുപ്പ് (OP(s)) {
    അവസ്ഥ OPLUS_:
      plusnest++;
      നിർത്തുക;
    അവസ്ഥ O_PLUS:
      എങ്കിൽ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      നിർത്തുക;
    }
  } ആകുമ്പോൾ (OP(s) != OEND);
  എങ്കിൽ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  തിരികെ (maxnest);
}
