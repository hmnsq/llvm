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

#شامل "regex_impl.h"
#شامل <ctype.h>
#شامل <limits.h>
#شامل <stdint.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/types.h>

#شامل "regex2.h"
#شامل "regutils.h"

#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"

/* character-class table */
ایستا ساختار cclass {
  ثابت نویسه *name;
  ثابت نویسه *chars;
  ثابت نویسه *multis;
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
    {پوچ, 0, ""}};

/* character-name table */
ایستا ساختار cname {
  ثابت نویسه *name;
  نویسه code;
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
              {پوچ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ساختار parse {
  ثابت نویسه *next; /* next character in RE */
  ثابت نویسه *end;  /* end of string (-> NUL normally) */
  صحیح error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  صحیح ncsalloc;     /* number of csets allocated */
  ساختار re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ایستا تهی p_ere(ساختار parse *, صحیح);
ایستا تهی p_ere_exp(ساختار parse *);
ایستا تهی p_str(ساختار parse *);
ایستا تهی p_bre(ساختار parse *, صحیح, صحیح);
ایستا صحیح p_simp_re(ساختار parse *, صحیح);
ایستا صحیح p_شمار(ساختار parse *);
ایستا تهی p_bracket(ساختار parse *);
ایستا تهی p_b_term(ساختار parse *, cset *);
ایستا تهی p_b_cclass(ساختار parse *, cset *);
ایستا تهی p_b_eclass(ساختار parse *, cset *);
ایستا نویسه p_b_نماد(ساختار parse *);
ایستا نویسه p_b_coll_elem(ساختار parse *, صحیح);
ایستا نویسه othercase(صحیح);
ایستا تهی bothcases(ساختار parse *, صحیح);
ایستا تهی uncased(ساختار parse *, صحیح);
ایستا تهی ordinary(ساختار parse *, صحیح);
ایستا تهی nonnewline(ساختار parse *);
ایستا تهی repeat(ساختار parse *, sopno, صحیح, صحیح);
ایستا صحیح seterr(ساختار parse *, صحیح);
ایستا cset *allocset(ساختار parse *);
ایستا تهی freeset(ساختار parse *, cset *);
ایستا صحیح freezeset(ساختار parse *, cset *);
ایستا صحیح firstch(ساختار parse *, cset *);
ایستا صحیح nch(ساختار parse *, cset *);
ایستا تهی mcadd(ساختار parse *, cset *, ثابت نویسه *);
ایستا تهی mcinvert(ساختار parse *, cset *);
ایستا تهی mccase(ساختار parse *, cset *);
ایستا صحیح isinsets(ساختار re_guts *, صحیح);
ایستا صحیح samesets(ساختار re_guts *, صحیح, صحیح);
ایستا تهی categorize(ساختار parse *, ساختار re_guts *);
ایستا sopno dupl(ساختار parse *, sopno, sopno);
ایستا تهی doemit(ساختار parse *, sop, size_t);
ایستا تهی doinsert(ساختار parse *, sop, size_t, sopno);
ایستا تهی dofwd(ساختار parse *, sopno, sop);
ایستا تهی enlarge(ساختار parse *, sopno);
ایستا تهی stripsnug(ساختار parse *, ساختار re_guts *);
ایستا تهی findmust(ساختار parse *, ساختار re_guts *);
ایستا sopno pluscount(ساختار parse *, ساختار re_guts *);

ایستا نویسه nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (تهی)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define درج_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#وگرنه
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
ایستا صحیح never = 0; /* for use in asserts; shuts lint up */
#وگرنه
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
صحیح /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ثابت نویسه *pattern, صحیح cflags) {
  ساختار parse pa;
  ساختار re_guts *g;
  ساختار parse *p = &pa;
  صحیح i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#وگرنه
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  اگر ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    بازگشت (REG_INVARG);

  اگر (cflags & REG_PEND) {
    اگر (preg->re_endp < pattern)
      بازگشت (REG_INVARG);
    len = preg->re_endp - pattern;
  } وگرنه {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ساختار re_guts *)malloc(اندازه(ساختار re_guts) +
                               (NC - 1) * اندازه(cat_t));
  اگر (g == پوچ)
    بازگشت (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, اندازه(sop));
  p->slen = 0;
  اگر (p->strip == پوچ) {
    آزاد((نویسه *)g);
    بازگشت (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  برای (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = پوچ;
  g->setbits = پوچ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = پوچ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (تهی)memset((نویسه *)g->catspace, 0, NC * اندازه(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  اگر (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  وگرنه اگر (cflags & REG_NOSPEC)
    p_str(p);
  وگرنه
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
  اگر (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  اگر (p->error != 0) /* lose */
    llvm_regfree(preg);
  بازگشت (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ایستا تهی p_ere(ساختار parse *p,
                  صحیح stop) /* character this ERE should end at */
{
  نویسه c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  صحیح first = 1; /* is this the first alternative? */

  برای (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    وقتی (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    اگر (!EAT('|'))
      بشکن; /* NOTE BREAK OUT */

    اگر (first) {
      درج_3(OCH_, conc); /* offset is wrong */
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

  اگر (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  أكد(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ایستا تهی p_ere_exp(ساختار parse *p) {
  نویسه c;
  sopno pos;
  صحیح شمار;
  صحیح count2;
  صحیح backrefnum;
  sopno subno;
  صحیح wascaret = 0;

  أكد(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  انتخاب (c) {
  حالت '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    اگر (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    اگر (!SEE(')'))
      p_ere(p, ')');
    اگر (subno < NPAREN) {
      p->pend[subno] = HERE();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    بشکن;
#ifndef POSIX_MISTAKE
  حالت ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    بشکن;
#endif
  حالت '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    بشکن;
  حالت '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    بشکن;
  حالت '|':
    SETERROR(REG_EMPTY);
    بشکن;
  حالت '*':
  حالت '+':
  حالت '?':
    SETERROR(REG_BADRPT);
    بشکن;
  حالت '.':
    اگر (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    وگرنه
      EMIT(OANY, 0);
    بشکن;
  حالت '[':
    p_bracket(p);
    بشکن;
  حالت '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    اگر (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } وگرنه اگر (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      وقتی (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } وگرنه اگر (c == 'n') {
      ordinary(p, '\n');
      بشکن;
    } وگرنه اگر (c == 't') {
      ordinary(p, '\t');
      بشکن;
    } وگرنه اگر (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      نویسه hexstr[3] = {0};
      نویسه *hexp;
      صحیح val;
      اگر (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        اگر (*hexp == '\0') {
          NEXT2();
          uncased(p, (نویسه)val);
          بشکن;
        }
      }
      ordinary(p, 'x');
      بشکن;
    } وگرنه {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      بشکن;
    }

    اگر (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      بشکن;
    }

    اگر (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      بشکن;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    أكد(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    أكد(p->pbegin[backrefnum] != 0);
    أكد(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    أكد(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (تهی)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    بشکن;
  حالت '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  پیشفرض:
    ordinary(p, c);
    بشکن;
  }

  اگر (!MORE())
    بازگشت;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  اگر (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    بازگشت; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  انتخاب (c) {
  حالت '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    درج_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    درج_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    بشکن;
  حالت '+':
    درج_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    بشکن;
  حالت '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    درج_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    بشکن;
  حالت '{':
    شمار = p_شمار(p);
    اگر (EAT(',')) {
      اگر (isdigit((uch)PEEK())) {
        count2 = p_شمار(p);
        REQUIRE(شمار <= count2, REG_BADBR);
      } وگرنه /* single number with comma */
        count2 = REGINFINITY;
    } وگرنه /* just a single number */
      count2 = شمار;
    repeat(p, pos, شمار, count2);
    اگر (!EAT('}')) { /* error heuristics */
      وقتی (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    بشکن;
  }

  اگر (!MORE())
    بازگشت;
  c = PEEK();
  اگر (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    بازگشت;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ایستا تهی p_str(ساختار parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  وقتی (MORE())
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
ایستا تهی p_bre(ساختار parse *p, صحیح end1, /* first terminating character */
                  صحیح end2)                  /* second terminating character */
{
  sopno start = HERE();
  صحیح first = 1; /* first subexpression? */
  صحیح wasdollar = 0;

  اگر (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  وقتی (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  اگر (wasdollar) { /* oops, that was a trailing anchor */
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
ایستا صحیح /* was the simple RE an unbackslashed $? */
p_simp_re(ساختار parse *p,
          صحیح starordinary) /* is a leading * an ordinary character? */
{
  صحیح c;
  صحیح شمار;
  صحیح count2;
  sopno pos;
  صحیح i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  أكد(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  اگر (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  انتخاب (c) {
  حالت '.':
    اگر (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    وگرنه
      EMIT(OANY, 0);
    بشکن;
  حالت '[':
    p_bracket(p);
    بشکن;
  حالت BACKSL | '{':
    SETERROR(REG_BADRPT);
    بشکن;
  حالت BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    اگر (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    اگر (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    اگر (subno < NPAREN) {
      p->pend[subno] = HERE();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    بشکن;
  حالت BACKSL | ')': /* should not get here -- must be user */
  حالت BACKSL | '}':
    SETERROR(REG_EPAREN);
    بشکن;
  حالت BACKSL | '1':
  حالت BACKSL | '2':
  حالت BACKSL | '3':
  حالت BACKSL | '4':
  حالت BACKSL | '5':
  حالت BACKSL | '6':
  حالت BACKSL | '7':
  حالت BACKSL | '8':
  حالت BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    أكد(i < NPAREN);
    اگر (p->pend[i] != 0) {
      أكد(i <= p->g->nsub);
      EMIT(OBACK_, i);
      أكد(p->pbegin[i] != 0);
      أكد(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      أكد(OP(p->strip[p->pend[i]]) == ORPAREN);
      (تهی)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } وگرنه {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    بشکن;
  حالت '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  پیشفرض:
    ordinary(p, (نویسه)c);
    بشکن;
  }

  اگر (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    درج_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    درج_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } وگرنه اگر (EATTWO('\\', '{')) {
    شمار = p_شمار(p);
    اگر (EAT(',')) {
      اگر (MORE() && isdigit((uch)PEEK())) {
        count2 = p_شمار(p);
        REQUIRE(شمار <= count2, REG_BADBR);
      } وگرنه /* single number with comma */
        count2 = REGINFINITY;
    } وگرنه /* just a single number */
      count2 = شمار;
    repeat(p, pos, شمار, count2);
    اگر (!EATTWO('\\', '}')) { /* error heuristics */
      وقتی (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } وگرنه اگر (c == '$') /* $ (but not \$) ends it */
    بازگشت (1);

  بازگشت (0);
}

/*
 - p_count - parse a repetition count
 */
ایستا صحیح /* the value */
p_شمار(ساختار parse *p) {
  صحیح شمار = 0;
  صحیح ndigits = 0;

  وقتی (MORE() && isdigit((uch)PEEK()) && شمار <= DUPMAX) {
    شمار = شمار * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && شمار <= DUPMAX, REG_BADBR);
  بازگشت (شمار);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ایستا تهی p_bracket(ساختار parse *p) {
  cset *cs;
  صحیح invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  اگر (p->end - p->next > 5) {
    اگر (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      بازگشت;
    }
    اگر (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      بازگشت;
    }
  }

  اگر ((cs = allocset(p)) == پوچ) {
    /* allocset did set error status in p */
    بازگشت;
  }

  اگر (EAT('^'))
    invert++; /* make note to invert set at end */
  اگر (EAT(']'))
    CHadd(cs, ']');
  وگرنه اگر (EAT('-'))
    CHadd(cs, '-');
  وقتی (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  اگر (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  اگر (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    بازگشت;
  }

  اگر (p->g->cflags & REG_ICASE) {
    صحیح i;
    صحیح ci;

    برای (i = p->g->csetsize - 1; i >= 0; i--)
      اگر (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        اگر (ci != i)
          CHadd(cs, ci);
      }
    اگر (cs->multis != پوچ)
      mccase(p, cs);
  }
  اگر (invert) {
    صحیح i;

    برای (i = p->g->csetsize - 1; i >= 0; i--)
      اگر (CHIN(cs, i))
        CHsub(cs, i);
      وگرنه
        CHadd(cs, i);
    اگر (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    اگر (cs->multis != پوچ)
      mcinvert(p, cs);
  }

  أكد(cs->multis == پوچ); /* xxx */

  اگر (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } وگرنه {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ایستا تهی p_b_term(ساختار parse *p, cset *cs) {
  نویسه c;
  نویسه start, finish;
  صحیح i;

  /* classify what we've got */
  انتخاب ((MORE()) ? PEEK() : '\0') {
  حالت '[':
    c = (MORE2()) ? PEEK2() : '\0';
    بشکن;
  حالت '-':
    SETERROR(REG_ERANGE);
    بازگشت; /* NOTE RETURN */
    بشکن;
  پیشفرض:
    c = '\0';
    بشکن;
  }

  انتخاب (c) {
  حالت ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    بشکن;
  حالت '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    بشکن;
  پیشفرض: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_نماد(p);
    اگر (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      اگر (EAT('-'))
        finish = '-';
      وگرنه
        finish = p_b_نماد(p);
    } وگرنه {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    برای (i = start; i <= finish; i++)
      CHadd(cs, i);
    بشکن;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ایستا تهی p_b_cclass(ساختار parse *p, cset *cs) {
  ثابت نویسه *sp = p->next;
  ساختار cclass *cp;
  size_t len;
  ثابت نویسه *u;
  نویسه c;

  وقتی (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  برای (cp = cclasses; cp->name != پوچ; cp++)
    اگر (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      بشکن;
  اگر (cp->name == پوچ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    بازگشت;
  }

  u = cp->chars;
  وقتی ((c = *u++) != '\0')
    CHadd(cs, c);
  برای (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ایستا تهی p_b_eclass(ساختار parse *p, cset *cs) {
  نویسه c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ایستا نویسه /* value of symbol */
p_b_نماد(ساختار parse *p) {
  نویسه مقدار_2;

  REQUIRE(MORE(), REG_EBRACK);
  اگر (!EATTWO('[', '.'))
    بازگشت (GETNEXT());

  /* collating symbol */
  مقدار_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  بازگشت (مقدار_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ایستا نویسه                              /* value of collating element */
p_b_coll_elem(ساختار parse *p, صحیح endc) /* name ended by endc,']' */
{
  ثابت نویسه *sp = p->next;
  ساختار cname *cp;
  size_t len;

  وقتی (MORE() && !SEETWO(endc, ']'))
    NEXT();
  اگر (!MORE()) {
    SETERROR(REG_EBRACK);
    بازگشت (0);
  }
  len = p->next - sp;
  برای (cp = cnames; cp->name != پوچ; cp++)
    اگر (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      بازگشت (cp->code); /* known name */
  اگر (len == 1)
    بازگشت (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  بازگشت (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ایستا نویسه /* if no counterpart, return ch */
othercase(صحیح ch) {
  ch = (uch)ch;
  أكد(isalpha(ch));
  اگر (isupper(ch))
    بازگشت ((uch)tolower(ch));
  وگرنه اگر (islower(ch))
    بازگشت ((uch)toupper(ch));
  وگرنه /* peculiar, but could happen */
    بازگشت (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ایستا تهی bothcases(ساختار parse *p, صحیح ch) {
  ثابت نویسه *oldnext = p->next;
  ثابت نویسه *oldend = p->end;
  نویسه bracket[3];

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
ایستا تهی uncased(ساختار parse *p, صحیح ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  اگر (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ایستا تهی ordinary(ساختار parse *p, صحیح ch) {
  اگر ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  وگرنه
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ایستا تهی nonnewline(ساختار parse *p) {
  ثابت نویسه *oldnext = p->next;
  ثابت نویسه *oldend = p->end;
  ایستا ثابت نویسه bracket[4] = {'^', '\n', ']', '\0'};

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
ایستا تهی repeat(ساختار parse *p,
                   sopno start, /* operand from here to end of strip */
                   صحیح from,    /* repeated from this number */
                   صحیح to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  اگر (p->error != 0) /* head off possible runaway recursion */
    بازگشت;

  أكد(from <= to);

  انتخاب (REP(MAP(from), MAP(to))) {
  حالت REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    بشکن;
  حالت REP(0, 1):   /* as x{1,1}? */
  حالت REP(0, N):   /* as x{1,n}? */
  حالت REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    درج_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    بشکن;
  حالت REP(1, 1): /* trivial case */
    /* done */
    بشکن;
  حالت REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    درج_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    أكد(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    بشکن;
  حالت REP(1, INF): /* as x+ */
    درج_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    بشکن;
  حالت REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    بشکن;
  حالت REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    بشکن;
  پیشفرض:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    بشکن;
  }
}

/*
 - seterr - set an error condition
 */
ایستا صحیح /* useless but makes type checking happy */
seterr(ساختار parse *p, صحیح e) {
  اگر (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  بازگشت (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ایستا cset *allocset(ساختار parse *p) {
  صحیح no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  صحیح i;

  اگر (no >= p->ncsalloc) { /* need another column of space */
    تهی *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    اگر (nc > SIZE_MAX / اندازه(cset))
      برو nomem;
    أكد(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((نویسه *)p->g->sets, nc * اندازه(cset));
    اگر (ptr == پوچ)
      برو nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((نویسه *)p->g->setbits, nbytes);
    اگر (ptr == پوچ)
      برو nomem;
    p->g->setbits = ptr;

    برای (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (تهی)memset((نویسه *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  اگر (p->g->sets == پوچ || p->g->setbits == پوچ)
    برو nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = پوچ;

  بازگشت (cs);
nomem:
  آزاد(p->g->sets);
  p->g->sets = پوچ;
  آزاد(p->g->setbits);
  p->g->setbits = پوچ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  بازگشت (پوچ);
}

/*
 - freeset - free a now-unused set
 */
ایستا تهی freeset(ساختار parse *p, cset *cs) {
  size_t i;
  cset *بالاترین_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  برای (i = 0; i < css; i++)
    CHsub(cs, i);
  اگر (cs == بالاترین_2 - 1) /* recover only the easy case */
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
ایستا صحیح /* set number */
freezeset(ساختار parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *بالاترین_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  برای (cs2 = &p->g->sets[0]; cs2 < بالاترین_2; cs2++)
    اگر (cs2->hash == h && cs2 != cs) {
      /* maybe */
      برای (i = 0; i < css; i++)
        اگر (!!CHIN(cs2, i) != !!CHIN(cs, i))
          بشکن; /* no */
      اگر (i == css)
        بشکن; /* yes */
    }

  اگر (cs2 < بالاترین_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  بازگشت ((صحیح)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ایستا صحیح /* character; there is no "none" value */
firstch(ساختار parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  برای (i = 0; i < css; i++)
    اگر (CHIN(cs, i))
      بازگشت ((نویسه)i);
  أكد(never);
  بازگشت (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ایستا صحیح nch(ساختار parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  صحیح n = 0;

  برای (i = 0; i < css; i++)
    اگر (CHIN(cs, i))
      n++;
  بازگشت (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ایستا تهی mcadd(ساختار parse *p, cset *cs, ثابت نویسه *cp) {
  size_t oldend = cs->smultis;
  تهی *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  اگر (np == پوچ) {
    اگر (cs->multis)
      آزاد(cs->multis);
    cs->multis = پوچ;
    SETERROR(REG_ESPACE);
    بازگشت;
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
ایستا تهی mcinvert(ساختار parse *p, cset *cs) {
  أكد(cs->multis == پوچ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ایستا تهی mccase(ساختار parse *p, cset *cs) {
  أكد(cs->multis == پوچ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ایستا صحیح /* predicate */
isinsets(ساختار re_guts *g, صحیح c) {
  uch *col;
  صحیح i;
  صحیح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  برای (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    اگر (col[uc] != 0)
      بازگشت (1);
  بازگشت (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ایستا صحیح /* predicate */
samesets(ساختار re_guts *g, صحیح c1, صحیح c2) {
  uch *col;
  صحیح i;
  صحیح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  برای (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    اگر (col[uc1] != col[uc2])
      بازگشت (0);
  بازگشت (1);
}

/*
 - categorize - sort out character categories
 */
ایستا تهی categorize(ساختار parse *p, ساختار re_guts *g) {
  cat_t *cats = g->categories;
  صحیح c;
  صحیح c2;
  cat_t cat;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    بازگشت;

  برای (c = CHAR_MIN; c <= CHAR_MAX; c++)
    اگر (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      برای (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        اگر (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ایستا sopno                       /* start of duplicate */
dupl(ساختار parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  أكد(finish >= start);
  اگر (len == 0)
    بازگشت (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  أكد(p->ssize >= p->slen + len);
  (تهی)memmove((نویسه *)(p->strip + p->slen), (نویسه *)(p->strip + start),
                (size_t)len * اندازه(sop));
  p->slen += len;
  بازگشت (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ایستا تهی doemit(ساختار parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  اگر (p->error != 0)
    بازگشت;

  /* deal with oversize operands ("can't happen", more or less) */
  أكد(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  اگر (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  أكد(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
ایستا تهی doinsert(ساختار parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  صحیح i;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    بازگشت;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  أكد(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  أكد(pos > 0);
  برای (i = 1; i < NPAREN; i++) {
    اگر (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    اگر (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((نویسه *)&p->strip[pos + 1], (نویسه *)&p->strip[pos],
          (HERE() - pos - 1) * اندازه(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ایستا تهی dofwd(ساختار parse *p, sopno pos, sop مقدار_2) {
  /* avoid making error situations worse */
  اگر (p->error != 0)
    بازگشت;

  أكد(مقدار_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | مقدار_2;
}

/*
 - enlarge - enlarge the strip
 */
ایستا تهی enlarge(ساختار parse *p, sopno size) {
  sop *sp;

  اگر (p->ssize >= size)
    بازگشت;

  اگر ((uintptr_t)size > SIZE_MAX / اندازه(sop)) {
    SETERROR(REG_ESPACE);
    بازگشت;
  }

  sp = (sop *)realloc(p->strip, size * اندازه(sop));
  اگر (sp == پوچ) {
    SETERROR(REG_ESPACE);
    بازگشت;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
ایستا تهی stripsnug(ساختار parse *p, ساختار re_guts *g) {
  g->nstates = p->slen;
  اگر ((uintptr_t)p->slen > SIZE_MAX / اندازه(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    بازگشت;
  }

  g->strip = (sop *)realloc((نویسه *)p->strip, p->slen * اندازه(sop));
  اگر (g->strip == پوچ) {
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
ایستا تهی findmust(ساختار parse *p, ساختار re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  نویسه *cp;
  sopno i;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    بازگشت;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  انجامبده {
    s = *scan++;
    انتخاب (OP(s)) {
    حالت OCHAR:        /* sequence member */
      اگر (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      بشکن;
    حالت OPLUS_: /* things that don't break one */
    حالت OLPAREN:
    حالت ORPAREN:
      بشکن;
    حالت OQUEST_: /* things that must be skipped */
    حالت OCH_:
      scan--;
      انجامبده {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        اگر (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          بازگشت;
        }
      } وقتی (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    پیشفرض:                  /* things that break a sequence */
      اگر (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      بشکن;
    }
  } وقتی (OP(s) != OEND);

  اگر (g->mlen == 0) /* there isn't one */
    بازگشت;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  اگر (g->must == پوچ) { /* argh; just forget it */
    g->mlen = 0;
    بازگشت;
  }
  cp = g->must;
  scan = start;
  برای (i = g->mlen; i > 0; i--) {
    وقتی (OP(s = *scan++) != OCHAR)
      ادامه;
    أكد(cp < g->must + g->mlen);
    *cp++ = (نویسه)OPND(s);
  }
  أكد(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ایستا sopno /* nesting depth */
pluscount(ساختار parse *p, ساختار re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  اگر (p->error != 0)
    بازگشت (0); /* there may not be an OEND */

  scan = g->strip + 1;
  انجامبده {
    s = *scan++;
    انتخاب (OP(s)) {
    حالت OPLUS_:
      plusnest++;
      بشکن;
    حالت O_PLUS:
      اگر (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      بشکن;
    }
  } وقتی (OP(s) != OEND);
  اگر (plusnest != 0)
    g->iflags |= REGEX_BAD;
  بازگشت (maxnest);
}
