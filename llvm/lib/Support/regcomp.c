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

#অন্তর্ভাব "regex_impl.h"
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "regex2.h"
#অন্তর্ভাব "regutils.h"

#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"

/* character-class table */
স্থির কাঠামো cclass {
  ধ্রুবক অক্ষর *name;
  ধ্রুবক অক্ষর *chars;
  ধ্রুবক অক্ষর *multis;
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
    {ফাঁকা, 0, ""}};

/* character-name table */
স্থির কাঠামো cname {
  ধ্রুবক অক্ষর *name;
  অক্ষর code;
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
              {ফাঁকা, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
কাঠামো parse {
  ধ্রুবক অক্ষর *পরবর্তী; /* next character in RE */
  ধ্রুবক অক্ষর *end;  /* end of string (-> NUL normally) */
  পূর্ণসংখ্যা error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  পূর্ণসংখ্যা ncsalloc;     /* number of csets allocated */
  কাঠামো re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

স্থির শূন্য p_ere(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির শূন্য p_ere_exp(কাঠামো parse *);
স্থির শূন্য p_str(কাঠামো parse *);
স্থির শূন্য p_bre(কাঠামো parse *, পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা p_simp_re(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা p_গুনতি(কাঠামো parse *);
স্থির শূন্য p_bracket(কাঠামো parse *);
স্থির শূন্য p_b_term(কাঠামো parse *, cset *);
স্থির শূন্য p_b_cclass(কাঠামো parse *, cset *);
স্থির শূন্য p_b_eclass(কাঠামো parse *, cset *);
স্থির অক্ষর p_b_প্রতীক(কাঠামো parse *);
স্থির অক্ষর p_b_coll_elem(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির অক্ষর othercase(পূর্ণসংখ্যা);
স্থির শূন্য bothcases(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির শূন্য uncased(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির শূন্য ordinary(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির শূন্য nonnewline(কাঠামো parse *);
স্থির শূন্য repeat(কাঠামো parse *, sopno, পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা seterr(কাঠামো parse *, পূর্ণসংখ্যা);
স্থির cset *allocset(কাঠামো parse *);
স্থির শূন্য freeset(কাঠামো parse *, cset *);
স্থির পূর্ণসংখ্যা freezeset(কাঠামো parse *, cset *);
স্থির পূর্ণসংখ্যা firstch(কাঠামো parse *, cset *);
স্থির পূর্ণসংখ্যা nch(কাঠামো parse *, cset *);
স্থির শূন্য mcadd(কাঠামো parse *, cset *, ধ্রুবক অক্ষর *);
স্থির শূন্য mcinvert(কাঠামো parse *, cset *);
স্থির শূন্য mccase(কাঠামো parse *, cset *);
স্থির পূর্ণসংখ্যা isinsets(কাঠামো re_guts *, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা samesets(কাঠামো re_guts *, পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির শূন্য categorize(কাঠামো parse *, কাঠামো re_guts *);
স্থির sopno dupl(কাঠামো parse *, sopno, sopno);
স্থির শূন্য doemit(কাঠামো parse *, sop, size_t);
স্থির শূন্য doinsert(কাঠামো parse *, sop, size_t, sopno);
স্থির শূন্য dofwd(কাঠামো parse *, sopno, sop);
স্থির শূন্য enlarge(কাঠামো parse *, sopno);
স্থির শূন্য stripsnug(কাঠামো parse *, কাঠামো re_guts *);
স্থির শূন্য findmust(কাঠামো parse *, কাঠামো re_guts *);
স্থির sopno pluscount(কাঠামো parse *, কাঠামো re_guts *);

স্থির অক্ষর nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#define PEEK() (*p->পরবর্তী)
#define PEEK2() (*(p->পরবর্তী + 1))
#define MORE() (p->end - p->পরবর্তী > 0)
#define MORE2() (p->end - p->পরবর্তী > 1)
#define SEE(c) (MORE() && PEEK() == (c))
#define SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#define EAT(c) ((SEE(c)) ? (পরবর্তী_3(), 1) : 0)
#define EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#define পরবর্তী_3() (p->পরবর্তী++)
#define NEXT2() (p->পরবর্তী += 2)
#define NEXTn(n) (p->পরবর্তী += (n))
#define GETNEXT() (*p->পরবর্তী++)
#define SETERROR(e) seterr(p, (e))
#define REQUIRE(co, e) (শূন্য)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define ঢোকাও_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#অন্যথা
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
স্থির পূর্ণসংখ্যা never = 0; /* for use in asserts; shuts lint up */
#অন্যথা
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
পূর্ণসংখ্যা /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ধ্রুবক অক্ষর *pattern, পূর্ণসংখ্যা cflags) {
  কাঠামো parse pa;
  কাঠামো re_guts *g;
  কাঠামো parse *p = &pa;
  পূর্ণসংখ্যা i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#অন্যথা
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  যদি ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ফেরত (REG_INVARG);

  যদি (cflags & REG_PEND) {
    যদি (preg->re_endp < pattern)
      ফেরত (REG_INVARG);
    len = preg->re_endp - pattern;
  } অন্যথা {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (কাঠামো re_guts *)malloc(আকার(কাঠামো re_guts) +
                               (NC - 1) * আকার(cat_t));
  যদি (g == ফাঁকা)
    ফেরত (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, আকার(sop));
  p->slen = 0;
  যদি (p->strip == ফাঁকা) {
    মুক্ত((অক্ষর *)g);
    ফেরত (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->পরবর্তী = pattern;
  p->end = p->পরবর্তী + len;
  p->error = 0;
  p->ncsalloc = 0;
  জন্য (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ফাঁকা;
  g->setbits = ফাঁকা;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ফাঁকা;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (শূন্য)memset((অক্ষর *)g->catspace, 0, NC * আকার(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  যদি (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  অন্যথা যদি (cflags & REG_NOSPEC)
    p_str(p);
  অন্যথা
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
  যদি (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  যদি (p->error != 0) /* lose */
    llvm_regfree(preg);
  ফেরত (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
স্থির শূন্য p_ere(কাঠামো parse *p,
                  পূর্ণসংখ্যা stop) /* character this ERE should end at */
{
  অক্ষর c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  পূর্ণসংখ্যা first = 1; /* is this the first alternative? */

  জন্য (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    যখন (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    যদি (!EAT('|'))
      বিরতি; /* NOTE BREAK OUT */

    যদি (first) {
      ঢোকাও_3(OCH_, conc); /* offset is wrong */
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

  যদি (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  প্রতিজ্ঞা(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
স্থির শূন্য p_ere_exp(কাঠামো parse *p) {
  অক্ষর c;
  sopno pos;
  পূর্ণসংখ্যা গুনতি;
  পূর্ণসংখ্যা count2;
  পূর্ণসংখ্যা backrefnum;
  sopno subno;
  পূর্ণসংখ্যা wascaret = 0;

  প্রতিজ্ঞা(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  নির্বাচন (c) {
  ক্ষেত্র '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    যদি (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    যদি (!SEE(')'))
      p_ere(p, ')');
    যদি (subno < NPAREN) {
      p->pend[subno] = HERE();
      প্রতিজ্ঞা(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    বিরতি;
#ifndef POSIX_MISTAKE
  ক্ষেত্র ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    বিরতি;
#endif
  ক্ষেত্র '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    বিরতি;
  ক্ষেত্র '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    বিরতি;
  ক্ষেত্র '|':
    SETERROR(REG_EMPTY);
    বিরতি;
  ক্ষেত্র '*':
  ক্ষেত্র '+':
  ক্ষেত্র '?':
    SETERROR(REG_BADRPT);
    বিরতি;
  ক্ষেত্র '.':
    যদি (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    অন্যথা
      EMIT(OANY, 0);
    বিরতি;
  ক্ষেত্র '[':
    p_bracket(p);
    বিরতি;
  ক্ষেত্র '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    যদি (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } অন্যথা যদি (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      যখন (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } অন্যথা যদি (c == 'n') {
      ordinary(p, '\n');
      বিরতি;
    } অন্যথা যদি (c == 't') {
      ordinary(p, '\t');
      বিরতি;
    } অন্যথা যদি (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      অক্ষর hexstr[3] = {0};
      অক্ষর *hexp;
      পূর্ণসংখ্যা val;
      যদি (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        যদি (*hexp == '\0') {
          NEXT2();
          uncased(p, (অক্ষর)val);
          বিরতি;
        }
      }
      ordinary(p, 'x');
      বিরতি;
    } অন্যথা {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      বিরতি;
    }

    যদি (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      বিরতি;
    }

    যদি (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      বিরতি;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    প্রতিজ্ঞা(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    প্রতিজ্ঞা(p->pbegin[backrefnum] != 0);
    প্রতিজ্ঞা(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    প্রতিজ্ঞা(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (শূন্য)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    বিরতি;
  ক্ষেত্র '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  পূর্বনির্ধারিত:
    ordinary(p, c);
    বিরতি;
  }

  যদি (!MORE())
    ফেরত;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  যদি (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ফেরত; /* no repetition, we're done */
  পরবর্তী_3();

  REQUIRE(!wascaret, REG_BADRPT);
  নির্বাচন (c) {
  ক্ষেত্র '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ঢোকাও_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ঢোকাও_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    বিরতি;
  ক্ষেত্র '+':
    ঢোকাও_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    বিরতি;
  ক্ষেত্র '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ঢোকাও_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    বিরতি;
  ক্ষেত্র '{':
    গুনতি = p_গুনতি(p);
    যদি (EAT(',')) {
      যদি (isdigit((uch)PEEK())) {
        count2 = p_গুনতি(p);
        REQUIRE(গুনতি <= count2, REG_BADBR);
      } অন্যথা /* single number with comma */
        count2 = REGINFINITY;
    } অন্যথা /* just a single number */
      count2 = গুনতি;
    repeat(p, pos, গুনতি, count2);
    যদি (!EAT('}')) { /* error heuristics */
      যখন (MORE() && PEEK() != '}')
        পরবর্তী_3();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    বিরতি;
  }

  যদি (!MORE())
    ফেরত;
  c = PEEK();
  যদি (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ফেরত;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
স্থির শূন্য p_str(কাঠামো parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  যখন (MORE())
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
স্থির শূন্য p_bre(কাঠামো parse *p, পূর্ণসংখ্যা end1, /* first terminating character */
                  পূর্ণসংখ্যা end2)                  /* second terminating character */
{
  sopno start = HERE();
  পূর্ণসংখ্যা first = 1; /* first subexpression? */
  পূর্ণসংখ্যা wasdollar = 0;

  যদি (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  যখন (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  যদি (wasdollar) { /* oops, that was a trailing anchor */
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
স্থির পূর্ণসংখ্যা /* was the simple RE an unbackslashed $? */
p_simp_re(কাঠামো parse *p,
          পূর্ণসংখ্যা starordinary) /* is a leading * an ordinary character? */
{
  পূর্ণসংখ্যা c;
  পূর্ণসংখ্যা গুনতি;
  পূর্ণসংখ্যা count2;
  sopno pos;
  পূর্ণসংখ্যা i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  প্রতিজ্ঞা(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  যদি (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  নির্বাচন (c) {
  ক্ষেত্র '.':
    যদি (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    অন্যথা
      EMIT(OANY, 0);
    বিরতি;
  ক্ষেত্র '[':
    p_bracket(p);
    বিরতি;
  ক্ষেত্র BACKSL | '{':
    SETERROR(REG_BADRPT);
    বিরতি;
  ক্ষেত্র BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    যদি (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    যদি (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    যদি (subno < NPAREN) {
      p->pend[subno] = HERE();
      প্রতিজ্ঞা(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    বিরতি;
  ক্ষেত্র BACKSL | ')': /* should not get here -- must be user */
  ক্ষেত্র BACKSL | '}':
    SETERROR(REG_EPAREN);
    বিরতি;
  ক্ষেত্র BACKSL | '1':
  ক্ষেত্র BACKSL | '2':
  ক্ষেত্র BACKSL | '3':
  ক্ষেত্র BACKSL | '4':
  ক্ষেত্র BACKSL | '5':
  ক্ষেত্র BACKSL | '6':
  ক্ষেত্র BACKSL | '7':
  ক্ষেত্র BACKSL | '8':
  ক্ষেত্র BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    প্রতিজ্ঞা(i < NPAREN);
    যদি (p->pend[i] != 0) {
      প্রতিজ্ঞা(i <= p->g->nsub);
      EMIT(OBACK_, i);
      প্রতিজ্ঞা(p->pbegin[i] != 0);
      প্রতিজ্ঞা(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      প্রতিজ্ঞা(OP(p->strip[p->pend[i]]) == ORPAREN);
      (শূন্য)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } অন্যথা {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    বিরতি;
  ক্ষেত্র '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  পূর্বনির্ধারিত:
    ordinary(p, (অক্ষর)c);
    বিরতি;
  }

  যদি (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ঢোকাও_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ঢোকাও_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } অন্যথা যদি (EATTWO('\\', '{')) {
    গুনতি = p_গুনতি(p);
    যদি (EAT(',')) {
      যদি (MORE() && isdigit((uch)PEEK())) {
        count2 = p_গুনতি(p);
        REQUIRE(গুনতি <= count2, REG_BADBR);
      } অন্যথা /* single number with comma */
        count2 = REGINFINITY;
    } অন্যথা /* just a single number */
      count2 = গুনতি;
    repeat(p, pos, গুনতি, count2);
    যদি (!EATTWO('\\', '}')) { /* error heuristics */
      যখন (MORE() && !SEETWO('\\', '}'))
        পরবর্তী_3();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } অন্যথা যদি (c == '$') /* $ (but not \$) ends it */
    ফেরত (1);

  ফেরত (0);
}

/*
 - p_count - parse a repetition count
 */
স্থির পূর্ণসংখ্যা /* the value */
p_গুনতি(কাঠামো parse *p) {
  পূর্ণসংখ্যা গুনতি = 0;
  পূর্ণসংখ্যা ndigits = 0;

  যখন (MORE() && isdigit((uch)PEEK()) && গুনতি <= DUPMAX) {
    গুনতি = গুনতি * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && গুনতি <= DUPMAX, REG_BADBR);
  ফেরত (গুনতি);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
স্থির শূন্য p_bracket(কাঠামো parse *p) {
  cset *cs;
  পূর্ণসংখ্যা invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  যদি (p->end - p->পরবর্তী > 5) {
    যদি (strncmp(p->পরবর্তী, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ফেরত;
    }
    যদি (strncmp(p->পরবর্তী, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ফেরত;
    }
  }

  যদি ((cs = allocset(p)) == ফাঁকা) {
    /* allocset did set error status in p */
    ফেরত;
  }

  যদি (EAT('^'))
    invert++; /* make note to invert set at end */
  যদি (EAT(']'))
    CHadd(cs, ']');
  অন্যথা যদি (EAT('-'))
    CHadd(cs, '-');
  যখন (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  যদি (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  যদি (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ফেরত;
  }

  যদি (p->g->cflags & REG_ICASE) {
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা ci;

    জন্য (i = p->g->csetsize - 1; i >= 0; i--)
      যদি (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        যদি (ci != i)
          CHadd(cs, ci);
      }
    যদি (cs->multis != ফাঁকা)
      mccase(p, cs);
  }
  যদি (invert) {
    পূর্ণসংখ্যা i;

    জন্য (i = p->g->csetsize - 1; i >= 0; i--)
      যদি (CHIN(cs, i))
        CHsub(cs, i);
      অন্যথা
        CHadd(cs, i);
    যদি (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    যদি (cs->multis != ফাঁকা)
      mcinvert(p, cs);
  }

  প্রতিজ্ঞা(cs->multis == ফাঁকা); /* xxx */

  যদি (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } অন্যথা {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
স্থির শূন্য p_b_term(কাঠামো parse *p, cset *cs) {
  অক্ষর c;
  অক্ষর start, finish;
  পূর্ণসংখ্যা i;

  /* classify what we've got */
  নির্বাচন ((MORE()) ? PEEK() : '\0') {
  ক্ষেত্র '[':
    c = (MORE2()) ? PEEK2() : '\0';
    বিরতি;
  ক্ষেত্র '-':
    SETERROR(REG_ERANGE);
    ফেরত; /* NOTE RETURN */
    বিরতি;
  পূর্বনির্ধারিত:
    c = '\0';
    বিরতি;
  }

  নির্বাচন (c) {
  ক্ষেত্র ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    বিরতি;
  ক্ষেত্র '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    বিরতি;
  পূর্বনির্ধারিত: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_প্রতীক(p);
    যদি (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      পরবর্তী_3();
      যদি (EAT('-'))
        finish = '-';
      অন্যথা
        finish = p_b_প্রতীক(p);
    } অন্যথা {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    জন্য (i = start; i <= finish; i++)
      CHadd(cs, i);
    বিরতি;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
স্থির শূন্য p_b_cclass(কাঠামো parse *p, cset *cs) {
  ধ্রুবক অক্ষর *sp = p->পরবর্তী;
  কাঠামো cclass *cp;
  size_t len;
  ধ্রুবক অক্ষর *u;
  অক্ষর c;

  যখন (MORE() && isalpha((uch)PEEK()))
    পরবর্তী_3();
  len = p->পরবর্তী - sp;
  জন্য (cp = cclasses; cp->name != ফাঁকা; cp++)
    যদি (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      বিরতি;
  যদি (cp->name == ফাঁকা) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ফেরত;
  }

  u = cp->chars;
  যখন ((c = *u++) != '\0')
    CHadd(cs, c);
  জন্য (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
স্থির শূন্য p_b_eclass(কাঠামো parse *p, cset *cs) {
  অক্ষর c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
স্থির অক্ষর /* value of symbol */
p_b_প্রতীক(কাঠামো parse *p) {
  অক্ষর মান_2;

  REQUIRE(MORE(), REG_EBRACK);
  যদি (!EATTWO('[', '.'))
    ফেরত (GETNEXT());

  /* collating symbol */
  মান_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ফেরত (মান_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
স্থির অক্ষর                              /* value of collating element */
p_b_coll_elem(কাঠামো parse *p, পূর্ণসংখ্যা endc) /* name ended by endc,']' */
{
  ধ্রুবক অক্ষর *sp = p->পরবর্তী;
  কাঠামো cname *cp;
  size_t len;

  যখন (MORE() && !SEETWO(endc, ']'))
    পরবর্তী_3();
  যদি (!MORE()) {
    SETERROR(REG_EBRACK);
    ফেরত (0);
  }
  len = p->পরবর্তী - sp;
  জন্য (cp = cnames; cp->name != ফাঁকা; cp++)
    যদি (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      ফেরত (cp->code); /* known name */
  যদি (len == 1)
    ফেরত (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ফেরত (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
স্থির অক্ষর /* if no counterpart, return ch */
othercase(পূর্ণসংখ্যা ch) {
  ch = (uch)ch;
  প্রতিজ্ঞা(isalpha(ch));
  যদি (isupper(ch))
    ফেরত ((uch)tolower(ch));
  অন্যথা যদি (islower(ch))
    ফেরত ((uch)toupper(ch));
  অন্যথা /* peculiar, but could happen */
    ফেরত (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
স্থির শূন্য bothcases(কাঠামো parse *p, পূর্ণসংখ্যা ch) {
  ধ্রুবক অক্ষর *oldnext = p->পরবর্তী;
  ধ্রুবক অক্ষর *oldend = p->end;
  অক্ষর bracket[3];

  ch = (uch)ch;
  প্রতিজ্ঞা(othercase(ch) != ch); /* p_bracket() would recurse */
  p->পরবর্তী = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  প্রতিজ্ঞা(p->পরবর্তী == bracket + 2);
  p->পরবর্তী = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
স্থির শূন্য uncased(কাঠামো parse *p, পূর্ণসংখ্যা ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  যদি (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
স্থির শূন্য ordinary(কাঠামো parse *p, পূর্ণসংখ্যা ch) {
  যদি ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  অন্যথা
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
স্থির শূন্য nonnewline(কাঠামো parse *p) {
  ধ্রুবক অক্ষর *oldnext = p->পরবর্তী;
  ধ্রুবক অক্ষর *oldend = p->end;
  স্থির ধ্রুবক অক্ষর bracket[4] = {'^', '\n', ']', '\0'};

  p->পরবর্তী = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  প্রতিজ্ঞা(p->পরবর্তী == bracket + 3);
  p->পরবর্তী = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
স্থির শূন্য repeat(কাঠামো parse *p,
                   sopno start, /* operand from here to end of strip */
                   পূর্ণসংখ্যা from,    /* repeated from this number */
                   পূর্ণসংখ্যা to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  যদি (p->error != 0) /* head off possible runaway recursion */
    ফেরত;

  প্রতিজ্ঞা(from <= to);

  নির্বাচন (REP(MAP(from), MAP(to))) {
  ক্ষেত্র REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    বিরতি;
  ক্ষেত্র REP(0, 1):   /* as x{1,1}? */
  ক্ষেত্র REP(0, N):   /* as x{1,n}? */
  ক্ষেত্র REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ঢোকাও_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    বিরতি;
  ক্ষেত্র REP(1, 1): /* trivial case */
    /* done */
    বিরতি;
  ক্ষেত্র REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ঢোকাও_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    প্রতিজ্ঞা(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    বিরতি;
  ক্ষেত্র REP(1, INF): /* as x+ */
    ঢোকাও_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    বিরতি;
  ক্ষেত্র REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    বিরতি;
  ক্ষেত্র REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    বিরতি;
  পূর্বনির্ধারিত:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    বিরতি;
  }
}

/*
 - seterr - set an error condition
 */
স্থির পূর্ণসংখ্যা /* useless but makes type checking happy */
seterr(কাঠামো parse *p, পূর্ণসংখ্যা e) {
  যদি (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->পরবর্তী = nuls; /* try to bring things to a halt */
  p->end = nuls;
  ফেরত (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
স্থির cset *allocset(কাঠামো parse *p) {
  পূর্ণসংখ্যা no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  পূর্ণসংখ্যা i;

  যদি (no >= p->ncsalloc) { /* need another column of space */
    শূন্য *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    যদি (nc > SIZE_MAX / আকার(cset))
      যাও nomem;
    প্রতিজ্ঞা(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((অক্ষর *)p->g->sets, nc * আকার(cset));
    যদি (ptr == ফাঁকা)
      যাও nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((অক্ষর *)p->g->setbits, nbytes);
    যদি (ptr == ফাঁকা)
      যাও nomem;
    p->g->setbits = ptr;

    জন্য (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (শূন্য)memset((অক্ষর *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  যদি (p->g->sets == ফাঁকা || p->g->setbits == ফাঁকা)
    যাও nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ফাঁকা;

  ফেরত (cs);
nomem:
  মুক্ত(p->g->sets);
  p->g->sets = ফাঁকা;
  মুক্ত(p->g->setbits);
  p->g->setbits = ফাঁকা;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ফেরত (ফাঁকা);
}

/*
 - freeset - free a now-unused set
 */
স্থির শূন্য freeset(কাঠামো parse *p, cset *cs) {
  size_t i;
  cset *শীর্ষ_4 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  জন্য (i = 0; i < css; i++)
    CHsub(cs, i);
  যদি (cs == শীর্ষ_4 - 1) /* recover only the easy case */
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
স্থির পূর্ণসংখ্যা /* set number */
freezeset(কাঠামো parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *শীর্ষ_4 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  জন্য (cs2 = &p->g->sets[0]; cs2 < শীর্ষ_4; cs2++)
    যদি (cs2->hash == h && cs2 != cs) {
      /* maybe */
      জন্য (i = 0; i < css; i++)
        যদি (!!CHIN(cs2, i) != !!CHIN(cs, i))
          বিরতি; /* no */
      যদি (i == css)
        বিরতি; /* yes */
    }

  যদি (cs2 < শীর্ষ_4) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ফেরত ((পূর্ণসংখ্যা)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
স্থির পূর্ণসংখ্যা /* character; there is no "none" value */
firstch(কাঠামো parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  জন্য (i = 0; i < css; i++)
    যদি (CHIN(cs, i))
      ফেরত ((অক্ষর)i);
  প্রতিজ্ঞা(never);
  ফেরত (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
স্থির পূর্ণসংখ্যা nch(কাঠামো parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  পূর্ণসংখ্যা n = 0;

  জন্য (i = 0; i < css; i++)
    যদি (CHIN(cs, i))
      n++;
  ফেরত (n);
}

/*
 - mcadd - add a collating element to a cset
 */
স্থির শূন্য mcadd(কাঠামো parse *p, cset *cs, ধ্রুবক অক্ষর *cp) {
  size_t oldend = cs->smultis;
  শূন্য *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  যদি (np == ফাঁকা) {
    যদি (cs->multis)
      মুক্ত(cs->multis);
    cs->multis = ফাঁকা;
    SETERROR(REG_ESPACE);
    ফেরত;
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
স্থির শূন্য mcinvert(কাঠামো parse *p, cset *cs) {
  প্রতিজ্ঞা(cs->multis == ফাঁকা); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
স্থির শূন্য mccase(কাঠামো parse *p, cset *cs) {
  প্রতিজ্ঞা(cs->multis == ফাঁকা); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
স্থির পূর্ণসংখ্যা /* predicate */
isinsets(কাঠামো re_guts *g, পূর্ণসংখ্যা c) {
  uch *col;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  জন্য (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    যদি (col[uc] != 0)
      ফেরত (1);
  ফেরত (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
স্থির পূর্ণসংখ্যা /* predicate */
samesets(কাঠামো re_guts *g, পূর্ণসংখ্যা c1, পূর্ণসংখ্যা c2) {
  uch *col;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  জন্য (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    যদি (col[uc1] != col[uc2])
      ফেরত (0);
  ফেরত (1);
}

/*
 - categorize - sort out character categories
 */
স্থির শূন্য categorize(কাঠামো parse *p, কাঠামো re_guts *g) {
  cat_t *cats = g->categories;
  পূর্ণসংখ্যা c;
  পূর্ণসংখ্যা c2;
  cat_t cat;

  /* avoid making error situations worse */
  যদি (p->error != 0)
    ফেরত;

  জন্য (c = CHAR_MIN; c <= CHAR_MAX; c++)
    যদি (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      জন্য (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        যদি (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
স্থির sopno                       /* start of duplicate */
dupl(কাঠামো parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  প্রতিজ্ঞা(finish >= start);
  যদি (len == 0)
    ফেরত (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  প্রতিজ্ঞা(p->ssize >= p->slen + len);
  (শূন্য)memmove((অক্ষর *)(p->strip + p->slen), (অক্ষর *)(p->strip + start),
                (size_t)len * আকার(sop));
  p->slen += len;
  ফেরত (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
স্থির শূন্য doemit(কাঠামো parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  যদি (p->error != 0)
    ফেরত;

  /* deal with oversize operands ("can't happen", more or less) */
  প্রতিজ্ঞা(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  যদি (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  প্রতিজ্ঞা(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
স্থির শূন্য doinsert(কাঠামো parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  পূর্ণসংখ্যা i;

  /* avoid making error situations worse */
  যদি (p->error != 0)
    ফেরত;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  প্রতিজ্ঞা(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  প্রতিজ্ঞা(pos > 0);
  জন্য (i = 1; i < NPAREN; i++) {
    যদি (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    যদি (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((অক্ষর *)&p->strip[pos + 1], (অক্ষর *)&p->strip[pos],
          (HERE() - pos - 1) * আকার(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
স্থির শূন্য dofwd(কাঠামো parse *p, sopno pos, sop মান_2) {
  /* avoid making error situations worse */
  যদি (p->error != 0)
    ফেরত;

  প্রতিজ্ঞা(মান_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | মান_2;
}

/*
 - enlarge - enlarge the strip
 */
স্থির শূন্য enlarge(কাঠামো parse *p, sopno আকার_3) {
  sop *sp;

  যদি (p->ssize >= আকার_3)
    ফেরত;

  যদি ((uintptr_t)আকার_3 > SIZE_MAX / আকার(sop)) {
    SETERROR(REG_ESPACE);
    ফেরত;
  }

  sp = (sop *)realloc(p->strip, আকার_3 * আকার(sop));
  যদি (sp == ফাঁকা) {
    SETERROR(REG_ESPACE);
    ফেরত;
  }
  p->strip = sp;
  p->ssize = আকার_3;
}

/*
 - stripsnug - compact the strip
 */
স্থির শূন্য stripsnug(কাঠামো parse *p, কাঠামো re_guts *g) {
  g->nstates = p->slen;
  যদি ((uintptr_t)p->slen > SIZE_MAX / আকার(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ফেরত;
  }

  g->strip = (sop *)realloc((অক্ষর *)p->strip, p->slen * আকার(sop));
  যদি (g->strip == ফাঁকা) {
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
স্থির শূন্য findmust(কাঠামো parse *p, কাঠামো re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  অক্ষর *cp;
  sopno i;

  /* avoid making error situations worse */
  যদি (p->error != 0)
    ফেরত;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  করো {
    s = *scan++;
    নির্বাচন (OP(s)) {
    ক্ষেত্র OCHAR:        /* sequence member */
      যদি (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      বিরতি;
    ক্ষেত্র OPLUS_: /* things that don't break one */
    ক্ষেত্র OLPAREN:
    ক্ষেত্র ORPAREN:
      বিরতি;
    ক্ষেত্র OQUEST_: /* things that must be skipped */
    ক্ষেত্র OCH_:
      scan--;
      করো {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        যদি (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ফেরত;
        }
      } যখন (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    পূর্বনির্ধারিত:                  /* things that break a sequence */
      যদি (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      বিরতি;
    }
  } যখন (OP(s) != OEND);

  যদি (g->mlen == 0) /* there isn't one */
    ফেরত;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  যদি (g->must == ফাঁকা) { /* argh; just forget it */
    g->mlen = 0;
    ফেরত;
  }
  cp = g->must;
  scan = start;
  জন্য (i = g->mlen; i > 0; i--) {
    যখন (OP(s = *scan++) != OCHAR)
      চালু;
    প্রতিজ্ঞা(cp < g->must + g->mlen);
    *cp++ = (অক্ষর)OPND(s);
  }
  প্রতিজ্ঞা(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
স্থির sopno /* nesting depth */
pluscount(কাঠামো parse *p, কাঠামো re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  যদি (p->error != 0)
    ফেরত (0); /* there may not be an OEND */

  scan = g->strip + 1;
  করো {
    s = *scan++;
    নির্বাচন (OP(s)) {
    ক্ষেত্র OPLUS_:
      plusnest++;
      বিরতি;
    ক্ষেত্র O_PLUS:
      যদি (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      বিরতি;
    }
  } যখন (OP(s) != OEND);
  যদি (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ফেরত (maxnest);
}
