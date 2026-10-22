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

#ଅନ୍ତର୍ଭାଵ "regex_impl.h"
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ "regex2.h"
#ଅନ୍ତର୍ଭାଵ "regutils.h"

#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"

/* character-class table */
ସ୍ଥିର ସଂରଚନା cclass {
  ଅଚଳ ଅକ୍ଷର *name;
  ଅଚଳ ଅକ୍ଷର *chars;
  ଅଚଳ ଅକ୍ଷର *multis;
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
    {ଖାଲି, 0, ""}};

/* character-name table */
ସ୍ଥିର ସଂରଚନା cname {
  ଅଚଳ ଅକ୍ଷର *name;
  ଅକ୍ଷର code;
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
              {ଖାଲି, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ସଂରଚନା parse {
  ଅଚଳ ଅକ୍ଷର *next; /* next character in RE */
  ଅଚଳ ଅକ୍ଷର *end;  /* end of string (-> NUL normally) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncsalloc;     /* number of csets allocated */
  ସଂରଚନା re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ସ୍ଥିର ଶୂନ୍ୟ p_ere(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ p_ere_exp(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ p_str(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ p_bre(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_simp_re(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_ଗଣନା(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ p_bracket(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ p_b_term(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ଶୂନ୍ୟ p_b_cclass(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ଶୂନ୍ୟ p_b_eclass(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ଅକ୍ଷର p_b_ପ୍ରତୀକ(ସଂରଚନା parse *);
ସ୍ଥିର ଅକ୍ଷର p_b_coll_elem(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଅକ୍ଷର othercase(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ bothcases(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ uncased(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ ordinary(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ nonnewline(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ repeat(ସଂରଚନା parse *, sopno, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା seterr(ସଂରଚନା parse *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର cset *allocset(ସଂରଚନା parse *);
ସ୍ଥିର ଶୂନ୍ୟ freeset(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା freezeset(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstch(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nch(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ଶୂନ୍ୟ mcadd(ସଂରଚନା parse *, cset *, ଅଚଳ ଅକ୍ଷର *);
ସ୍ଥିର ଶୂନ୍ୟ mcinvert(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ଶୂନ୍ୟ mccase(ସଂରଚନା parse *, cset *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isinsets(ସଂରଚନା re_guts *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା samesets(ସଂରଚନା re_guts *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ଶୂନ୍ୟ categorize(ସଂରଚନା parse *, ସଂରଚନା re_guts *);
ସ୍ଥିର sopno dupl(ସଂରଚନା parse *, sopno, sopno);
ସ୍ଥିର ଶୂନ୍ୟ doemit(ସଂରଚନା parse *, sop, size_t);
ସ୍ଥିର ଶୂନ୍ୟ doinsert(ସଂରଚନା parse *, sop, size_t, sopno);
ସ୍ଥିର ଶୂନ୍ୟ dofwd(ସଂରଚନା parse *, sopno, sop);
ସ୍ଥିର ଶୂନ୍ୟ enlarge(ସଂରଚନା parse *, sopno);
ସ୍ଥିର ଶୂନ୍ୟ stripsnug(ସଂରଚନା parse *, ସଂରଚନା re_guts *);
ସ୍ଥିର ଶୂନ୍ୟ findmust(ସଂରଚନା parse *, ସଂରଚନା re_guts *);
ସ୍ଥିର sopno pluscount(ସଂରଚନା parse *, ସଂରଚନା re_guts *);

ସ୍ଥିର ଅକ୍ଷର nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (ଶୂନ୍ୟ)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define ଭର୍ତ୍ତି_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#ଅନ୍ୟଥା
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା never = 0; /* for use in asserts; shuts lint up */
#ଅନ୍ୟଥା
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ଅଚଳ ଅକ୍ଷର *pattern, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cflags) {
  ସଂରଚନା parse pa;
  ସଂରଚନା re_guts *g;
  ସଂରଚନା parse *p = &pa;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#ଅନ୍ୟଥା
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  ଯଦି ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ଫେରନ୍ତୁ (REG_INVARG);

  ଯଦି (cflags & REG_PEND) {
    ଯଦି (preg->re_endp < pattern)
      ଫେରନ୍ତୁ (REG_INVARG);
    len = preg->re_endp - pattern;
  } ଅନ୍ୟଥା {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ସଂରଚନା re_guts *)malloc(ଆକାର(ସଂରଚନା re_guts) +
                               (NC - 1) * ଆକାର(cat_t));
  ଯଦି (g == ଖାଲି)
    ଫେରନ୍ତୁ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, ଆକାର(sop));
  p->slen = 0;
  ଯଦି (p->strip == ଖାଲି) {
    ମୁକ୍ତ((ଅକ୍ଷର *)g);
    ଫେରନ୍ତୁ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  ପାଇଁ (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ଖାଲି;
  g->setbits = ଖାଲି;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ଖାଲି;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (ଶୂନ୍ୟ)memset((ଅକ୍ଷର *)g->catspace, 0, NC * ଆକାର(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ଯଦି (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  ଅନ୍ୟଥା ଯଦି (cflags & REG_NOSPEC)
    p_str(p);
  ଅନ୍ୟଥା
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
  ଯଦି (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  ଯଦି (p->error != 0) /* lose */
    llvm_regfree(preg);
  ଫେରନ୍ତୁ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ସ୍ଥିର ଶୂନ୍ୟ p_ere(ସଂରଚନା parse *p,
                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stop) /* character this ERE should end at */
{
  ଅକ୍ଷର c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା first = 1; /* is this the first alternative? */

  ପାଇଁ (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    ଯେତେବେଳେ (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    ଯଦି (!EAT('|'))
      ଭାଙ୍ଗ; /* NOTE BREAK OUT */

    ଯଦି (first) {
      ଭର୍ତ୍ତି_3(OCH_, conc); /* offset is wrong */
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

  ଯଦି (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ପ୍ରତିଜ୍ଞା(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ସ୍ଥିର ଶୂନ୍ୟ p_ere_exp(ସଂରଚନା parse *p) {
  ଅକ୍ଷର c;
  sopno pos;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା count2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା backrefnum;
  sopno subno;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା wascaret = 0;

  ପ୍ରତିଜ୍ଞା(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  ବାଛ (c) {
  ସ୍ଥିତି '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ଯଦି (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    ଯଦି (!SEE(')'))
      p_ere(p, ')');
    ଯଦି (subno < NPAREN) {
      p->pend[subno] = HERE();
      ପ୍ରତିଜ୍ଞା(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    ଭାଙ୍ଗ;
#ifndef POSIX_MISTAKE
  ସ୍ଥିତି ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    ଭାଙ୍ଗ;
#endif
  ସ୍ଥିତି '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '|':
    SETERROR(REG_EMPTY);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '*':
  ସ୍ଥିତି '+':
  ସ୍ଥିତି '?':
    SETERROR(REG_BADRPT);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '.':
    ଯଦି (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ଅନ୍ୟଥା
      EMIT(OANY, 0);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '[':
    p_bracket(p);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    ଯଦି (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ଅନ୍ୟଥା ଯଦି (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ଯେତେବେଳେ (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ଅନ୍ୟଥା ଯଦି (c == 'n') {
      ordinary(p, '\n');
      ଭାଙ୍ଗ;
    } ଅନ୍ୟଥା ଯଦି (c == 't') {
      ordinary(p, '\t');
      ଭାଙ୍ଗ;
    } ଅନ୍ୟଥା ଯଦି (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      ଅକ୍ଷର hexstr[3] = {0};
      ଅକ୍ଷର *hexp;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
      ଯଦି (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ଯଦି (*hexp == '\0') {
          NEXT2();
          uncased(p, (ଅକ୍ଷର)val);
          ଭାଙ୍ଗ;
        }
      }
      ordinary(p, 'x');
      ଭାଙ୍ଗ;
    } ଅନ୍ୟଥା {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      ଭାଙ୍ଗ;
    }

    ଯଦି (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      ଭାଙ୍ଗ;
    }

    ଯଦି (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      ଭାଙ୍ଗ;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    ପ୍ରତିଜ୍ଞା(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    ପ୍ରତିଜ୍ଞା(p->pbegin[backrefnum] != 0);
    ପ୍ରତିଜ୍ଞା(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    ପ୍ରତିଜ୍ଞା(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (ଶୂନ୍ୟ)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ordinary(p, c);
    ଭାଙ୍ଗ;
  }

  ଯଦି (!MORE())
    ଫେରନ୍ତୁ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ଯଦି (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ଫେରନ୍ତୁ; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  ବାଛ (c) {
  ସ୍ଥିତି '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ଭର୍ତ୍ତି_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ଭର୍ତ୍ତି_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '+':
    ଭର୍ତ୍ତି_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ଭର୍ତ୍ତି_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '{':
    ଗଣନା_2 = p_ଗଣନା(p);
    ଯଦି (EAT(',')) {
      ଯଦି (isdigit((uch)PEEK())) {
        count2 = p_ଗଣନା(p);
        REQUIRE(ଗଣନା_2 <= count2, REG_BADBR);
      } ଅନ୍ୟଥା /* single number with comma */
        count2 = REGINFINITY;
    } ଅନ୍ୟଥା /* just a single number */
      count2 = ଗଣନା_2;
    repeat(p, pos, ଗଣନା_2, count2);
    ଯଦି (!EAT('}')) { /* error heuristics */
      ଯେତେବେଳେ (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    ଭାଙ୍ଗ;
  }

  ଯଦି (!MORE())
    ଫେରନ୍ତୁ;
  c = PEEK();
  ଯଦି (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ଫେରନ୍ତୁ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ସ୍ଥିର ଶୂନ୍ୟ p_str(ସଂରଚନା parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  ଯେତେବେଳେ (MORE())
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
ସ୍ଥିର ଶୂନ୍ୟ p_bre(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା end1, /* first terminating character */
                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା end2)                  /* second terminating character */
{
  sopno start = HERE();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା first = 1; /* first subexpression? */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା wasdollar = 0;

  ଯଦି (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ଯେତେବେଳେ (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  ଯଦି (wasdollar) { /* oops, that was a trailing anchor */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* was the simple RE an unbackslashed $? */
p_simp_re(ସଂରଚନା parse *p,
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା starordinary) /* is a leading * an ordinary character? */
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା count2;
  sopno pos;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  ପ୍ରତିଜ୍ଞା(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  ଯଦି (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  ବାଛ (c) {
  ସ୍ଥିତି '.':
    ଯଦି (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ଅନ୍ୟଥା
      EMIT(OANY, 0);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '[':
    p_bracket(p);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି BACKSL | '{':
    SETERROR(REG_BADRPT);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ଯଦି (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ଯଦି (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ଯଦି (subno < NPAREN) {
      p->pend[subno] = HERE();
      ପ୍ରତିଜ୍ଞା(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି BACKSL | ')': /* should not get here -- must be user */
  ସ୍ଥିତି BACKSL | '}':
    SETERROR(REG_EPAREN);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି BACKSL | '1':
  ସ୍ଥିତି BACKSL | '2':
  ସ୍ଥିତି BACKSL | '3':
  ସ୍ଥିତି BACKSL | '4':
  ସ୍ଥିତି BACKSL | '5':
  ସ୍ଥିତି BACKSL | '6':
  ସ୍ଥିତି BACKSL | '7':
  ସ୍ଥିତି BACKSL | '8':
  ସ୍ଥିତି BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ପ୍ରତିଜ୍ଞା(i < NPAREN);
    ଯଦି (p->pend[i] != 0) {
      ପ୍ରତିଜ୍ଞା(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ପ୍ରତିଜ୍ଞା(p->pbegin[i] != 0);
      ପ୍ରତିଜ୍ଞା(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ପ୍ରତିଜ୍ଞା(OP(p->strip[p->pend[i]]) == ORPAREN);
      (ଶୂନ୍ୟ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } ଅନ୍ୟଥା {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ordinary(p, (ଅକ୍ଷର)c);
    ଭାଙ୍ଗ;
  }

  ଯଦି (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ଭର୍ତ୍ତି_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ଭର୍ତ୍ତି_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ଅନ୍ୟଥା ଯଦି (EATTWO('\\', '{')) {
    ଗଣନା_2 = p_ଗଣନା(p);
    ଯଦି (EAT(',')) {
      ଯଦି (MORE() && isdigit((uch)PEEK())) {
        count2 = p_ଗଣନା(p);
        REQUIRE(ଗଣନା_2 <= count2, REG_BADBR);
      } ଅନ୍ୟଥା /* single number with comma */
        count2 = REGINFINITY;
    } ଅନ୍ୟଥା /* just a single number */
      count2 = ଗଣନା_2;
    repeat(p, pos, ଗଣନା_2, count2);
    ଯଦି (!EATTWO('\\', '}')) { /* error heuristics */
      ଯେତେବେଳେ (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ଅନ୍ୟଥା ଯଦି (c == '$') /* $ (but not \$) ends it */
    ଫେରନ୍ତୁ (1);

  ଫେରନ୍ତୁ (0);
}

/*
 - p_count - parse a repetition count
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* the value */
p_ଗଣନା(ସଂରଚନା parse *p) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ndigits = 0;

  ଯେତେବେଳେ (MORE() && isdigit((uch)PEEK()) && ଗଣନା_2 <= DUPMAX) {
    ଗଣନା_2 = ଗଣନା_2 * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ଗଣନା_2 <= DUPMAX, REG_BADBR);
  ଫେରନ୍ତୁ (ଗଣନା_2);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ସ୍ଥିର ଶୂନ୍ୟ p_bracket(ସଂରଚନା parse *p) {
  cset *cs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ଯଦି (p->end - p->next > 5) {
    ଯଦି (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ଫେରନ୍ତୁ;
    }
    ଯଦି (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ଫେରନ୍ତୁ;
    }
  }

  ଯଦି ((cs = allocset(p)) == ଖାଲି) {
    /* allocset did set error status in p */
    ଫେରନ୍ତୁ;
  }

  ଯଦି (EAT('^'))
    invert++; /* make note to invert set at end */
  ଯଦି (EAT(']'))
    CHadd(cs, ']');
  ଅନ୍ୟଥା ଯଦି (EAT('-'))
    CHadd(cs, '-');
  ଯେତେବେଳେ (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  ଯଦି (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ଯଦି (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ଫେରନ୍ତୁ;
  }

  ଯଦି (p->g->cflags & REG_ICASE) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ci;

    ପାଇଁ (i = p->g->csetsize - 1; i >= 0; i--)
      ଯଦି (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        ଯଦି (ci != i)
          CHadd(cs, ci);
      }
    ଯଦି (cs->multis != ଖାଲି)
      mccase(p, cs);
  }
  ଯଦି (invert) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    ପାଇଁ (i = p->g->csetsize - 1; i >= 0; i--)
      ଯଦି (CHIN(cs, i))
        CHsub(cs, i);
      ଅନ୍ୟଥା
        CHadd(cs, i);
    ଯଦି (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ଯଦି (cs->multis != ଖାଲି)
      mcinvert(p, cs);
  }

  ପ୍ରତିଜ୍ଞା(cs->multis == ଖାଲି); /* xxx */

  ଯଦି (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ଅନ୍ୟଥା {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ସ୍ଥିର ଶୂନ୍ୟ p_b_term(ସଂରଚନା parse *p, cset *cs) {
  ଅକ୍ଷର c;
  ଅକ୍ଷର start, finish;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* classify what we've got */
  ବାଛ ((MORE()) ? PEEK() : '\0') {
  ସ୍ଥିତି '[':
    c = (MORE2()) ? PEEK2() : '\0';
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '-':
    SETERROR(REG_ERANGE);
    ଫେରନ୍ତୁ; /* NOTE RETURN */
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    c = '\0';
    ଭାଙ୍ଗ;
  }

  ବାଛ (c) {
  ସ୍ଥିତି ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_ପ୍ରତୀକ(p);
    ଯଦି (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      ଯଦି (EAT('-'))
        finish = '-';
      ଅନ୍ୟଥା
        finish = p_b_ପ୍ରତୀକ(p);
    } ଅନ୍ୟଥା {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    ପାଇଁ (i = start; i <= finish; i++)
      CHadd(cs, i);
    ଭାଙ୍ଗ;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ସ୍ଥିର ଶୂନ୍ୟ p_b_cclass(ସଂରଚନା parse *p, cset *cs) {
  ଅଚଳ ଅକ୍ଷର *sp = p->next;
  ସଂରଚନା cclass *cp;
  size_t len;
  ଅଚଳ ଅକ୍ଷର *u;
  ଅକ୍ଷର c;

  ଯେତେବେଳେ (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  ପାଇଁ (cp = cclasses; cp->name != ଖାଲି; cp++)
    ଯଦି (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      ଭାଙ୍ଗ;
  ଯଦି (cp->name == ଖାଲି) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ଫେରନ୍ତୁ;
  }

  u = cp->chars;
  ଯେତେବେଳେ ((c = *u++) != '\0')
    CHadd(cs, c);
  ପାଇଁ (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ସ୍ଥିର ଶୂନ୍ୟ p_b_eclass(ସଂରଚନା parse *p, cset *cs) {
  ଅକ୍ଷର c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ସ୍ଥିର ଅକ୍ଷର /* value of symbol */
p_b_ପ୍ରତୀକ(ସଂରଚନା parse *p) {
  ଅକ୍ଷର ମୂଲ୍ୟ_2;

  REQUIRE(MORE(), REG_EBRACK);
  ଯଦି (!EATTWO('[', '.'))
    ଫେରନ୍ତୁ (GETNEXT());

  /* collating symbol */
  ମୂଲ୍ୟ_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ଫେରନ୍ତୁ (ମୂଲ୍ୟ_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ସ୍ଥିର ଅକ୍ଷର                              /* value of collating element */
p_b_coll_elem(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା endc) /* name ended by endc,']' */
{
  ଅଚଳ ଅକ୍ଷର *sp = p->next;
  ସଂରଚନା cname *cp;
  size_t len;

  ଯେତେବେଳେ (MORE() && !SEETWO(endc, ']'))
    NEXT();
  ଯଦି (!MORE()) {
    SETERROR(REG_EBRACK);
    ଫେରନ୍ତୁ (0);
  }
  len = p->next - sp;
  ପାଇଁ (cp = cnames; cp->name != ଖାଲି; cp++)
    ଯଦି (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      ଫେରନ୍ତୁ (cp->code); /* known name */
  ଯଦି (len == 1)
    ଫେରନ୍ତୁ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ଫେରନ୍ତୁ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ସ୍ଥିର ଅକ୍ଷର /* if no counterpart, return ch */
othercase(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch) {
  ch = (uch)ch;
  ପ୍ରତିଜ୍ଞା(isalpha(ch));
  ଯଦି (isupper(ch))
    ଫେରନ୍ତୁ ((uch)tolower(ch));
  ଅନ୍ୟଥା ଯଦି (islower(ch))
    ଫେରନ୍ତୁ ((uch)toupper(ch));
  ଅନ୍ୟଥା /* peculiar, but could happen */
    ଫେରନ୍ତୁ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ସ୍ଥିର ଶୂନ୍ୟ bothcases(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch) {
  ଅଚଳ ଅକ୍ଷର *oldnext = p->next;
  ଅଚଳ ଅକ୍ଷର *oldend = p->end;
  ଅକ୍ଷର bracket[3];

  ch = (uch)ch;
  ପ୍ରତିଜ୍ଞା(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ପ୍ରତିଜ୍ଞା(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ସ୍ଥିର ଶୂନ୍ୟ uncased(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ଯଦି (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ସ୍ଥିର ଶୂନ୍ୟ ordinary(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch) {
  ଯଦି ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ଅନ୍ୟଥା
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ସ୍ଥିର ଶୂନ୍ୟ nonnewline(ସଂରଚନା parse *p) {
  ଅଚଳ ଅକ୍ଷର *oldnext = p->next;
  ଅଚଳ ଅକ୍ଷର *oldend = p->end;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  ପ୍ରତିଜ୍ଞା(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ସ୍ଥିର ଶୂନ୍ୟ repeat(ସଂରଚନା parse *p,
                   sopno start, /* operand from here to end of strip */
                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା from,    /* repeated from this number */
                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ଯଦି (p->error != 0) /* head off possible runaway recursion */
    ଫେରନ୍ତୁ;

  ପ୍ରତିଜ୍ଞା(from <= to);

  ବାଛ (REP(MAP(from), MAP(to))) {
  ସ୍ଥିତି REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(0, 1):   /* as x{1,1}? */
  ସ୍ଥିତି REP(0, N):   /* as x{1,n}? */
  ସ୍ଥିତି REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ଭର୍ତ୍ତି_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(1, 1): /* trivial case */
    /* done */
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ଭର୍ତ୍ତି_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    ପ୍ରତିଜ୍ଞା(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(1, INF): /* as x+ */
    ଭର୍ତ୍ତି_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    ଭାଙ୍ଗ;
  }
}

/*
 - seterr - set an error condition
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* useless but makes type checking happy */
seterr(ସଂରଚନା parse *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା e) {
  ଯଦି (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  ଫେରନ୍ତୁ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ସ୍ଥିର cset *allocset(ସଂରଚନା parse *p) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (no >= p->ncsalloc) { /* need another column of space */
    ଶୂନ୍ୟ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ଯଦି (nc > SIZE_MAX / ଆକାର(cset))
      ଯାଅ nomem;
    ପ୍ରତିଜ୍ଞା(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((ଅକ୍ଷର *)p->g->sets, nc * ଆକାର(cset));
    ଯଦି (ptr == ଖାଲି)
      ଯାଅ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((ଅକ୍ଷର *)p->g->setbits, nbytes);
    ଯଦି (ptr == ଖାଲି)
      ଯାଅ nomem;
    p->g->setbits = ptr;

    ପାଇଁ (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (ଶୂନ୍ୟ)memset((ଅକ୍ଷର *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ଯଦି (p->g->sets == ଖାଲି || p->g->setbits == ଖାଲି)
    ଯାଅ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ଖାଲି;

  ଫେରନ୍ତୁ (cs);
nomem:
  ମୁକ୍ତ(p->g->sets);
  p->g->sets = ଖାଲି;
  ମୁକ୍ତ(p->g->setbits);
  p->g->setbits = ଖାଲି;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ଫେରନ୍ତୁ (ଖାଲି);
}

/*
 - freeset - free a now-unused set
 */
ସ୍ଥିର ଶୂନ୍ୟ freeset(ସଂରଚନା parse *p, cset *cs) {
  size_t i;
  cset *ଶୀର୍ଷ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  ପାଇଁ (i = 0; i < css; i++)
    CHsub(cs, i);
  ଯଦି (cs == ଶୀର୍ଷ_2 - 1) /* recover only the easy case */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* set number */
freezeset(ସଂରଚନା parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *ଶୀର୍ଷ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  ପାଇଁ (cs2 = &p->g->sets[0]; cs2 < ଶୀର୍ଷ_2; cs2++)
    ଯଦି (cs2->hash == h && cs2 != cs) {
      /* maybe */
      ପାଇଁ (i = 0; i < css; i++)
        ଯଦି (!!CHIN(cs2, i) != !!CHIN(cs, i))
          ଭାଙ୍ଗ; /* no */
      ଯଦି (i == css)
        ଭାଙ୍ଗ; /* yes */
    }

  ଯଦି (cs2 < ଶୀର୍ଷ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ଫେରନ୍ତୁ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* character; there is no "none" value */
firstch(ସଂରଚନା parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  ପାଇଁ (i = 0; i < css; i++)
    ଯଦି (CHIN(cs, i))
      ଫେରନ୍ତୁ ((ଅକ୍ଷର)i);
  ପ୍ରତିଜ୍ଞା(never);
  ଫେରନ୍ତୁ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nch(ସଂରଚନା parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;

  ପାଇଁ (i = 0; i < css; i++)
    ଯଦି (CHIN(cs, i))
      n++;
  ଫେରନ୍ତୁ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ସ୍ଥିର ଶୂନ୍ୟ mcadd(ସଂରଚନା parse *p, cset *cs, ଅଚଳ ଅକ୍ଷର *cp) {
  size_t oldend = cs->smultis;
  ଶୂନ୍ୟ *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ଯଦି (np == ଖାଲି) {
    ଯଦି (cs->multis)
      ମୁକ୍ତ(cs->multis);
    cs->multis = ଖାଲି;
    SETERROR(REG_ESPACE);
    ଫେରନ୍ତୁ;
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
ସ୍ଥିର ଶୂନ୍ୟ mcinvert(ସଂରଚନା parse *p, cset *cs) {
  ପ୍ରତିଜ୍ଞା(cs->multis == ଖାଲି); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ସ୍ଥିର ଶୂନ୍ୟ mccase(ସଂରଚନା parse *p, cset *cs) {
  ପ୍ରତିଜ୍ଞା(cs->multis == ଖାଲି); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* predicate */
isinsets(ସଂରଚନା re_guts *g, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  uch *col;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  ପାଇଁ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ଯଦି (col[uc] != 0)
      ଫେରନ୍ତୁ (1);
  ଫେରନ୍ତୁ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା /* predicate */
samesets(ସଂରଚନା re_guts *g, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c2) {
  uch *col;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  ପାଇଁ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ଯଦି (col[uc1] != col[uc2])
      ଫେରନ୍ତୁ (0);
  ଫେରନ୍ତୁ (1);
}

/*
 - categorize - sort out character categories
 */
ସ୍ଥିର ଶୂନ୍ୟ categorize(ସଂରଚନା parse *p, ସଂରଚନା re_guts *g) {
  cat_t *cats = g->categories;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c2;
  cat_t cat;

  /* avoid making error situations worse */
  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ;

  ପାଇଁ (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ଯଦି (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      ପାଇଁ (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ଯଦି (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ସ୍ଥିର sopno                       /* start of duplicate */
dupl(ସଂରଚନା parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  ପ୍ରତିଜ୍ଞା(finish >= start);
  ଯଦି (len == 0)
    ଫେରନ୍ତୁ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ପ୍ରତିଜ୍ଞା(p->ssize >= p->slen + len);
  (ଶୂନ୍ୟ)memmove((ଅକ୍ଷର *)(p->strip + p->slen), (ଅକ୍ଷର *)(p->strip + start),
                (size_t)len * ଆକାର(sop));
  p->slen += len;
  ଫେରନ୍ତୁ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ସ୍ଥିର ଶୂନ୍ୟ doemit(ସଂରଚନା parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ;

  /* deal with oversize operands ("can't happen", more or less) */
  ପ୍ରତିଜ୍ଞା(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ଯଦି (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ପ୍ରତିଜ୍ଞା(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ସ୍ଥିର ଶୂନ୍ୟ doinsert(ସଂରଚନା parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* avoid making error situations worse */
  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  ପ୍ରତିଜ୍ଞା(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ପ୍ରତିଜ୍ଞା(pos > 0);
  ପାଇଁ (i = 1; i < NPAREN; i++) {
    ଯଦି (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ଯଦି (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((ଅକ୍ଷର *)&p->strip[pos + 1], (ଅକ୍ଷର *)&p->strip[pos],
          (HERE() - pos - 1) * ଆକାର(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ସ୍ଥିର ଶୂନ୍ୟ dofwd(ସଂରଚନା parse *p, sopno pos, sop ମୂଲ୍ୟ_2) {
  /* avoid making error situations worse */
  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ;

  ପ୍ରତିଜ୍ଞା(ମୂଲ୍ୟ_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | ମୂଲ୍ୟ_2;
}

/*
 - enlarge - enlarge the strip
 */
ସ୍ଥିର ଶୂନ୍ୟ enlarge(ସଂରଚନା parse *p, sopno size) {
  sop *sp;

  ଯଦି (p->ssize >= size)
    ଫେରନ୍ତୁ;

  ଯଦି ((uintptr_t)size > SIZE_MAX / ଆକାର(sop)) {
    SETERROR(REG_ESPACE);
    ଫେରନ୍ତୁ;
  }

  sp = (sop *)realloc(p->strip, size * ଆକାର(sop));
  ଯଦି (sp == ଖାଲି) {
    SETERROR(REG_ESPACE);
    ଫେରନ୍ତୁ;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
ସ୍ଥିର ଶୂନ୍ୟ stripsnug(ସଂରଚନା parse *p, ସଂରଚନା re_guts *g) {
  g->nstates = p->slen;
  ଯଦି ((uintptr_t)p->slen > SIZE_MAX / ଆକାର(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ଫେରନ୍ତୁ;
  }

  g->strip = (sop *)realloc((ଅକ୍ଷର *)p->strip, p->slen * ଆକାର(sop));
  ଯଦି (g->strip == ଖାଲି) {
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
ସ୍ଥିର ଶୂନ୍ୟ findmust(ସଂରଚନା parse *p, ସଂରଚନା re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  ଅକ୍ଷର *cp;
  sopno i;

  /* avoid making error situations worse */
  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  କର {
    s = *scan++;
    ବାଛ (OP(s)) {
    ସ୍ଥିତି OCHAR:        /* sequence member */
      ଯଦି (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି OPLUS_: /* things that don't break one */
    ସ୍ଥିତି OLPAREN:
    ସ୍ଥିତି ORPAREN:
      ଭାଙ୍ଗ;
    ସ୍ଥିତି OQUEST_: /* things that must be skipped */
    ସ୍ଥିତି OCH_:
      scan--;
      କର {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        ଯଦି (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ଫେରନ୍ତୁ;
        }
      } ଯେତେବେଳେ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:                  /* things that break a sequence */
      ଯଦି (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      ଭାଙ୍ଗ;
    }
  } ଯେତେବେଳେ (OP(s) != OEND);

  ଯଦି (g->mlen == 0) /* there isn't one */
    ଫେରନ୍ତୁ;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  ଯଦି (g->must == ଖାଲି) { /* argh; just forget it */
    g->mlen = 0;
    ଫେରନ୍ତୁ;
  }
  cp = g->must;
  scan = start;
  ପାଇଁ (i = g->mlen; i > 0; i--) {
    ଯେତେବେଳେ (OP(s = *scan++) != OCHAR)
      ଜାରି;
    ପ୍ରତିଜ୍ଞା(cp < g->must + g->mlen);
    *cp++ = (ଅକ୍ଷର)OPND(s);
  }
  ପ୍ରତିଜ୍ଞା(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ସ୍ଥିର sopno /* nesting depth */
pluscount(ସଂରଚନା parse *p, ସଂରଚନା re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ଯଦି (p->error != 0)
    ଫେରନ୍ତୁ (0); /* there may not be an OEND */

  scan = g->strip + 1;
  କର {
    s = *scan++;
    ବାଛ (OP(s)) {
    ସ୍ଥିତି OPLUS_:
      plusnest++;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି O_PLUS:
      ଯଦି (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      ଭାଙ୍ଗ;
    }
  } ଯେତେବେଳେ (OP(s) != OEND);
  ଯଦି (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ଫେରନ୍ତୁ (maxnest);
}
