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

#அந்தர்பாவ "regex_impl.h"
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "regex2.h"
#அந்தர்பாவ "regutils.h"

#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"

/* character-class table */
நிலையான கட்டமைப்பு cclass {
  மாறிலி எழுத்து *name;
  மாறிலி எழுத்து *chars;
  மாறிலி எழுத்து *multis;
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
    {வெற்று, 0, ""}};

/* character-name table */
நிலையான கட்டமைப்பு cname {
  மாறிலி எழுத்து *name;
  எழுத்து code;
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
              {வெற்று, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
கட்டமைப்பு parse {
  மாறிலி எழுத்து *next; /* next character in RE */
  மாறிலி எழுத்து *end;  /* end of string (-> NUL normally) */
  முழு error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  முழு ncsalloc;     /* number of csets allocated */
  கட்டமைப்பு re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

நிலையான வெறுமை p_ere(கட்டமைப்பு parse *, முழு);
நிலையான வெறுமை p_ere_exp(கட்டமைப்பு parse *);
நிலையான வெறுமை p_str(கட்டமைப்பு parse *);
நிலையான வெறுமை p_bre(கட்டமைப்பு parse *, முழு, முழு);
நிலையான முழு p_simp_re(கட்டமைப்பு parse *, முழு);
நிலையான முழு p_மொத்தம்(கட்டமைப்பு parse *);
நிலையான வெறுமை p_bracket(கட்டமைப்பு parse *);
நிலையான வெறுமை p_b_term(கட்டமைப்பு parse *, cset *);
நிலையான வெறுமை p_b_cclass(கட்டமைப்பு parse *, cset *);
நிலையான வெறுமை p_b_eclass(கட்டமைப்பு parse *, cset *);
நிலையான எழுத்து p_b_குறியீடு(கட்டமைப்பு parse *);
நிலையான எழுத்து p_b_coll_elem(கட்டமைப்பு parse *, முழு);
நிலையான எழுத்து othercase(முழு);
நிலையான வெறுமை bothcases(கட்டமைப்பு parse *, முழு);
நிலையான வெறுமை uncased(கட்டமைப்பு parse *, முழு);
நிலையான வெறுமை ordinary(கட்டமைப்பு parse *, முழு);
நிலையான வெறுமை nonnewline(கட்டமைப்பு parse *);
நிலையான வெறுமை repeat(கட்டமைப்பு parse *, sopno, முழு, முழு);
நிலையான முழு seterr(கட்டமைப்பு parse *, முழு);
நிலையான cset *allocset(கட்டமைப்பு parse *);
நிலையான வெறுமை freeset(கட்டமைப்பு parse *, cset *);
நிலையான முழு freezeset(கட்டமைப்பு parse *, cset *);
நிலையான முழு firstch(கட்டமைப்பு parse *, cset *);
நிலையான முழு nch(கட்டமைப்பு parse *, cset *);
நிலையான வெறுமை mcadd(கட்டமைப்பு parse *, cset *, மாறிலி எழுத்து *);
நிலையான வெறுமை mcinvert(கட்டமைப்பு parse *, cset *);
நிலையான வெறுமை mccase(கட்டமைப்பு parse *, cset *);
நிலையான முழு isinsets(கட்டமைப்பு re_guts *, முழு);
நிலையான முழு samesets(கட்டமைப்பு re_guts *, முழு, முழு);
நிலையான வெறுமை categorize(கட்டமைப்பு parse *, கட்டமைப்பு re_guts *);
நிலையான sopno dupl(கட்டமைப்பு parse *, sopno, sopno);
நிலையான வெறுமை doemit(கட்டமைப்பு parse *, sop, size_t);
நிலையான வெறுமை doinsert(கட்டமைப்பு parse *, sop, size_t, sopno);
நிலையான வெறுமை dofwd(கட்டமைப்பு parse *, sopno, sop);
நிலையான வெறுமை enlarge(கட்டமைப்பு parse *, sopno);
நிலையான வெறுமை stripsnug(கட்டமைப்பு parse *, கட்டமைப்பு re_guts *);
நிலையான வெறுமை findmust(கட்டமைப்பு parse *, கட்டமைப்பு re_guts *);
நிலையான sopno pluscount(கட்டமைப்பு parse *, கட்டமைப்பு re_guts *);

நிலையான எழுத்து nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (வெறுமை)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define செருகு_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#இல்லையெனில்
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
நிலையான முழு never = 0; /* for use in asserts; shuts lint up */
#இல்லையெனில்
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
முழு /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, மாறிலி எழுத்து *pattern, முழு cflags) {
  கட்டமைப்பு parse pa;
  கட்டமைப்பு re_guts *g;
  கட்டமைப்பு parse *p = &pa;
  முழு i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#இல்லையெனில்
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  எனில் ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    திரும்பு (REG_INVARG);

  எனில் (cflags & REG_PEND) {
    எனில் (preg->re_endp < pattern)
      திரும்பு (REG_INVARG);
    len = preg->re_endp - pattern;
  } இல்லையெனில் {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (கட்டமைப்பு re_guts *)malloc(அளவு(கட்டமைப்பு re_guts) +
                               (NC - 1) * அளவு(cat_t));
  எனில் (g == வெற்று)
    திரும்பு (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, அளவு(sop));
  p->slen = 0;
  எனில் (p->strip == வெற்று) {
    விடுவி((எழுத்து *)g);
    திரும்பு (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  க்கு (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = வெற்று;
  g->setbits = வெற்று;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = வெற்று;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (வெறுமை)memset((எழுத்து *)g->catspace, 0, NC * அளவு(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  எனில் (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  இல்லையெனில் எனில் (cflags & REG_NOSPEC)
    p_str(p);
  இல்லையெனில்
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
  எனில் (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  எனில் (p->error != 0) /* lose */
    llvm_regfree(preg);
  திரும்பு (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
நிலையான வெறுமை p_ere(கட்டமைப்பு parse *p,
                  முழு stop) /* character this ERE should end at */
{
  எழுத்து c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  முழு first = 1; /* is this the first alternative? */

  க்கு (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    போது (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    எனில் (!EAT('|'))
      நிறுத்து; /* NOTE BREAK OUT */

    எனில் (first) {
      செருகு_3(OCH_, conc); /* offset is wrong */
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

  எனில் (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ப்ரதிஜ்ஞா(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
நிலையான வெறுமை p_ere_exp(கட்டமைப்பு parse *p) {
  எழுத்து c;
  sopno pos;
  முழு மொத்தம்;
  முழு count2;
  முழு backrefnum;
  sopno subno;
  முழு wascaret = 0;

  ப்ரதிஜ்ஞா(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  தேர்வு (c) {
  நிலை '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    எனில் (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    எனில் (!SEE(')'))
      p_ere(p, ')');
    எனில் (subno < NPAREN) {
      p->pend[subno] = HERE();
      ப்ரதிஜ்ஞா(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    நிறுத்து;
#ifndef POSIX_MISTAKE
  நிலை ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    நிறுத்து;
#endif
  நிலை '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    நிறுத்து;
  நிலை '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    நிறுத்து;
  நிலை '|':
    SETERROR(REG_EMPTY);
    நிறுத்து;
  நிலை '*':
  நிலை '+':
  நிலை '?':
    SETERROR(REG_BADRPT);
    நிறுத்து;
  நிலை '.':
    எனில் (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    இல்லையெனில்
      EMIT(OANY, 0);
    நிறுத்து;
  நிலை '[':
    p_bracket(p);
    நிறுத்து;
  நிலை '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    எனில் (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } இல்லையெனில் எனில் (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      போது (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } இல்லையெனில் எனில் (c == 'n') {
      ordinary(p, '\n');
      நிறுத்து;
    } இல்லையெனில் எனில் (c == 't') {
      ordinary(p, '\t');
      நிறுத்து;
    } இல்லையெனில் எனில் (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      எழுத்து hexstr[3] = {0};
      எழுத்து *hexp;
      முழு val;
      எனில் (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        எனில் (*hexp == '\0') {
          NEXT2();
          uncased(p, (எழுத்து)val);
          நிறுத்து;
        }
      }
      ordinary(p, 'x');
      நிறுத்து;
    } இல்லையெனில் {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      நிறுத்து;
    }

    எனில் (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      நிறுத்து;
    }

    எனில் (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      நிறுத்து;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    ப்ரதிஜ்ஞா(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    ப்ரதிஜ்ஞா(p->pbegin[backrefnum] != 0);
    ப்ரதிஜ்ஞா(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    ப்ரதிஜ்ஞா(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (வெறுமை)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    நிறுத்து;
  நிலை '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  இயல்பு:
    ordinary(p, c);
    நிறுத்து;
  }

  எனில் (!MORE())
    திரும்பு;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  எனில் (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    திரும்பு; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  தேர்வு (c) {
  நிலை '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    செருகு_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    செருகு_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    நிறுத்து;
  நிலை '+':
    செருகு_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    நிறுத்து;
  நிலை '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    செருகு_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    நிறுத்து;
  நிலை '{':
    மொத்தம் = p_மொத்தம்(p);
    எனில் (EAT(',')) {
      எனில் (isdigit((uch)PEEK())) {
        count2 = p_மொத்தம்(p);
        REQUIRE(மொத்தம் <= count2, REG_BADBR);
      } இல்லையெனில் /* single number with comma */
        count2 = REGINFINITY;
    } இல்லையெனில் /* just a single number */
      count2 = மொத்தம்;
    repeat(p, pos, மொத்தம், count2);
    எனில் (!EAT('}')) { /* error heuristics */
      போது (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    நிறுத்து;
  }

  எனில் (!MORE())
    திரும்பு;
  c = PEEK();
  எனில் (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    திரும்பு;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
நிலையான வெறுமை p_str(கட்டமைப்பு parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  போது (MORE())
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
நிலையான வெறுமை p_bre(கட்டமைப்பு parse *p, முழு end1, /* first terminating character */
                  முழு end2)                  /* second terminating character */
{
  sopno start = HERE();
  முழு first = 1; /* first subexpression? */
  முழு wasdollar = 0;

  எனில் (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  போது (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  எனில் (wasdollar) { /* oops, that was a trailing anchor */
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
நிலையான முழு /* was the simple RE an unbackslashed $? */
p_simp_re(கட்டமைப்பு parse *p,
          முழு starordinary) /* is a leading * an ordinary character? */
{
  முழு c;
  முழு மொத்தம்;
  முழு count2;
  sopno pos;
  முழு i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  ப்ரதிஜ்ஞா(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  எனில் (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  தேர்வு (c) {
  நிலை '.':
    எனில் (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    இல்லையெனில்
      EMIT(OANY, 0);
    நிறுத்து;
  நிலை '[':
    p_bracket(p);
    நிறுத்து;
  நிலை BACKSL | '{':
    SETERROR(REG_BADRPT);
    நிறுத்து;
  நிலை BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    எனில் (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    எனில் (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    எனில் (subno < NPAREN) {
      p->pend[subno] = HERE();
      ப்ரதிஜ்ஞா(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    நிறுத்து;
  நிலை BACKSL | ')': /* should not get here -- must be user */
  நிலை BACKSL | '}':
    SETERROR(REG_EPAREN);
    நிறுத்து;
  நிலை BACKSL | '1':
  நிலை BACKSL | '2':
  நிலை BACKSL | '3':
  நிலை BACKSL | '4':
  நிலை BACKSL | '5':
  நிலை BACKSL | '6':
  நிலை BACKSL | '7':
  நிலை BACKSL | '8':
  நிலை BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ப்ரதிஜ்ஞா(i < NPAREN);
    எனில் (p->pend[i] != 0) {
      ப்ரதிஜ்ஞா(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ப்ரதிஜ்ஞா(p->pbegin[i] != 0);
      ப்ரதிஜ்ஞா(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ப்ரதிஜ்ஞா(OP(p->strip[p->pend[i]]) == ORPAREN);
      (வெறுமை)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } இல்லையெனில் {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    நிறுத்து;
  நிலை '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  இயல்பு:
    ordinary(p, (எழுத்து)c);
    நிறுத்து;
  }

  எனில் (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    செருகு_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    செருகு_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } இல்லையெனில் எனில் (EATTWO('\\', '{')) {
    மொத்தம் = p_மொத்தம்(p);
    எனில் (EAT(',')) {
      எனில் (MORE() && isdigit((uch)PEEK())) {
        count2 = p_மொத்தம்(p);
        REQUIRE(மொத்தம் <= count2, REG_BADBR);
      } இல்லையெனில் /* single number with comma */
        count2 = REGINFINITY;
    } இல்லையெனில் /* just a single number */
      count2 = மொத்தம்;
    repeat(p, pos, மொத்தம், count2);
    எனில் (!EATTWO('\\', '}')) { /* error heuristics */
      போது (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } இல்லையெனில் எனில் (c == '$') /* $ (but not \$) ends it */
    திரும்பு (1);

  திரும்பு (0);
}

/*
 - p_count - parse a repetition count
 */
நிலையான முழு /* the value */
p_மொத்தம்(கட்டமைப்பு parse *p) {
  முழு மொத்தம் = 0;
  முழு ndigits = 0;

  போது (MORE() && isdigit((uch)PEEK()) && மொத்தம் <= DUPMAX) {
    மொத்தம் = மொத்தம் * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && மொத்தம் <= DUPMAX, REG_BADBR);
  திரும்பு (மொத்தம்);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
நிலையான வெறுமை p_bracket(கட்டமைப்பு parse *p) {
  cset *cs;
  முழு invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  எனில் (p->end - p->next > 5) {
    எனில் (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      திரும்பு;
    }
    எனில் (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      திரும்பு;
    }
  }

  எனில் ((cs = allocset(p)) == வெற்று) {
    /* allocset did set error status in p */
    திரும்பு;
  }

  எனில் (EAT('^'))
    invert++; /* make note to invert set at end */
  எனில் (EAT(']'))
    CHadd(cs, ']');
  இல்லையெனில் எனில் (EAT('-'))
    CHadd(cs, '-');
  போது (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  எனில் (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  எனில் (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    திரும்பு;
  }

  எனில் (p->g->cflags & REG_ICASE) {
    முழு i;
    முழு ci;

    க்கு (i = p->g->csetsize - 1; i >= 0; i--)
      எனில் (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        எனில் (ci != i)
          CHadd(cs, ci);
      }
    எனில் (cs->multis != வெற்று)
      mccase(p, cs);
  }
  எனில் (invert) {
    முழு i;

    க்கு (i = p->g->csetsize - 1; i >= 0; i--)
      எனில் (CHIN(cs, i))
        CHsub(cs, i);
      இல்லையெனில்
        CHadd(cs, i);
    எனில் (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    எனில் (cs->multis != வெற்று)
      mcinvert(p, cs);
  }

  ப்ரதிஜ்ஞா(cs->multis == வெற்று); /* xxx */

  எனில் (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } இல்லையெனில் {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
நிலையான வெறுமை p_b_term(கட்டமைப்பு parse *p, cset *cs) {
  எழுத்து c;
  எழுத்து start, finish;
  முழு i;

  /* classify what we've got */
  தேர்வு ((MORE()) ? PEEK() : '\0') {
  நிலை '[':
    c = (MORE2()) ? PEEK2() : '\0';
    நிறுத்து;
  நிலை '-':
    SETERROR(REG_ERANGE);
    திரும்பு; /* NOTE RETURN */
    நிறுத்து;
  இயல்பு:
    c = '\0';
    நிறுத்து;
  }

  தேர்வு (c) {
  நிலை ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    நிறுத்து;
  நிலை '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    நிறுத்து;
  இயல்பு: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_குறியீடு(p);
    எனில் (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      எனில் (EAT('-'))
        finish = '-';
      இல்லையெனில்
        finish = p_b_குறியீடு(p);
    } இல்லையெனில் {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    க்கு (i = start; i <= finish; i++)
      CHadd(cs, i);
    நிறுத்து;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
நிலையான வெறுமை p_b_cclass(கட்டமைப்பு parse *p, cset *cs) {
  மாறிலி எழுத்து *sp = p->next;
  கட்டமைப்பு cclass *cp;
  size_t len;
  மாறிலி எழுத்து *u;
  எழுத்து c;

  போது (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  க்கு (cp = cclasses; cp->name != வெற்று; cp++)
    எனில் (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      நிறுத்து;
  எனில் (cp->name == வெற்று) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    திரும்பு;
  }

  u = cp->chars;
  போது ((c = *u++) != '\0')
    CHadd(cs, c);
  க்கு (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
நிலையான வெறுமை p_b_eclass(கட்டமைப்பு parse *p, cset *cs) {
  எழுத்து c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
நிலையான எழுத்து /* value of symbol */
p_b_குறியீடு(கட்டமைப்பு parse *p) {
  எழுத்து மதிப்பு_2;

  REQUIRE(MORE(), REG_EBRACK);
  எனில் (!EATTWO('[', '.'))
    திரும்பு (GETNEXT());

  /* collating symbol */
  மதிப்பு_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  திரும்பு (மதிப்பு_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
நிலையான எழுத்து                              /* value of collating element */
p_b_coll_elem(கட்டமைப்பு parse *p, முழு endc) /* name ended by endc,']' */
{
  மாறிலி எழுத்து *sp = p->next;
  கட்டமைப்பு cname *cp;
  size_t len;

  போது (MORE() && !SEETWO(endc, ']'))
    NEXT();
  எனில் (!MORE()) {
    SETERROR(REG_EBRACK);
    திரும்பு (0);
  }
  len = p->next - sp;
  க்கு (cp = cnames; cp->name != வெற்று; cp++)
    எனில் (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      திரும்பு (cp->code); /* known name */
  எனில் (len == 1)
    திரும்பு (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  திரும்பு (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
நிலையான எழுத்து /* if no counterpart, return ch */
othercase(முழு ch) {
  ch = (uch)ch;
  ப்ரதிஜ்ஞா(isalpha(ch));
  எனில் (isupper(ch))
    திரும்பு ((uch)tolower(ch));
  இல்லையெனில் எனில் (islower(ch))
    திரும்பு ((uch)toupper(ch));
  இல்லையெனில் /* peculiar, but could happen */
    திரும்பு (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
நிலையான வெறுமை bothcases(கட்டமைப்பு parse *p, முழு ch) {
  மாறிலி எழுத்து *oldnext = p->next;
  மாறிலி எழுத்து *oldend = p->end;
  எழுத்து bracket[3];

  ch = (uch)ch;
  ப்ரதிஜ்ஞா(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ப்ரதிஜ்ஞா(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
நிலையான வெறுமை uncased(கட்டமைப்பு parse *p, முழு ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  எனில் (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
நிலையான வெறுமை ordinary(கட்டமைப்பு parse *p, முழு ch) {
  எனில் ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  இல்லையெனில்
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
நிலையான வெறுமை nonnewline(கட்டமைப்பு parse *p) {
  மாறிலி எழுத்து *oldnext = p->next;
  மாறிலி எழுத்து *oldend = p->end;
  நிலையான மாறிலி எழுத்து bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  ப்ரதிஜ்ஞா(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
நிலையான வெறுமை repeat(கட்டமைப்பு parse *p,
                   sopno start, /* operand from here to end of strip */
                   முழு from,    /* repeated from this number */
                   முழு to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  எனில் (p->error != 0) /* head off possible runaway recursion */
    திரும்பு;

  ப்ரதிஜ்ஞா(from <= to);

  தேர்வு (REP(MAP(from), MAP(to))) {
  நிலை REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    நிறுத்து;
  நிலை REP(0, 1):   /* as x{1,1}? */
  நிலை REP(0, N):   /* as x{1,n}? */
  நிலை REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    செருகு_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    நிறுத்து;
  நிலை REP(1, 1): /* trivial case */
    /* done */
    நிறுத்து;
  நிலை REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    செருகு_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    ப்ரதிஜ்ஞா(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    நிறுத்து;
  நிலை REP(1, INF): /* as x+ */
    செருகு_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    நிறுத்து;
  நிலை REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    நிறுத்து;
  நிலை REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    நிறுத்து;
  இயல்பு:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    நிறுத்து;
  }
}

/*
 - seterr - set an error condition
 */
நிலையான முழு /* useless but makes type checking happy */
seterr(கட்டமைப்பு parse *p, முழு e) {
  எனில் (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  திரும்பு (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
நிலையான cset *allocset(கட்டமைப்பு parse *p) {
  முழு no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  முழு i;

  எனில் (no >= p->ncsalloc) { /* need another column of space */
    வெறுமை *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    எனில் (nc > SIZE_MAX / அளவு(cset))
      செல் nomem;
    ப்ரதிஜ்ஞா(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((எழுத்து *)p->g->sets, nc * அளவு(cset));
    எனில் (ptr == வெற்று)
      செல் nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((எழுத்து *)p->g->setbits, nbytes);
    எனில் (ptr == வெற்று)
      செல் nomem;
    p->g->setbits = ptr;

    க்கு (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (வெறுமை)memset((எழுத்து *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  எனில் (p->g->sets == வெற்று || p->g->setbits == வெற்று)
    செல் nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = வெற்று;

  திரும்பு (cs);
nomem:
  விடுவி(p->g->sets);
  p->g->sets = வெற்று;
  விடுவி(p->g->setbits);
  p->g->setbits = வெற்று;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  திரும்பு (வெற்று);
}

/*
 - freeset - free a now-unused set
 */
நிலையான வெறுமை freeset(கட்டமைப்பு parse *p, cset *cs) {
  size_t i;
  cset *உச்சி_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  க்கு (i = 0; i < css; i++)
    CHsub(cs, i);
  எனில் (cs == உச்சி_2 - 1) /* recover only the easy case */
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
நிலையான முழு /* set number */
freezeset(கட்டமைப்பு parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *உச்சி_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  க்கு (cs2 = &p->g->sets[0]; cs2 < உச்சி_2; cs2++)
    எனில் (cs2->hash == h && cs2 != cs) {
      /* maybe */
      க்கு (i = 0; i < css; i++)
        எனில் (!!CHIN(cs2, i) != !!CHIN(cs, i))
          நிறுத்து; /* no */
      எனில் (i == css)
        நிறுத்து; /* yes */
    }

  எனில் (cs2 < உச்சி_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  திரும்பு ((முழு)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
நிலையான முழு /* character; there is no "none" value */
firstch(கட்டமைப்பு parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  க்கு (i = 0; i < css; i++)
    எனில் (CHIN(cs, i))
      திரும்பு ((எழுத்து)i);
  ப்ரதிஜ்ஞா(never);
  திரும்பு (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
நிலையான முழு nch(கட்டமைப்பு parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  முழு n = 0;

  க்கு (i = 0; i < css; i++)
    எனில் (CHIN(cs, i))
      n++;
  திரும்பு (n);
}

/*
 - mcadd - add a collating element to a cset
 */
நிலையான வெறுமை mcadd(கட்டமைப்பு parse *p, cset *cs, மாறிலி எழுத்து *cp) {
  size_t oldend = cs->smultis;
  வெறுமை *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  எனில் (np == வெற்று) {
    எனில் (cs->multis)
      விடுவி(cs->multis);
    cs->multis = வெற்று;
    SETERROR(REG_ESPACE);
    திரும்பு;
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
நிலையான வெறுமை mcinvert(கட்டமைப்பு parse *p, cset *cs) {
  ப்ரதிஜ்ஞா(cs->multis == வெற்று); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
நிலையான வெறுமை mccase(கட்டமைப்பு parse *p, cset *cs) {
  ப்ரதிஜ்ஞா(cs->multis == வெற்று); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
நிலையான முழு /* predicate */
isinsets(கட்டமைப்பு re_guts *g, முழு c) {
  uch *col;
  முழு i;
  முழு ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  க்கு (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    எனில் (col[uc] != 0)
      திரும்பு (1);
  திரும்பு (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
நிலையான முழு /* predicate */
samesets(கட்டமைப்பு re_guts *g, முழு c1, முழு c2) {
  uch *col;
  முழு i;
  முழு ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  க்கு (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    எனில் (col[uc1] != col[uc2])
      திரும்பு (0);
  திரும்பு (1);
}

/*
 - categorize - sort out character categories
 */
நிலையான வெறுமை categorize(கட்டமைப்பு parse *p, கட்டமைப்பு re_guts *g) {
  cat_t *cats = g->categories;
  முழு c;
  முழு c2;
  cat_t cat;

  /* avoid making error situations worse */
  எனில் (p->error != 0)
    திரும்பு;

  க்கு (c = CHAR_MIN; c <= CHAR_MAX; c++)
    எனில் (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      க்கு (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        எனில் (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
நிலையான sopno                       /* start of duplicate */
dupl(கட்டமைப்பு parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  ப்ரதிஜ்ஞா(finish >= start);
  எனில் (len == 0)
    திரும்பு (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ப்ரதிஜ்ஞா(p->ssize >= p->slen + len);
  (வெறுமை)memmove((எழுத்து *)(p->strip + p->slen), (எழுத்து *)(p->strip + start),
                (size_t)len * அளவு(sop));
  p->slen += len;
  திரும்பு (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
நிலையான வெறுமை doemit(கட்டமைப்பு parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  எனில் (p->error != 0)
    திரும்பு;

  /* deal with oversize operands ("can't happen", more or less) */
  ப்ரதிஜ்ஞா(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  எனில் (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ப்ரதிஜ்ஞா(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
நிலையான வெறுமை doinsert(கட்டமைப்பு parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  முழு i;

  /* avoid making error situations worse */
  எனில் (p->error != 0)
    திரும்பு;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  ப்ரதிஜ்ஞா(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ப்ரதிஜ்ஞா(pos > 0);
  க்கு (i = 1; i < NPAREN; i++) {
    எனில் (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    எனில் (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((எழுத்து *)&p->strip[pos + 1], (எழுத்து *)&p->strip[pos],
          (HERE() - pos - 1) * அளவு(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
நிலையான வெறுமை dofwd(கட்டமைப்பு parse *p, sopno pos, sop மதிப்பு_2) {
  /* avoid making error situations worse */
  எனில் (p->error != 0)
    திரும்பு;

  ப்ரதிஜ்ஞா(மதிப்பு_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | மதிப்பு_2;
}

/*
 - enlarge - enlarge the strip
 */
நிலையான வெறுமை enlarge(கட்டமைப்பு parse *p, sopno size) {
  sop *sp;

  எனில் (p->ssize >= size)
    திரும்பு;

  எனில் ((uintptr_t)size > SIZE_MAX / அளவு(sop)) {
    SETERROR(REG_ESPACE);
    திரும்பு;
  }

  sp = (sop *)realloc(p->strip, size * அளவு(sop));
  எனில் (sp == வெற்று) {
    SETERROR(REG_ESPACE);
    திரும்பு;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
நிலையான வெறுமை stripsnug(கட்டமைப்பு parse *p, கட்டமைப்பு re_guts *g) {
  g->nstates = p->slen;
  எனில் ((uintptr_t)p->slen > SIZE_MAX / அளவு(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    திரும்பு;
  }

  g->strip = (sop *)realloc((எழுத்து *)p->strip, p->slen * அளவு(sop));
  எனில் (g->strip == வெற்று) {
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
நிலையான வெறுமை findmust(கட்டமைப்பு parse *p, கட்டமைப்பு re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  எழுத்து *cp;
  sopno i;

  /* avoid making error situations worse */
  எனில் (p->error != 0)
    திரும்பு;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  செய் {
    s = *scan++;
    தேர்வு (OP(s)) {
    நிலை OCHAR:        /* sequence member */
      எனில் (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      நிறுத்து;
    நிலை OPLUS_: /* things that don't break one */
    நிலை OLPAREN:
    நிலை ORPAREN:
      நிறுத்து;
    நிலை OQUEST_: /* things that must be skipped */
    நிலை OCH_:
      scan--;
      செய் {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        எனில் (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          திரும்பு;
        }
      } போது (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    இயல்பு:                  /* things that break a sequence */
      எனில் (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      நிறுத்து;
    }
  } போது (OP(s) != OEND);

  எனில் (g->mlen == 0) /* there isn't one */
    திரும்பு;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  எனில் (g->must == வெற்று) { /* argh; just forget it */
    g->mlen = 0;
    திரும்பு;
  }
  cp = g->must;
  scan = start;
  க்கு (i = g->mlen; i > 0; i--) {
    போது (OP(s = *scan++) != OCHAR)
      தொடர்;
    ப்ரதிஜ்ஞா(cp < g->must + g->mlen);
    *cp++ = (எழுத்து)OPND(s);
  }
  ப்ரதிஜ்ஞா(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
நிலையான sopno /* nesting depth */
pluscount(கட்டமைப்பு parse *p, கட்டமைப்பு re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  எனில் (p->error != 0)
    திரும்பு (0); /* there may not be an OEND */

  scan = g->strip + 1;
  செய் {
    s = *scan++;
    தேர்வு (OP(s)) {
    நிலை OPLUS_:
      plusnest++;
      நிறுத்து;
    நிலை O_PLUS:
      எனில் (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      நிறுத்து;
    }
  } போது (OP(s) != OEND);
  எனில் (plusnest != 0)
    g->iflags |= REGEX_BAD;
  திரும்பு (maxnest);
}
