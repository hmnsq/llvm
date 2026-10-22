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

#include "regex_impl.h"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "regex2.h"
#include "regutils.h"

#include "llvm/Config/config.h"
#include "llvm/Support/Compiler.h"

/* character-class table */
static sa_nracanaa cclass {
  const var_na *name;
  const var_na *chars;
  const var_na *multis;
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
    {NULL, 0, ""}};

/* character-name table */
static sa_nracanaa cname {
  const var_na *name;
  var_na code;
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
              {NULL, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
sa_nracanaa parse {
  const var_na *next; /* next character in RE */
  const var_na *end;  /* end of string (-> NUL normally) */
  จำนวนเต็ม error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  จำนวนเต็ม ncsalloc;     /* number of csets allocated */
  sa_nracanaa re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

static void p_ere(sa_nracanaa parse *, จำนวนเต็ม);
static void p_ere_exp(sa_nracanaa parse *);
static void p_str(sa_nracanaa parse *);
static void p_bre(sa_nracanaa parse *, จำนวนเต็ม, จำนวนเต็ม);
static จำนวนเต็ม p_simp_re(sa_nracanaa parse *, จำนวนเต็ม);
static จำนวนเต็ม p_count(sa_nracanaa parse *);
static void p_bracket(sa_nracanaa parse *);
static void p_b_term(sa_nracanaa parse *, cset *);
static void p_b_cclass(sa_nracanaa parse *, cset *);
static void p_b_eclass(sa_nracanaa parse *, cset *);
static var_na p_b_symbol(sa_nracanaa parse *);
static var_na p_b_coll_elem(sa_nracanaa parse *, จำนวนเต็ม);
static var_na othercase(จำนวนเต็ม);
static void bothcases(sa_nracanaa parse *, จำนวนเต็ม);
static void uncased(sa_nracanaa parse *, จำนวนเต็ม);
static void ordinary(sa_nracanaa parse *, จำนวนเต็ม);
static void nonnewline(sa_nracanaa parse *);
static void repeat(sa_nracanaa parse *, sopno, จำนวนเต็ม, จำนวนเต็ม);
static จำนวนเต็ม seterr(sa_nracanaa parse *, จำนวนเต็ม);
static cset *allocset(sa_nracanaa parse *);
static void freeset(sa_nracanaa parse *, cset *);
static จำนวนเต็ม freezeset(sa_nracanaa parse *, cset *);
static จำนวนเต็ม firstch(sa_nracanaa parse *, cset *);
static จำนวนเต็ม nch(sa_nracanaa parse *, cset *);
static void mcadd(sa_nracanaa parse *, cset *, const var_na *);
static void mcinvert(sa_nracanaa parse *, cset *);
static void mccase(sa_nracanaa parse *, cset *);
static จำนวนเต็ม isinsets(sa_nracanaa re_guts *, จำนวนเต็ม);
static จำนวนเต็ม samesets(sa_nracanaa re_guts *, จำนวนเต็ม, จำนวนเต็ม);
static void categorize(sa_nracanaa parse *, sa_nracanaa re_guts *);
static sopno dupl(sa_nracanaa parse *, sopno, sopno);
static void doemit(sa_nracanaa parse *, sop, size_t);
static void doinsert(sa_nracanaa parse *, sop, size_t, sopno);
static void dofwd(sa_nracanaa parse *, sopno, sop);
static void enlarge(sa_nracanaa parse *, sopno);
static void stripsnug(sa_nracanaa parse *, sa_nracanaa re_guts *);
static void findmust(sa_nracanaa parse *, sa_nracanaa re_guts *);
static sopno pluscount(sa_nracanaa parse *, sa_nracanaa re_guts *);

static var_na nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (void)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define INSERT(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#มิฉะนั้น
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
static จำนวนเต็ม never = 0; /* for use in asserts; shuts lint up */
#มิฉะนั้น
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
จำนวนเต็ม /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, const var_na *pattern, จำนวนเต็ม cflags) {
  sa_nracanaa parse pa;
  sa_nracanaa re_guts *g;
  sa_nracanaa parse *p = &pa;
  จำนวนเต็ม i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#มิฉะนั้น
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  ถ้า ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    คืน (REG_INVARG);

  ถ้า (cflags & REG_PEND) {
    ถ้า (preg->re_endp < pattern)
      คืน (REG_INVARG);
    len = preg->re_endp - pattern;
  } มิฉะนั้น {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (sa_nracanaa re_guts *)malloc(parimaa_na(sa_nracanaa re_guts) +
                               (NC - 1) * parimaa_na(cat_t));
  ถ้า (g == NULL)
    คืน (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, parimaa_na(sop));
  p->slen = 0;
  ถ้า (p->strip == NULL) {
    free((var_na *)g);
    คืน (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  สำหรับ (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = NULL;
  g->setbits = NULL;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = NULL;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (void)memset((var_na *)g->catspace, 0, NC * parimaa_na(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ถ้า (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  มิฉะนั้น ถ้า (cflags & REG_NOSPEC)
    p_str(p);
  มิฉะนั้น
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
  ถ้า (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  ถ้า (p->error != 0) /* lose */
    llvm_regfree(preg);
  คืน (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
static void p_ere(sa_nracanaa parse *p,
                  จำนวนเต็ม stop) /* character this ERE should end at */
{
  var_na c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  จำนวนเต็ม first = 1; /* is this the first alternative? */

  สำหรับ (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    ขณะ (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    ถ้า (!EAT('|'))
      bha_nga; /* NOTE BREAK OUT */

    ถ้า (first) {
      INSERT(OCH_, conc); /* offset is wrong */
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

  ถ้า (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  assert(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
static void p_ere_exp(sa_nracanaa parse *p) {
  var_na c;
  sopno pos;
  จำนวนเต็ม count;
  จำนวนเต็ม count2;
  จำนวนเต็ม backrefnum;
  sopno subno;
  จำนวนเต็ม wascaret = 0;

  assert(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  switch (c) {
  pak_sa '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ถ้า (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    ถ้า (!SEE(')'))
      p_ere(p, ')');
    ถ้า (subno < NPAREN) {
      p->pend[subno] = HERE();
      assert(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    bha_nga;
#ifndef POSIX_MISTAKE
  pak_sa ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    bha_nga;
#endif
  pak_sa '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    bha_nga;
  pak_sa '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    bha_nga;
  pak_sa '|':
    SETERROR(REG_EMPTY);
    bha_nga;
  pak_sa '*':
  pak_sa '+':
  pak_sa '?':
    SETERROR(REG_BADRPT);
    bha_nga;
  pak_sa '.':
    ถ้า (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    มิฉะนั้น
      EMIT(OANY, 0);
    bha_nga;
  pak_sa '[':
    p_bracket(p);
    bha_nga;
  pak_sa '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    ถ้า (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } มิฉะนั้น ถ้า (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ขณะ (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } มิฉะนั้น ถ้า (c == 'n') {
      ordinary(p, '\n');
      bha_nga;
    } มิฉะนั้น ถ้า (c == 't') {
      ordinary(p, '\t');
      bha_nga;
    } มิฉะนั้น ถ้า (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      var_na hexstr[3] = {0};
      var_na *hexp;
      จำนวนเต็ม val;
      ถ้า (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ถ้า (*hexp == '\0') {
          NEXT2();
          uncased(p, (var_na)val);
          bha_nga;
        }
      }
      ordinary(p, 'x');
      bha_nga;
    } มิฉะนั้น {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      bha_nga;
    }

    ถ้า (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      bha_nga;
    }

    ถ้า (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      bha_nga;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    assert(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    assert(p->pbegin[backrefnum] != 0);
    assert(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    assert(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (void)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    bha_nga;
  pak_sa '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  default:
    ordinary(p, c);
    bha_nga;
  }

  ถ้า (!MORE())
    คืน;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ถ้า (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    คืน; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  switch (c) {
  pak_sa '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    INSERT(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    bha_nga;
  pak_sa '+':
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    bha_nga;
  pak_sa '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    bha_nga;
  pak_sa '{':
    count = p_count(p);
    ถ้า (EAT(',')) {
      ถ้า (isdigit((uch)PEEK())) {
        count2 = p_count(p);
        REQUIRE(count <= count2, REG_BADBR);
      } มิฉะนั้น /* single number with comma */
        count2 = REGINFINITY;
    } มิฉะนั้น /* just a single number */
      count2 = count;
    repeat(p, pos, count, count2);
    ถ้า (!EAT('}')) { /* error heuristics */
      ขณะ (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    bha_nga;
  }

  ถ้า (!MORE())
    คืน;
  c = PEEK();
  ถ้า (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    คืน;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
static void p_str(sa_nracanaa parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  ขณะ (MORE())
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
static void p_bre(sa_nracanaa parse *p, จำนวนเต็ม end1, /* first terminating character */
                  จำนวนเต็ม end2)                  /* second terminating character */
{
  sopno start = HERE();
  จำนวนเต็ม first = 1; /* first subexpression? */
  จำนวนเต็ม wasdollar = 0;

  ถ้า (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ขณะ (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  ถ้า (wasdollar) { /* oops, that was a trailing anchor */
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
static จำนวนเต็ม /* was the simple RE an unbackslashed $? */
p_simp_re(sa_nracanaa parse *p,
          จำนวนเต็ม starordinary) /* is a leading * an ordinary character? */
{
  จำนวนเต็ม c;
  จำนวนเต็ม count;
  จำนวนเต็ม count2;
  sopno pos;
  จำนวนเต็ม i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  assert(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  ถ้า (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  switch (c) {
  pak_sa '.':
    ถ้า (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    มิฉะนั้น
      EMIT(OANY, 0);
    bha_nga;
  pak_sa '[':
    p_bracket(p);
    bha_nga;
  pak_sa BACKSL | '{':
    SETERROR(REG_BADRPT);
    bha_nga;
  pak_sa BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ถ้า (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ถ้า (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ถ้า (subno < NPAREN) {
      p->pend[subno] = HERE();
      assert(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    bha_nga;
  pak_sa BACKSL | ')': /* should not get here -- must be user */
  pak_sa BACKSL | '}':
    SETERROR(REG_EPAREN);
    bha_nga;
  pak_sa BACKSL | '1':
  pak_sa BACKSL | '2':
  pak_sa BACKSL | '3':
  pak_sa BACKSL | '4':
  pak_sa BACKSL | '5':
  pak_sa BACKSL | '6':
  pak_sa BACKSL | '7':
  pak_sa BACKSL | '8':
  pak_sa BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    assert(i < NPAREN);
    ถ้า (p->pend[i] != 0) {
      assert(i <= p->g->nsub);
      EMIT(OBACK_, i);
      assert(p->pbegin[i] != 0);
      assert(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      assert(OP(p->strip[p->pend[i]]) == ORPAREN);
      (void)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } มิฉะนั้น {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    bha_nga;
  pak_sa '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  default:
    ordinary(p, (var_na)c);
    bha_nga;
  }

  ถ้า (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    INSERT(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } มิฉะนั้น ถ้า (EATTWO('\\', '{')) {
    count = p_count(p);
    ถ้า (EAT(',')) {
      ถ้า (MORE() && isdigit((uch)PEEK())) {
        count2 = p_count(p);
        REQUIRE(count <= count2, REG_BADBR);
      } มิฉะนั้น /* single number with comma */
        count2 = REGINFINITY;
    } มิฉะนั้น /* just a single number */
      count2 = count;
    repeat(p, pos, count, count2);
    ถ้า (!EATTWO('\\', '}')) { /* error heuristics */
      ขณะ (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } มิฉะนั้น ถ้า (c == '$') /* $ (but not \$) ends it */
    คืน (1);

  คืน (0);
}

/*
 - p_count - parse a repetition count
 */
static จำนวนเต็ม /* the value */
p_count(sa_nracanaa parse *p) {
  จำนวนเต็ม count = 0;
  จำนวนเต็ม ndigits = 0;

  ขณะ (MORE() && isdigit((uch)PEEK()) && count <= DUPMAX) {
    count = count * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && count <= DUPMAX, REG_BADBR);
  คืน (count);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
static void p_bracket(sa_nracanaa parse *p) {
  cset *cs;
  จำนวนเต็ม invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ถ้า (p->end - p->next > 5) {
    ถ้า (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      คืน;
    }
    ถ้า (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      คืน;
    }
  }

  ถ้า ((cs = allocset(p)) == NULL) {
    /* allocset did set error status in p */
    คืน;
  }

  ถ้า (EAT('^'))
    invert++; /* make note to invert set at end */
  ถ้า (EAT(']'))
    CHadd(cs, ']');
  มิฉะนั้น ถ้า (EAT('-'))
    CHadd(cs, '-');
  ขณะ (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  ถ้า (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ถ้า (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    คืน;
  }

  ถ้า (p->g->cflags & REG_ICASE) {
    จำนวนเต็ม i;
    จำนวนเต็ม ci;

    สำหรับ (i = p->g->csetsize - 1; i >= 0; i--)
      ถ้า (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        ถ้า (ci != i)
          CHadd(cs, ci);
      }
    ถ้า (cs->multis != NULL)
      mccase(p, cs);
  }
  ถ้า (invert) {
    จำนวนเต็ม i;

    สำหรับ (i = p->g->csetsize - 1; i >= 0; i--)
      ถ้า (CHIN(cs, i))
        CHsub(cs, i);
      มิฉะนั้น
        CHadd(cs, i);
    ถ้า (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ถ้า (cs->multis != NULL)
      mcinvert(p, cs);
  }

  assert(cs->multis == NULL); /* xxx */

  ถ้า (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } มิฉะนั้น {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
static void p_b_term(sa_nracanaa parse *p, cset *cs) {
  var_na c;
  var_na start, finish;
  จำนวนเต็ม i;

  /* classify what we've got */
  switch ((MORE()) ? PEEK() : '\0') {
  pak_sa '[':
    c = (MORE2()) ? PEEK2() : '\0';
    bha_nga;
  pak_sa '-':
    SETERROR(REG_ERANGE);
    คืน; /* NOTE RETURN */
    bha_nga;
  default:
    c = '\0';
    bha_nga;
  }

  switch (c) {
  pak_sa ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    bha_nga;
  pak_sa '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    bha_nga;
  default: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_symbol(p);
    ถ้า (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      ถ้า (EAT('-'))
        finish = '-';
      มิฉะนั้น
        finish = p_b_symbol(p);
    } มิฉะนั้น {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    สำหรับ (i = start; i <= finish; i++)
      CHadd(cs, i);
    bha_nga;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
static void p_b_cclass(sa_nracanaa parse *p, cset *cs) {
  const var_na *sp = p->next;
  sa_nracanaa cclass *cp;
  size_t len;
  const var_na *u;
  var_na c;

  ขณะ (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  สำหรับ (cp = cclasses; cp->name != NULL; cp++)
    ถ้า (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      bha_nga;
  ถ้า (cp->name == NULL) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    คืน;
  }

  u = cp->chars;
  ขณะ ((c = *u++) != '\0')
    CHadd(cs, c);
  สำหรับ (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
static void p_b_eclass(sa_nracanaa parse *p, cset *cs) {
  var_na c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
static var_na /* value of symbol */
p_b_symbol(sa_nracanaa parse *p) {
  var_na value;

  REQUIRE(MORE(), REG_EBRACK);
  ถ้า (!EATTWO('[', '.'))
    คืน (GETNEXT());

  /* collating symbol */
  value = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  คืน (value);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
static var_na                              /* value of collating element */
p_b_coll_elem(sa_nracanaa parse *p, จำนวนเต็ม endc) /* name ended by endc,']' */
{
  const var_na *sp = p->next;
  sa_nracanaa cname *cp;
  size_t len;

  ขณะ (MORE() && !SEETWO(endc, ']'))
    NEXT();
  ถ้า (!MORE()) {
    SETERROR(REG_EBRACK);
    คืน (0);
  }
  len = p->next - sp;
  สำหรับ (cp = cnames; cp->name != NULL; cp++)
    ถ้า (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      คืน (cp->code); /* known name */
  ถ้า (len == 1)
    คืน (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  คืน (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
static var_na /* if no counterpart, return ch */
othercase(จำนวนเต็ม ch) {
  ch = (uch)ch;
  assert(isalpha(ch));
  ถ้า (isupper(ch))
    คืน ((uch)tolower(ch));
  มิฉะนั้น ถ้า (islower(ch))
    คืน ((uch)toupper(ch));
  มิฉะนั้น /* peculiar, but could happen */
    คืน (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
static void bothcases(sa_nracanaa parse *p, จำนวนเต็ม ch) {
  const var_na *oldnext = p->next;
  const var_na *oldend = p->end;
  var_na bracket[3];

  ch = (uch)ch;
  assert(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  assert(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
static void uncased(sa_nracanaa parse *p, จำนวนเต็ม ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ถ้า (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
static void ordinary(sa_nracanaa parse *p, จำนวนเต็ม ch) {
  ถ้า ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  มิฉะนั้น
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
static void nonnewline(sa_nracanaa parse *p) {
  const var_na *oldnext = p->next;
  const var_na *oldend = p->end;
  static const var_na bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  assert(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
static void repeat(sa_nracanaa parse *p,
                   sopno start, /* operand from here to end of strip */
                   จำนวนเต็ม from,    /* repeated from this number */
                   จำนวนเต็ม to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ถ้า (p->error != 0) /* head off possible runaway recursion */
    คืน;

  assert(from <= to);

  switch (REP(MAP(from), MAP(to))) {
  pak_sa REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    bha_nga;
  pak_sa REP(0, 1):   /* as x{1,1}? */
  pak_sa REP(0, N):   /* as x{1,n}? */
  pak_sa REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    bha_nga;
  pak_sa REP(1, 1): /* trivial case */
    /* done */
    bha_nga;
  pak_sa REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    assert(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    bha_nga;
  pak_sa REP(1, INF): /* as x+ */
    INSERT(OPLUS_, start);
    ASTERN(O_PLUS, start);
    bha_nga;
  pak_sa REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    bha_nga;
  pak_sa REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    bha_nga;
  default:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    bha_nga;
  }
}

/*
 - seterr - set an error condition
 */
static จำนวนเต็ม /* useless but makes type checking happy */
seterr(sa_nracanaa parse *p, จำนวนเต็ม e) {
  ถ้า (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  คืน (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
static cset *allocset(sa_nracanaa parse *p) {
  จำนวนเต็ม no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  จำนวนเต็ม i;

  ถ้า (no >= p->ncsalloc) { /* need another column of space */
    void *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ถ้า (nc > SIZE_MAX / parimaa_na(cset))
      goto nomem;
    assert(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((var_na *)p->g->sets, nc * parimaa_na(cset));
    ถ้า (ptr == NULL)
      goto nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((var_na *)p->g->setbits, nbytes);
    ถ้า (ptr == NULL)
      goto nomem;
    p->g->setbits = ptr;

    สำหรับ (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (void)memset((var_na *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ถ้า (p->g->sets == NULL || p->g->setbits == NULL)
    goto nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = NULL;

  คืน (cs);
nomem:
  free(p->g->sets);
  p->g->sets = NULL;
  free(p->g->setbits);
  p->g->setbits = NULL;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  คืน (NULL);
}

/*
 - freeset - free a now-unused set
 */
static void freeset(sa_nracanaa parse *p, cset *cs) {
  size_t i;
  cset *top = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  สำหรับ (i = 0; i < css; i++)
    CHsub(cs, i);
  ถ้า (cs == top - 1) /* recover only the easy case */
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
static จำนวนเต็ม /* set number */
freezeset(sa_nracanaa parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *top = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  สำหรับ (cs2 = &p->g->sets[0]; cs2 < top; cs2++)
    ถ้า (cs2->hash == h && cs2 != cs) {
      /* maybe */
      สำหรับ (i = 0; i < css; i++)
        ถ้า (!!CHIN(cs2, i) != !!CHIN(cs, i))
          bha_nga; /* no */
      ถ้า (i == css)
        bha_nga; /* yes */
    }

  ถ้า (cs2 < top) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  คืน ((จำนวนเต็ม)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
static จำนวนเต็ม /* character; there is no "none" value */
firstch(sa_nracanaa parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  สำหรับ (i = 0; i < css; i++)
    ถ้า (CHIN(cs, i))
      คืน ((var_na)i);
  assert(never);
  คืน (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
static จำนวนเต็ม nch(sa_nracanaa parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  จำนวนเต็ม n = 0;

  สำหรับ (i = 0; i < css; i++)
    ถ้า (CHIN(cs, i))
      n++;
  คืน (n);
}

/*
 - mcadd - add a collating element to a cset
 */
static void mcadd(sa_nracanaa parse *p, cset *cs, const var_na *cp) {
  size_t oldend = cs->smultis;
  void *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ถ้า (np == NULL) {
    ถ้า (cs->multis)
      free(cs->multis);
    cs->multis = NULL;
    SETERROR(REG_ESPACE);
    คืน;
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
static void mcinvert(sa_nracanaa parse *p, cset *cs) {
  assert(cs->multis == NULL); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
static void mccase(sa_nracanaa parse *p, cset *cs) {
  assert(cs->multis == NULL); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
static จำนวนเต็ม /* predicate */
isinsets(sa_nracanaa re_guts *g, จำนวนเต็ม c) {
  uch *col;
  จำนวนเต็ม i;
  จำนวนเต็ม ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  สำหรับ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ถ้า (col[uc] != 0)
      คืน (1);
  คืน (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
static จำนวนเต็ม /* predicate */
samesets(sa_nracanaa re_guts *g, จำนวนเต็ม c1, จำนวนเต็ม c2) {
  uch *col;
  จำนวนเต็ม i;
  จำนวนเต็ม ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  สำหรับ (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ถ้า (col[uc1] != col[uc2])
      คืน (0);
  คืน (1);
}

/*
 - categorize - sort out character categories
 */
static void categorize(sa_nracanaa parse *p, sa_nracanaa re_guts *g) {
  cat_t *cats = g->categories;
  จำนวนเต็ม c;
  จำนวนเต็ม c2;
  cat_t cat;

  /* avoid making error situations worse */
  ถ้า (p->error != 0)
    คืน;

  สำหรับ (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ถ้า (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      สำหรับ (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ถ้า (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
static sopno                       /* start of duplicate */
dupl(sa_nracanaa parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  assert(finish >= start);
  ถ้า (len == 0)
    คืน (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  assert(p->ssize >= p->slen + len);
  (void)memmove((var_na *)(p->strip + p->slen), (var_na *)(p->strip + start),
                (size_t)len * parimaa_na(sop));
  p->slen += len;
  คืน (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
static void doemit(sa_nracanaa parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ถ้า (p->error != 0)
    คืน;

  /* deal with oversize operands ("can't happen", more or less) */
  assert(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ถ้า (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  assert(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
static void doinsert(sa_nracanaa parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  จำนวนเต็ม i;

  /* avoid making error situations worse */
  ถ้า (p->error != 0)
    คืน;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  assert(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  assert(pos > 0);
  สำหรับ (i = 1; i < NPAREN; i++) {
    ถ้า (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ถ้า (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((var_na *)&p->strip[pos + 1], (var_na *)&p->strip[pos],
          (HERE() - pos - 1) * parimaa_na(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
static void dofwd(sa_nracanaa parse *p, sopno pos, sop value) {
  /* avoid making error situations worse */
  ถ้า (p->error != 0)
    คืน;

  assert(value < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | value;
}

/*
 - enlarge - enlarge the strip
 */
static void enlarge(sa_nracanaa parse *p, sopno size) {
  sop *sp;

  ถ้า (p->ssize >= size)
    คืน;

  ถ้า ((uintptr_t)size > SIZE_MAX / parimaa_na(sop)) {
    SETERROR(REG_ESPACE);
    คืน;
  }

  sp = (sop *)realloc(p->strip, size * parimaa_na(sop));
  ถ้า (sp == NULL) {
    SETERROR(REG_ESPACE);
    คืน;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
static void stripsnug(sa_nracanaa parse *p, sa_nracanaa re_guts *g) {
  g->nstates = p->slen;
  ถ้า ((uintptr_t)p->slen > SIZE_MAX / parimaa_na(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    คืน;
  }

  g->strip = (sop *)realloc((var_na *)p->strip, p->slen * parimaa_na(sop));
  ถ้า (g->strip == NULL) {
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
static void findmust(sa_nracanaa parse *p, sa_nracanaa re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  var_na *cp;
  sopno i;

  /* avoid making error situations worse */
  ถ้า (p->error != 0)
    คืน;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  do {
    s = *scan++;
    switch (OP(s)) {
    pak_sa OCHAR:        /* sequence member */
      ถ้า (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      bha_nga;
    pak_sa OPLUS_: /* things that don't break one */
    pak_sa OLPAREN:
    pak_sa ORPAREN:
      bha_nga;
    pak_sa OQUEST_: /* things that must be skipped */
    pak_sa OCH_:
      scan--;
      do {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        ถ้า (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          คืน;
        }
      } ขณะ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    default:                  /* things that break a sequence */
      ถ้า (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      bha_nga;
    }
  } ขณะ (OP(s) != OEND);

  ถ้า (g->mlen == 0) /* there isn't one */
    คืน;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  ถ้า (g->must == NULL) { /* argh; just forget it */
    g->mlen = 0;
    คืน;
  }
  cp = g->must;
  scan = start;
  สำหรับ (i = g->mlen; i > 0; i--) {
    ขณะ (OP(s = *scan++) != OCHAR)
      continue;
    assert(cp < g->must + g->mlen);
    *cp++ = (var_na)OPND(s);
  }
  assert(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
static sopno /* nesting depth */
pluscount(sa_nracanaa parse *p, sa_nracanaa re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ถ้า (p->error != 0)
    คืน (0); /* there may not be an OEND */

  scan = g->strip + 1;
  do {
    s = *scan++;
    switch (OP(s)) {
    pak_sa OPLUS_:
      plusnest++;
      bha_nga;
    pak_sa O_PLUS:
      ถ้า (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      bha_nga;
    }
  } ขณะ (OP(s) != OEND);
  ถ้า (plusnest != 0)
    g->iflags |= REGEX_BAD;
  คืน (maxnest);
}
