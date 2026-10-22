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

#ضمن "regex_impl.h"
#ضمن <ctype.h>
#ضمن <limits.h>
#ضمن <stdint.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/types.h>

#ضمن "regex2.h"
#ضمن "regutils.h"

#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"

/* character-class table */
ساكن بنية cclass {
  ثابت حرف *name;
  ثابت حرف *chars;
  ثابت حرف *multis;
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
    {عدم, 0, ""}};

/* character-name table */
ساكن بنية cname {
  ثابت حرف *name;
  حرف code;
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
              {عدم, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
بنية parse {
  ثابت حرف *next; /* next character in RE */
  ثابت حرف *end;  /* end of string (-> NUL normally) */
  صحيح error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  صحيح ncsalloc;     /* number of csets allocated */
  بنية re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ساكن فراغ p_ere(بنية parse *, صحيح);
ساكن فراغ p_ere_exp(بنية parse *);
ساكن فراغ p_str(بنية parse *);
ساكن فراغ p_bre(بنية parse *, صحيح, صحيح);
ساكن صحيح p_simp_re(بنية parse *, صحيح);
ساكن صحيح p_count(بنية parse *);
ساكن فراغ p_bracket(بنية parse *);
ساكن فراغ p_b_term(بنية parse *, cset *);
ساكن فراغ p_b_cclass(بنية parse *, cset *);
ساكن فراغ p_b_eclass(بنية parse *, cset *);
ساكن حرف p_b_symbol(بنية parse *);
ساكن حرف p_b_coll_elem(بنية parse *, صحيح);
ساكن حرف othercase(صحيح);
ساكن فراغ bothcases(بنية parse *, صحيح);
ساكن فراغ uncased(بنية parse *, صحيح);
ساكن فراغ ordinary(بنية parse *, صحيح);
ساكن فراغ nonnewline(بنية parse *);
ساكن فراغ repeat(بنية parse *, sopno, صحيح, صحيح);
ساكن صحيح seterr(بنية parse *, صحيح);
ساكن cset *allocset(بنية parse *);
ساكن فراغ freeset(بنية parse *, cset *);
ساكن صحيح freezeset(بنية parse *, cset *);
ساكن صحيح firstch(بنية parse *, cset *);
ساكن صحيح nch(بنية parse *, cset *);
ساكن فراغ mcadd(بنية parse *, cset *, ثابت حرف *);
ساكن فراغ mcinvert(بنية parse *, cset *);
ساكن فراغ mccase(بنية parse *, cset *);
ساكن صحيح isinsets(بنية re_guts *, صحيح);
ساكن صحيح samesets(بنية re_guts *, صحيح, صحيح);
ساكن فراغ categorize(بنية parse *, بنية re_guts *);
ساكن sopno dupl(بنية parse *, sopno, sopno);
ساكن فراغ doemit(بنية parse *, sop, size_t);
ساكن فراغ doinsert(بنية parse *, sop, size_t, sopno);
ساكن فراغ dofwd(بنية parse *, sopno, sop);
ساكن فراغ enlarge(بنية parse *, sopno);
ساكن فراغ stripsnug(بنية parse *, بنية re_guts *);
ساكن فراغ findmust(بنية parse *, بنية re_guts *);
ساكن sopno pluscount(بنية parse *, بنية re_guts *);

ساكن حرف nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (فراغ)((co) || SETERROR(e))
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
#ܐܚܪܢܐ
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
ساكن صحيح never = 0; /* for use in asserts; shuts lint up */
#ܐܚܪܢܐ
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
صحيح /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ثابت حرف *pattern, صحيح cflags) {
  بنية parse pa;
  بنية re_guts *g;
  بنية parse *p = &pa;
  صحيح i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#ܐܚܪܢܐ
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  ܐܢ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    ܦܢܐ (REG_INVARG);

  ܐܢ (cflags & REG_PEND) {
    ܐܢ (preg->re_endp < pattern)
      ܦܢܐ (REG_INVARG);
    len = preg->re_endp - pattern;
  } ܐܚܪܢܐ {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (بنية re_guts *)malloc(حجم(بنية re_guts) +
                               (NC - 1) * حجم(cat_t));
  ܐܢ (g == عدم)
    ܦܢܐ (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, حجم(sop));
  p->slen = 0;
  ܐܢ (p->strip == عدم) {
    حرر((حرف *)g);
    ܦܢܐ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  لكل (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = عدم;
  g->setbits = عدم;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = عدم;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (فراغ)memset((حرف *)g->catspace, 0, NC * حجم(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ܐܢ (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  ܐܚܪܢܐ ܐܢ (cflags & REG_NOSPEC)
    p_str(p);
  ܐܚܪܢܐ
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
  ܐܢ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  ܐܢ (p->error != 0) /* lose */
    llvm_regfree(preg);
  ܦܢܐ (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ساكن فراغ p_ere(بنية parse *p,
                  صحيح stop) /* character this ERE should end at */
{
  حرف c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  صحيح first = 1; /* is this the first alternative? */

  لكل (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    ܟܡܐ (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    ܐܢ (!EAT('|'))
      اكسر; /* NOTE BREAK OUT */

    ܐܢ (first) {
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

  ܐܢ (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  أكد(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ساكن فراغ p_ere_exp(بنية parse *p) {
  حرف c;
  sopno pos;
  صحيح count;
  صحيح count2;
  صحيح backrefnum;
  sopno subno;
  صحيح wascaret = 0;

  أكد(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  بدل (c) {
  حالة '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ܐܢ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    ܐܢ (!SEE(')'))
      p_ere(p, ')');
    ܐܢ (subno < NPAREN) {
      p->pend[subno] = HERE();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    اكسر;
#ifndef POSIX_MISTAKE
  حالة ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    اكسر;
#endif
  حالة '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    اكسر;
  حالة '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    اكسر;
  حالة '|':
    SETERROR(REG_EMPTY);
    اكسر;
  حالة '*':
  حالة '+':
  حالة '?':
    SETERROR(REG_BADRPT);
    اكسر;
  حالة '.':
    ܐܢ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ܐܚܪܢܐ
      EMIT(OANY, 0);
    اكسر;
  حالة '[':
    p_bracket(p);
    اكسر;
  حالة '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    ܐܢ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ܐܚܪܢܐ ܐܢ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      ܟܡܐ (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ܐܚܪܢܐ ܐܢ (c == 'n') {
      ordinary(p, '\n');
      اكسر;
    } ܐܚܪܢܐ ܐܢ (c == 't') {
      ordinary(p, '\t');
      اكسر;
    } ܐܚܪܢܐ ܐܢ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      حرف hexstr[3] = {0};
      حرف *hexp;
      صحيح val;
      ܐܢ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ܐܢ (*hexp == '\0') {
          NEXT2();
          uncased(p, (حرف)val);
          اكسر;
        }
      }
      ordinary(p, 'x');
      اكسر;
    } ܐܚܪܢܐ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      اكسر;
    }

    ܐܢ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      اكسر;
    }

    ܐܢ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      اكسر;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    أكد(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    أكد(p->pbegin[backrefnum] != 0);
    أكد(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    أكد(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (فراغ)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    اكسر;
  حالة '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  افتراضي:
    ordinary(p, c);
    اكسر;
  }

  ܐܢ (!MORE())
    ܦܢܐ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ܐܢ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ܦܢܐ; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  بدل (c) {
  حالة '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    INSERT(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    اكسر;
  حالة '+':
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    اكسر;
  حالة '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    اكسر;
  حالة '{':
    count = p_count(p);
    ܐܢ (EAT(',')) {
      ܐܢ (isdigit((uch)PEEK())) {
        count2 = p_count(p);
        REQUIRE(count <= count2, REG_BADBR);
      } ܐܚܪܢܐ /* single number with comma */
        count2 = REGINFINITY;
    } ܐܚܪܢܐ /* just a single number */
      count2 = count;
    repeat(p, pos, count, count2);
    ܐܢ (!EAT('}')) { /* error heuristics */
      ܟܡܐ (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    اكسر;
  }

  ܐܢ (!MORE())
    ܦܢܐ;
  c = PEEK();
  ܐܢ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    ܦܢܐ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ساكن فراغ p_str(بنية parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  ܟܡܐ (MORE())
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
ساكن فراغ p_bre(بنية parse *p, صحيح end1, /* first terminating character */
                  صحيح end2)                  /* second terminating character */
{
  sopno start = HERE();
  صحيح first = 1; /* first subexpression? */
  صحيح wasdollar = 0;

  ܐܢ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  ܟܡܐ (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  ܐܢ (wasdollar) { /* oops, that was a trailing anchor */
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
ساكن صحيح /* was the simple RE an unbackslashed $? */
p_simp_re(بنية parse *p,
          صحيح starordinary) /* is a leading * an ordinary character? */
{
  صحيح c;
  صحيح count;
  صحيح count2;
  sopno pos;
  صحيح i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  أكد(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  ܐܢ (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  بدل (c) {
  حالة '.':
    ܐܢ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ܐܚܪܢܐ
      EMIT(OANY, 0);
    اكسر;
  حالة '[':
    p_bracket(p);
    اكسر;
  حالة BACKSL | '{':
    SETERROR(REG_BADRPT);
    اكسر;
  حالة BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ܐܢ (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ܐܢ (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ܐܢ (subno < NPAREN) {
      p->pend[subno] = HERE();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    اكسر;
  حالة BACKSL | ')': /* should not get here -- must be user */
  حالة BACKSL | '}':
    SETERROR(REG_EPAREN);
    اكسر;
  حالة BACKSL | '1':
  حالة BACKSL | '2':
  حالة BACKSL | '3':
  حالة BACKSL | '4':
  حالة BACKSL | '5':
  حالة BACKSL | '6':
  حالة BACKSL | '7':
  حالة BACKSL | '8':
  حالة BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    أكد(i < NPAREN);
    ܐܢ (p->pend[i] != 0) {
      أكد(i <= p->g->nsub);
      EMIT(OBACK_, i);
      أكد(p->pbegin[i] != 0);
      أكد(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      أكد(OP(p->strip[p->pend[i]]) == ORPAREN);
      (فراغ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } ܐܚܪܢܐ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    اكسر;
  حالة '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  افتراضي:
    ordinary(p, (حرف)c);
    اكسر;
  }

  ܐܢ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    INSERT(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    INSERT(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ܐܚܪܢܐ ܐܢ (EATTWO('\\', '{')) {
    count = p_count(p);
    ܐܢ (EAT(',')) {
      ܐܢ (MORE() && isdigit((uch)PEEK())) {
        count2 = p_count(p);
        REQUIRE(count <= count2, REG_BADBR);
      } ܐܚܪܢܐ /* single number with comma */
        count2 = REGINFINITY;
    } ܐܚܪܢܐ /* just a single number */
      count2 = count;
    repeat(p, pos, count, count2);
    ܐܢ (!EATTWO('\\', '}')) { /* error heuristics */
      ܟܡܐ (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ܐܚܪܢܐ ܐܢ (c == '$') /* $ (but not \$) ends it */
    ܦܢܐ (1);

  ܦܢܐ (0);
}

/*
 - p_count - parse a repetition count
 */
ساكن صحيح /* the value */
p_count(بنية parse *p) {
  صحيح count = 0;
  صحيح ndigits = 0;

  ܟܡܐ (MORE() && isdigit((uch)PEEK()) && count <= DUPMAX) {
    count = count * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && count <= DUPMAX, REG_BADBR);
  ܦܢܐ (count);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ساكن فراغ p_bracket(بنية parse *p) {
  cset *cs;
  صحيح invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ܐܢ (p->end - p->next > 5) {
    ܐܢ (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      ܦܢܐ;
    }
    ܐܢ (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      ܦܢܐ;
    }
  }

  ܐܢ ((cs = allocset(p)) == عدم) {
    /* allocset did set error status in p */
    ܦܢܐ;
  }

  ܐܢ (EAT('^'))
    invert++; /* make note to invert set at end */
  ܐܢ (EAT(']'))
    CHadd(cs, ']');
  ܐܚܪܢܐ ܐܢ (EAT('-'))
    CHadd(cs, '-');
  ܟܡܐ (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  ܐܢ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ܐܢ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    ܦܢܐ;
  }

  ܐܢ (p->g->cflags & REG_ICASE) {
    صحيح i;
    صحيح ci;

    لكل (i = p->g->csetsize - 1; i >= 0; i--)
      ܐܢ (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        ܐܢ (ci != i)
          CHadd(cs, ci);
      }
    ܐܢ (cs->multis != عدم)
      mccase(p, cs);
  }
  ܐܢ (invert) {
    صحيح i;

    لكل (i = p->g->csetsize - 1; i >= 0; i--)
      ܐܢ (CHIN(cs, i))
        CHsub(cs, i);
      ܐܚܪܢܐ
        CHadd(cs, i);
    ܐܢ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ܐܢ (cs->multis != عدم)
      mcinvert(p, cs);
  }

  أكد(cs->multis == عدم); /* xxx */

  ܐܢ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ܐܚܪܢܐ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ساكن فراغ p_b_term(بنية parse *p, cset *cs) {
  حرف c;
  حرف start, finish;
  صحيح i;

  /* classify what we've got */
  بدل ((MORE()) ? PEEK() : '\0') {
  حالة '[':
    c = (MORE2()) ? PEEK2() : '\0';
    اكسر;
  حالة '-':
    SETERROR(REG_ERANGE);
    ܦܢܐ; /* NOTE RETURN */
    اكسر;
  افتراضي:
    c = '\0';
    اكسر;
  }

  بدل (c) {
  حالة ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    اكسر;
  حالة '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    اكسر;
  افتراضي: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_symbol(p);
    ܐܢ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      ܐܢ (EAT('-'))
        finish = '-';
      ܐܚܪܢܐ
        finish = p_b_symbol(p);
    } ܐܚܪܢܐ {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    لكل (i = start; i <= finish; i++)
      CHadd(cs, i);
    اكسر;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ساكن فراغ p_b_cclass(بنية parse *p, cset *cs) {
  ثابت حرف *sp = p->next;
  بنية cclass *cp;
  size_t len;
  ثابت حرف *u;
  حرف c;

  ܟܡܐ (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  لكل (cp = cclasses; cp->name != عدم; cp++)
    ܐܢ (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      اكسر;
  ܐܢ (cp->name == عدم) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    ܦܢܐ;
  }

  u = cp->chars;
  ܟܡܐ ((c = *u++) != '\0')
    CHadd(cs, c);
  لكل (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ساكن فراغ p_b_eclass(بنية parse *p, cset *cs) {
  حرف c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ساكن حرف /* value of symbol */
p_b_symbol(بنية parse *p) {
  حرف value;

  REQUIRE(MORE(), REG_EBRACK);
  ܐܢ (!EATTWO('[', '.'))
    ܦܢܐ (GETNEXT());

  /* collating symbol */
  value = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  ܦܢܐ (value);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ساكن حرف                              /* value of collating element */
p_b_coll_elem(بنية parse *p, صحيح endc) /* name ended by endc,']' */
{
  ثابت حرف *sp = p->next;
  بنية cname *cp;
  size_t len;

  ܟܡܐ (MORE() && !SEETWO(endc, ']'))
    NEXT();
  ܐܢ (!MORE()) {
    SETERROR(REG_EBRACK);
    ܦܢܐ (0);
  }
  len = p->next - sp;
  لكل (cp = cnames; cp->name != عدم; cp++)
    ܐܢ (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      ܦܢܐ (cp->code); /* known name */
  ܐܢ (len == 1)
    ܦܢܐ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  ܦܢܐ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ساكن حرف /* if no counterpart, return ch */
othercase(صحيح ch) {
  ch = (uch)ch;
  أكد(isalpha(ch));
  ܐܢ (isupper(ch))
    ܦܢܐ ((uch)tolower(ch));
  ܐܚܪܢܐ ܐܢ (islower(ch))
    ܦܢܐ ((uch)toupper(ch));
  ܐܚܪܢܐ /* peculiar, but could happen */
    ܦܢܐ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ساكن فراغ bothcases(بنية parse *p, صحيح ch) {
  ثابت حرف *oldnext = p->next;
  ثابت حرف *oldend = p->end;
  حرف bracket[3];

  ch = (uch)ch;
  أكد(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  أكد(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ساكن فراغ uncased(بنية parse *p, صحيح ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ܐܢ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ساكن فراغ ordinary(بنية parse *p, صحيح ch) {
  ܐܢ ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ܐܚܪܢܐ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ساكن فراغ nonnewline(بنية parse *p) {
  ثابت حرف *oldnext = p->next;
  ثابت حرف *oldend = p->end;
  ساكن ثابت حرف bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  أكد(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ساكن فراغ repeat(بنية parse *p,
                   sopno start, /* operand from here to end of strip */
                   صحيح from,    /* repeated from this number */
                   صحيح to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ܐܢ (p->error != 0) /* head off possible runaway recursion */
    ܦܢܐ;

  أكد(from <= to);

  بدل (REP(MAP(from), MAP(to))) {
  حالة REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    اكسر;
  حالة REP(0, 1):   /* as x{1,1}? */
  حالة REP(0, N):   /* as x{1,n}? */
  حالة REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    اكسر;
  حالة REP(1, 1): /* trivial case */
    /* done */
    اكسر;
  حالة REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    INSERT(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    أكد(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    اكسر;
  حالة REP(1, INF): /* as x+ */
    INSERT(OPLUS_, start);
    ASTERN(O_PLUS, start);
    اكسر;
  حالة REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    اكسر;
  حالة REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    اكسر;
  افتراضي:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    اكسر;
  }
}

/*
 - seterr - set an error condition
 */
ساكن صحيح /* useless but makes type checking happy */
seterr(بنية parse *p, صحيح e) {
  ܐܢ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  ܦܢܐ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ساكن cset *allocset(بنية parse *p) {
  صحيح no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  صحيح i;

  ܐܢ (no >= p->ncsalloc) { /* need another column of space */
    فراغ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ܐܢ (nc > SIZE_MAX / حجم(cset))
      اذهب nomem;
    أكد(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((حرف *)p->g->sets, nc * حجم(cset));
    ܐܢ (ptr == عدم)
      اذهب nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((حرف *)p->g->setbits, nbytes);
    ܐܢ (ptr == عدم)
      اذهب nomem;
    p->g->setbits = ptr;

    لكل (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (فراغ)memset((حرف *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ܐܢ (p->g->sets == عدم || p->g->setbits == عدم)
    اذهب nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = عدم;

  ܦܢܐ (cs);
nomem:
  حرر(p->g->sets);
  p->g->sets = عدم;
  حرر(p->g->setbits);
  p->g->setbits = عدم;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  ܦܢܐ (عدم);
}

/*
 - freeset - free a now-unused set
 */
ساكن فراغ freeset(بنية parse *p, cset *cs) {
  size_t i;
  cset *top = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  لكل (i = 0; i < css; i++)
    CHsub(cs, i);
  ܐܢ (cs == top - 1) /* recover only the easy case */
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
ساكن صحيح /* set number */
freezeset(بنية parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *top = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  لكل (cs2 = &p->g->sets[0]; cs2 < top; cs2++)
    ܐܢ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      لكل (i = 0; i < css; i++)
        ܐܢ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          اكسر; /* no */
      ܐܢ (i == css)
        اكسر; /* yes */
    }

  ܐܢ (cs2 < top) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  ܦܢܐ ((صحيح)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ساكن صحيح /* character; there is no "none" value */
firstch(بنية parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  لكل (i = 0; i < css; i++)
    ܐܢ (CHIN(cs, i))
      ܦܢܐ ((حرف)i);
  أكد(never);
  ܦܢܐ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ساكن صحيح nch(بنية parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  صحيح n = 0;

  لكل (i = 0; i < css; i++)
    ܐܢ (CHIN(cs, i))
      n++;
  ܦܢܐ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ساكن فراغ mcadd(بنية parse *p, cset *cs, ثابت حرف *cp) {
  size_t oldend = cs->smultis;
  فراغ *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ܐܢ (np == عدم) {
    ܐܢ (cs->multis)
      حرر(cs->multis);
    cs->multis = عدم;
    SETERROR(REG_ESPACE);
    ܦܢܐ;
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
ساكن فراغ mcinvert(بنية parse *p, cset *cs) {
  أكد(cs->multis == عدم); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ساكن فراغ mccase(بنية parse *p, cset *cs) {
  أكد(cs->multis == عدم); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ساكن صحيح /* predicate */
isinsets(بنية re_guts *g, صحيح c) {
  uch *col;
  صحيح i;
  صحيح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  لكل (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ܐܢ (col[uc] != 0)
      ܦܢܐ (1);
  ܦܢܐ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ساكن صحيح /* predicate */
samesets(بنية re_guts *g, صحيح c1, صحيح c2) {
  uch *col;
  صحيح i;
  صحيح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  لكل (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    ܐܢ (col[uc1] != col[uc2])
      ܦܢܐ (0);
  ܦܢܐ (1);
}

/*
 - categorize - sort out character categories
 */
ساكن فراغ categorize(بنية parse *p, بنية re_guts *g) {
  cat_t *cats = g->categories;
  صحيح c;
  صحيح c2;
  cat_t cat;

  /* avoid making error situations worse */
  ܐܢ (p->error != 0)
    ܦܢܐ;

  لكل (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ܐܢ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      لكل (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ܐܢ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ساكن sopno                       /* start of duplicate */
dupl(بنية parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  أكد(finish >= start);
  ܐܢ (len == 0)
    ܦܢܐ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  أكد(p->ssize >= p->slen + len);
  (فراغ)memmove((حرف *)(p->strip + p->slen), (حرف *)(p->strip + start),
                (size_t)len * حجم(sop));
  p->slen += len;
  ܦܢܐ (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ساكن فراغ doemit(بنية parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ܐܢ (p->error != 0)
    ܦܢܐ;

  /* deal with oversize operands ("can't happen", more or less) */
  أكد(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ܐܢ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  أكد(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ساكن فراغ doinsert(بنية parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  صحيح i;

  /* avoid making error situations worse */
  ܐܢ (p->error != 0)
    ܦܢܐ;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  أكد(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  أكد(pos > 0);
  لكل (i = 1; i < NPAREN; i++) {
    ܐܢ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ܐܢ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((حرف *)&p->strip[pos + 1], (حرف *)&p->strip[pos],
          (HERE() - pos - 1) * حجم(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ساكن فراغ dofwd(بنية parse *p, sopno pos, sop value) {
  /* avoid making error situations worse */
  ܐܢ (p->error != 0)
    ܦܢܐ;

  أكد(value < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | value;
}

/*
 - enlarge - enlarge the strip
 */
ساكن فراغ enlarge(بنية parse *p, sopno size) {
  sop *sp;

  ܐܢ (p->ssize >= size)
    ܦܢܐ;

  ܐܢ ((uintptr_t)size > SIZE_MAX / حجم(sop)) {
    SETERROR(REG_ESPACE);
    ܦܢܐ;
  }

  sp = (sop *)realloc(p->strip, size * حجم(sop));
  ܐܢ (sp == عدم) {
    SETERROR(REG_ESPACE);
    ܦܢܐ;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
ساكن فراغ stripsnug(بنية parse *p, بنية re_guts *g) {
  g->nstates = p->slen;
  ܐܢ ((uintptr_t)p->slen > SIZE_MAX / حجم(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    ܦܢܐ;
  }

  g->strip = (sop *)realloc((حرف *)p->strip, p->slen * حجم(sop));
  ܐܢ (g->strip == عدم) {
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
ساكن فراغ findmust(بنية parse *p, بنية re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  حرف *cp;
  sopno i;

  /* avoid making error situations worse */
  ܐܢ (p->error != 0)
    ܦܢܐ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  ܥܒܕ {
    s = *scan++;
    بدل (OP(s)) {
    حالة OCHAR:        /* sequence member */
      ܐܢ (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      اكسر;
    حالة OPLUS_: /* things that don't break one */
    حالة OLPAREN:
    حالة ORPAREN:
      اكسر;
    حالة OQUEST_: /* things that must be skipped */
    حالة OCH_:
      scan--;
      ܥܒܕ {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        ܐܢ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          ܦܢܐ;
        }
      } ܟܡܐ (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    افتراضي:                  /* things that break a sequence */
      ܐܢ (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      اكسر;
    }
  } ܟܡܐ (OP(s) != OEND);

  ܐܢ (g->mlen == 0) /* there isn't one */
    ܦܢܐ;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  ܐܢ (g->must == عدم) { /* argh; just forget it */
    g->mlen = 0;
    ܦܢܐ;
  }
  cp = g->must;
  scan = start;
  لكل (i = g->mlen; i > 0; i--) {
    ܟܡܐ (OP(s = *scan++) != OCHAR)
      استمر;
    أكد(cp < g->must + g->mlen);
    *cp++ = (حرف)OPND(s);
  }
  أكد(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ساكن sopno /* nesting depth */
pluscount(بنية parse *p, بنية re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ܐܢ (p->error != 0)
    ܦܢܐ (0); /* there may not be an OEND */

  scan = g->strip + 1;
  ܥܒܕ {
    s = *scan++;
    بدل (OP(s)) {
    حالة OPLUS_:
      plusnest++;
      اكسر;
    حالة O_PLUS:
      ܐܢ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      اكسر;
    }
  } ܟܡܐ (OP(s) != OEND);
  ܐܢ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  ܦܢܐ (maxnest);
}
