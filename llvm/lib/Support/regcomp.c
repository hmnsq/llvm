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

#अन्तर्भाव "regex_impl.h"
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "regex2.h"
#अन्तर्भाव "regutils.h"

#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"

/* character-class table */
स्थिर संरचना cclass {
  ध्रुव वर्ण *name;
  ध्रुव वर्ण *chars;
  ध्रुव वर्ण *multis;
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
    {रिक्त, 0, ""}};

/* character-name table */
स्थिर संरचना cname {
  ध्रुव वर्ण *name;
  वर्ण code;
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
              {रिक्त, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
संरचना parse {
  ध्रुव वर्ण *next; /* next character in RE */
  ध्रुव वर्ण *end;  /* end of string (-> NUL normally) */
  पूर्णाङ्क error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  पूर्णाङ्क ncsalloc;     /* number of csets allocated */
  संरचना re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

स्थिर शून्य p_ere(संरचना parse *, पूर्णाङ्क);
स्थिर शून्य p_ere_exp(संरचना parse *);
स्थिर शून्य p_str(संरचना parse *);
स्थिर शून्य p_bre(संरचना parse *, पूर्णाङ्क, पूर्णाङ्क);
स्थिर पूर्णाङ्क p_simp_re(संरचना parse *, पूर्णाङ्क);
स्थिर पूर्णाङ्क p_गणनम्(संरचना parse *);
स्थिर शून्य p_bracket(संरचना parse *);
स्थिर शून्य p_b_term(संरचना parse *, cset *);
स्थिर शून्य p_b_cclass(संरचना parse *, cset *);
स्थिर शून्य p_b_eclass(संरचना parse *, cset *);
स्थिर वर्ण p_b_चिह्नम्(संरचना parse *);
स्थिर वर्ण p_b_coll_elem(संरचना parse *, पूर्णाङ्क);
स्थिर वर्ण othercase(पूर्णाङ्क);
स्थिर शून्य bothcases(संरचना parse *, पूर्णाङ्क);
स्थिर शून्य uncased(संरचना parse *, पूर्णाङ्क);
स्थिर शून्य ordinary(संरचना parse *, पूर्णाङ्क);
स्थिर शून्य nonnewline(संरचना parse *);
स्थिर शून्य repeat(संरचना parse *, sopno, पूर्णाङ्क, पूर्णाङ्क);
स्थिर पूर्णाङ्क seterr(संरचना parse *, पूर्णाङ्क);
स्थिर cset *allocset(संरचना parse *);
स्थिर शून्य freeset(संरचना parse *, cset *);
स्थिर पूर्णाङ्क freezeset(संरचना parse *, cset *);
स्थिर पूर्णाङ्क firstch(संरचना parse *, cset *);
स्थिर पूर्णाङ्क nch(संरचना parse *, cset *);
स्थिर शून्य mcadd(संरचना parse *, cset *, ध्रुव वर्ण *);
स्थिर शून्य mcinvert(संरचना parse *, cset *);
स्थिर शून्य mccase(संरचना parse *, cset *);
स्थिर पूर्णाङ्क isinsets(संरचना re_guts *, पूर्णाङ्क);
स्थिर पूर्णाङ्क samesets(संरचना re_guts *, पूर्णाङ्क, पूर्णाङ्क);
स्थिर शून्य categorize(संरचना parse *, संरचना re_guts *);
स्थिर sopno dupl(संरचना parse *, sopno, sopno);
स्थिर शून्य doemit(संरचना parse *, sop, size_t);
स्थिर शून्य doinsert(संरचना parse *, sop, size_t, sopno);
स्थिर शून्य dofwd(संरचना parse *, sopno, sop);
स्थिर शून्य enlarge(संरचना parse *, sopno);
स्थिर शून्य stripsnug(संरचना parse *, संरचना re_guts *);
स्थिर शून्य findmust(संरचना parse *, संरचना re_guts *);
स्थिर sopno pluscount(संरचना parse *, संरचना re_guts *);

स्थिर वर्ण nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (शून्य)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define निवेशय_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#अञ्ञथा
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
स्थिर पूर्णाङ्क never = 0; /* for use in asserts; shuts lint up */
#अञ्ञथा
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
पूर्णाङ्क /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ध्रुव वर्ण *pattern, पूर्णाङ्क cflags) {
  संरचना parse pa;
  संरचना re_guts *g;
  संरचना parse *p = &pa;
  पूर्णाङ्क i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#अञ्ञथा
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  सचे ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    पटिनिवत्त (REG_INVARG);

  सचे (cflags & REG_PEND) {
    सचे (preg->re_endp < pattern)
      पटिनिवत्त (REG_INVARG);
    len = preg->re_endp - pattern;
  } अञ्ञथा {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (संरचना re_guts *)malloc(परिमाण(संरचना re_guts) +
                               (NC - 1) * परिमाण(cat_t));
  सचे (g == रिक्त)
    पटिनिवत्त (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, परिमाण(sop));
  p->slen = 0;
  सचे (p->strip == रिक्त) {
    मुक्त((वर्ण *)g);
    पटिनिवत्त (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  अत्थाय (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = रिक्त;
  g->setbits = रिक्त;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = रिक्त;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (शून्य)memset((वर्ण *)g->catspace, 0, NC * परिमाण(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  सचे (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  अञ्ञथा सचे (cflags & REG_NOSPEC)
    p_str(p);
  अञ्ञथा
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
  सचे (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  सचे (p->error != 0) /* lose */
    llvm_regfree(preg);
  पटिनिवत्त (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
स्थिर शून्य p_ere(संरचना parse *p,
                  पूर्णाङ्क stop) /* character this ERE should end at */
{
  वर्ण c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  पूर्णाङ्क first = 1; /* is this the first alternative? */

  अत्थाय (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    याव (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    सचे (!EAT('|'))
      भङ्ग; /* NOTE BREAK OUT */

    सचे (first) {
      निवेशय_3(OCH_, conc); /* offset is wrong */
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

  सचे (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  प्रतिज्ञा(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
स्थिर शून्य p_ere_exp(संरचना parse *p) {
  वर्ण c;
  sopno pos;
  पूर्णाङ्क गणनम्;
  पूर्णाङ्क count2;
  पूर्णाङ्क backrefnum;
  sopno subno;
  पूर्णाङ्क wascaret = 0;

  प्रतिज्ञा(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  विकल्प (c) {
  पक्ष '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    सचे (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    सचे (!SEE(')'))
      p_ere(p, ')');
    सचे (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    भङ्ग;
#ifndef POSIX_MISTAKE
  पक्ष ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    भङ्ग;
#endif
  पक्ष '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    भङ्ग;
  पक्ष '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    भङ्ग;
  पक्ष '|':
    SETERROR(REG_EMPTY);
    भङ्ग;
  पक्ष '*':
  पक्ष '+':
  पक्ष '?':
    SETERROR(REG_BADRPT);
    भङ्ग;
  पक्ष '.':
    सचे (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अञ्ञथा
      EMIT(OANY, 0);
    भङ्ग;
  पक्ष '[':
    p_bracket(p);
    भङ्ग;
  पक्ष '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    सचे (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } अञ्ञथा सचे (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      याव (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } अञ्ञथा सचे (c == 'n') {
      ordinary(p, '\n');
      भङ्ग;
    } अञ्ञथा सचे (c == 't') {
      ordinary(p, '\t');
      भङ्ग;
    } अञ्ञथा सचे (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      वर्ण hexstr[3] = {0};
      वर्ण *hexp;
      पूर्णाङ्क val;
      सचे (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        सचे (*hexp == '\0') {
          NEXT2();
          uncased(p, (वर्ण)val);
          भङ्ग;
        }
      }
      ordinary(p, 'x');
      भङ्ग;
    } अञ्ञथा {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      भङ्ग;
    }

    सचे (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      भङ्ग;
    }

    सचे (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      भङ्ग;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    प्रतिज्ञा(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    प्रतिज्ञा(p->pbegin[backrefnum] != 0);
    प्रतिज्ञा(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    प्रतिज्ञा(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (शून्य)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    भङ्ग;
  पक्ष '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  स्वभाव:
    ordinary(p, c);
    भङ्ग;
  }

  सचे (!MORE())
    पटिनिवत्त;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  सचे (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    पटिनिवत्त; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  विकल्प (c) {
  पक्ष '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    निवेशय_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    निवेशय_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    भङ्ग;
  पक्ष '+':
    निवेशय_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    भङ्ग;
  पक्ष '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    निवेशय_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    भङ्ग;
  पक्ष '{':
    गणनम् = p_गणनम्(p);
    सचे (EAT(',')) {
      सचे (isdigit((uch)PEEK())) {
        count2 = p_गणनम्(p);
        REQUIRE(गणनम् <= count2, REG_BADBR);
      } अञ्ञथा /* single number with comma */
        count2 = REGINFINITY;
    } अञ्ञथा /* just a single number */
      count2 = गणनम्;
    repeat(p, pos, गणनम्, count2);
    सचे (!EAT('}')) { /* error heuristics */
      याव (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    भङ्ग;
  }

  सचे (!MORE())
    पटिनिवत्त;
  c = PEEK();
  सचे (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    पटिनिवत्त;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
स्थिर शून्य p_str(संरचना parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  याव (MORE())
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
स्थिर शून्य p_bre(संरचना parse *p, पूर्णाङ्क end1, /* first terminating character */
                  पूर्णाङ्क end2)                  /* second terminating character */
{
  sopno start = HERE();
  पूर्णाङ्क first = 1; /* first subexpression? */
  पूर्णाङ्क wasdollar = 0;

  सचे (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  याव (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  सचे (wasdollar) { /* oops, that was a trailing anchor */
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
स्थिर पूर्णाङ्क /* was the simple RE an unbackslashed $? */
p_simp_re(संरचना parse *p,
          पूर्णाङ्क starordinary) /* is a leading * an ordinary character? */
{
  पूर्णाङ्क c;
  पूर्णाङ्क गणनम्;
  पूर्णाङ्क count2;
  sopno pos;
  पूर्णाङ्क i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  प्रतिज्ञा(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  सचे (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  विकल्प (c) {
  पक्ष '.':
    सचे (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अञ्ञथा
      EMIT(OANY, 0);
    भङ्ग;
  पक्ष '[':
    p_bracket(p);
    भङ्ग;
  पक्ष BACKSL | '{':
    SETERROR(REG_BADRPT);
    भङ्ग;
  पक्ष BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    सचे (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    सचे (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    सचे (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    भङ्ग;
  पक्ष BACKSL | ')': /* should not get here -- must be user */
  पक्ष BACKSL | '}':
    SETERROR(REG_EPAREN);
    भङ्ग;
  पक्ष BACKSL | '1':
  पक्ष BACKSL | '2':
  पक्ष BACKSL | '3':
  पक्ष BACKSL | '4':
  पक्ष BACKSL | '5':
  पक्ष BACKSL | '6':
  पक्ष BACKSL | '7':
  पक्ष BACKSL | '8':
  पक्ष BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    प्रतिज्ञा(i < NPAREN);
    सचे (p->pend[i] != 0) {
      प्रतिज्ञा(i <= p->g->nsub);
      EMIT(OBACK_, i);
      प्रतिज्ञा(p->pbegin[i] != 0);
      प्रतिज्ञा(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      प्रतिज्ञा(OP(p->strip[p->pend[i]]) == ORPAREN);
      (शून्य)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } अञ्ञथा {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    भङ्ग;
  पक्ष '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  स्वभाव:
    ordinary(p, (वर्ण)c);
    भङ्ग;
  }

  सचे (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    निवेशय_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    निवेशय_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } अञ्ञथा सचे (EATTWO('\\', '{')) {
    गणनम् = p_गणनम्(p);
    सचे (EAT(',')) {
      सचे (MORE() && isdigit((uch)PEEK())) {
        count2 = p_गणनम्(p);
        REQUIRE(गणनम् <= count2, REG_BADBR);
      } अञ्ञथा /* single number with comma */
        count2 = REGINFINITY;
    } अञ्ञथा /* just a single number */
      count2 = गणनम्;
    repeat(p, pos, गणनम्, count2);
    सचे (!EATTWO('\\', '}')) { /* error heuristics */
      याव (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } अञ्ञथा सचे (c == '$') /* $ (but not \$) ends it */
    पटिनिवत्त (1);

  पटिनिवत्त (0);
}

/*
 - p_count - parse a repetition count
 */
स्थिर पूर्णाङ्क /* the value */
p_गणनम्(संरचना parse *p) {
  पूर्णाङ्क गणनम् = 0;
  पूर्णाङ्क ndigits = 0;

  याव (MORE() && isdigit((uch)PEEK()) && गणनम् <= DUPMAX) {
    गणनम् = गणनम् * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && गणनम् <= DUPMAX, REG_BADBR);
  पटिनिवत्त (गणनम्);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
स्थिर शून्य p_bracket(संरचना parse *p) {
  cset *cs;
  पूर्णाङ्क invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  सचे (p->end - p->next > 5) {
    सचे (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      पटिनिवत्त;
    }
    सचे (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      पटिनिवत्त;
    }
  }

  सचे ((cs = allocset(p)) == रिक्त) {
    /* allocset did set error status in p */
    पटिनिवत्त;
  }

  सचे (EAT('^'))
    invert++; /* make note to invert set at end */
  सचे (EAT(']'))
    CHadd(cs, ']');
  अञ्ञथा सचे (EAT('-'))
    CHadd(cs, '-');
  याव (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  सचे (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  सचे (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    पटिनिवत्त;
  }

  सचे (p->g->cflags & REG_ICASE) {
    पूर्णाङ्क i;
    पूर्णाङ्क ci;

    अत्थाय (i = p->g->csetsize - 1; i >= 0; i--)
      सचे (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        सचे (ci != i)
          CHadd(cs, ci);
      }
    सचे (cs->multis != रिक्त)
      mccase(p, cs);
  }
  सचे (invert) {
    पूर्णाङ्क i;

    अत्थाय (i = p->g->csetsize - 1; i >= 0; i--)
      सचे (CHIN(cs, i))
        CHsub(cs, i);
      अञ्ञथा
        CHadd(cs, i);
    सचे (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    सचे (cs->multis != रिक्त)
      mcinvert(p, cs);
  }

  प्रतिज्ञा(cs->multis == रिक्त); /* xxx */

  सचे (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } अञ्ञथा {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
स्थिर शून्य p_b_term(संरचना parse *p, cset *cs) {
  वर्ण c;
  वर्ण start, finish;
  पूर्णाङ्क i;

  /* classify what we've got */
  विकल्प ((MORE()) ? PEEK() : '\0') {
  पक्ष '[':
    c = (MORE2()) ? PEEK2() : '\0';
    भङ्ग;
  पक्ष '-':
    SETERROR(REG_ERANGE);
    पटिनिवत्त; /* NOTE RETURN */
    भङ्ग;
  स्वभाव:
    c = '\0';
    भङ्ग;
  }

  विकल्प (c) {
  पक्ष ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    भङ्ग;
  पक्ष '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    भङ्ग;
  स्वभाव: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_चिह्नम्(p);
    सचे (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      सचे (EAT('-'))
        finish = '-';
      अञ्ञथा
        finish = p_b_चिह्नम्(p);
    } अञ्ञथा {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    अत्थाय (i = start; i <= finish; i++)
      CHadd(cs, i);
    भङ्ग;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
स्थिर शून्य p_b_cclass(संरचना parse *p, cset *cs) {
  ध्रुव वर्ण *sp = p->next;
  संरचना cclass *cp;
  size_t len;
  ध्रुव वर्ण *u;
  वर्ण c;

  याव (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  अत्थाय (cp = cclasses; cp->name != रिक्त; cp++)
    सचे (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      भङ्ग;
  सचे (cp->name == रिक्त) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    पटिनिवत्त;
  }

  u = cp->chars;
  याव ((c = *u++) != '\0')
    CHadd(cs, c);
  अत्थाय (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
स्थिर शून्य p_b_eclass(संरचना parse *p, cset *cs) {
  वर्ण c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
स्थिर वर्ण /* value of symbol */
p_b_चिह्नम्(संरचना parse *p) {
  वर्ण मानम्_2;

  REQUIRE(MORE(), REG_EBRACK);
  सचे (!EATTWO('[', '.'))
    पटिनिवत्त (GETNEXT());

  /* collating symbol */
  मानम्_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  पटिनिवत्त (मानम्_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
स्थिर वर्ण                              /* value of collating element */
p_b_coll_elem(संरचना parse *p, पूर्णाङ्क endc) /* name ended by endc,']' */
{
  ध्रुव वर्ण *sp = p->next;
  संरचना cname *cp;
  size_t len;

  याव (MORE() && !SEETWO(endc, ']'))
    NEXT();
  सचे (!MORE()) {
    SETERROR(REG_EBRACK);
    पटिनिवत्त (0);
  }
  len = p->next - sp;
  अत्थाय (cp = cnames; cp->name != रिक्त; cp++)
    सचे (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      पटिनिवत्त (cp->code); /* known name */
  सचे (len == 1)
    पटिनिवत्त (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  पटिनिवत्त (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
स्थिर वर्ण /* if no counterpart, return ch */
othercase(पूर्णाङ्क ch) {
  ch = (uch)ch;
  प्रतिज्ञा(isalpha(ch));
  सचे (isupper(ch))
    पटिनिवत्त ((uch)tolower(ch));
  अञ्ञथा सचे (islower(ch))
    पटिनिवत्त ((uch)toupper(ch));
  अञ्ञथा /* peculiar, but could happen */
    पटिनिवत्त (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
स्थिर शून्य bothcases(संरचना parse *p, पूर्णाङ्क ch) {
  ध्रुव वर्ण *oldnext = p->next;
  ध्रुव वर्ण *oldend = p->end;
  वर्ण bracket[3];

  ch = (uch)ch;
  प्रतिज्ञा(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  प्रतिज्ञा(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
स्थिर शून्य uncased(संरचना parse *p, पूर्णाङ्क ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  सचे (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
स्थिर शून्य ordinary(संरचना parse *p, पूर्णाङ्क ch) {
  सचे ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  अञ्ञथा
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
स्थिर शून्य nonnewline(संरचना parse *p) {
  ध्रुव वर्ण *oldnext = p->next;
  ध्रुव वर्ण *oldend = p->end;
  स्थिर ध्रुव वर्ण bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  प्रतिज्ञा(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
स्थिर शून्य repeat(संरचना parse *p,
                   sopno start, /* operand from here to end of strip */
                   पूर्णाङ्क from,    /* repeated from this number */
                   पूर्णाङ्क to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  सचे (p->error != 0) /* head off possible runaway recursion */
    पटिनिवत्त;

  प्रतिज्ञा(from <= to);

  विकल्प (REP(MAP(from), MAP(to))) {
  पक्ष REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    भङ्ग;
  पक्ष REP(0, 1):   /* as x{1,1}? */
  पक्ष REP(0, N):   /* as x{1,n}? */
  पक्ष REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    निवेशय_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    भङ्ग;
  पक्ष REP(1, 1): /* trivial case */
    /* done */
    भङ्ग;
  पक्ष REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    निवेशय_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    प्रतिज्ञा(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    भङ्ग;
  पक्ष REP(1, INF): /* as x+ */
    निवेशय_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    भङ्ग;
  पक्ष REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    भङ्ग;
  पक्ष REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    भङ्ग;
  स्वभाव:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    भङ्ग;
  }
}

/*
 - seterr - set an error condition
 */
स्थिर पूर्णाङ्क /* useless but makes type checking happy */
seterr(संरचना parse *p, पूर्णाङ्क e) {
  सचे (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  पटिनिवत्त (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
स्थिर cset *allocset(संरचना parse *p) {
  पूर्णाङ्क no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  पूर्णाङ्क i;

  सचे (no >= p->ncsalloc) { /* need another column of space */
    शून्य *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    सचे (nc > SIZE_MAX / परिमाण(cset))
      गच्छ nomem;
    प्रतिज्ञा(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((वर्ण *)p->g->sets, nc * परिमाण(cset));
    सचे (ptr == रिक्त)
      गच्छ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((वर्ण *)p->g->setbits, nbytes);
    सचे (ptr == रिक्त)
      गच्छ nomem;
    p->g->setbits = ptr;

    अत्थाय (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (शून्य)memset((वर्ण *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  सचे (p->g->sets == रिक्त || p->g->setbits == रिक्त)
    गच्छ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = रिक्त;

  पटिनिवत्त (cs);
nomem:
  मुक्त(p->g->sets);
  p->g->sets = रिक्त;
  मुक्त(p->g->setbits);
  p->g->setbits = रिक्त;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  पटिनिवत्त (रिक्त);
}

/*
 - freeset - free a now-unused set
 */
स्थिर शून्य freeset(संरचना parse *p, cset *cs) {
  size_t i;
  cset *शिखरम्_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  अत्थाय (i = 0; i < css; i++)
    CHsub(cs, i);
  सचे (cs == शिखरम्_2 - 1) /* recover only the easy case */
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
स्थिर पूर्णाङ्क /* set number */
freezeset(संरचना parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *शिखरम्_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  अत्थाय (cs2 = &p->g->sets[0]; cs2 < शिखरम्_2; cs2++)
    सचे (cs2->hash == h && cs2 != cs) {
      /* maybe */
      अत्थाय (i = 0; i < css; i++)
        सचे (!!CHIN(cs2, i) != !!CHIN(cs, i))
          भङ्ग; /* no */
      सचे (i == css)
        भङ्ग; /* yes */
    }

  सचे (cs2 < शिखरम्_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  पटिनिवत्त ((पूर्णाङ्क)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
स्थिर पूर्णाङ्क /* character; there is no "none" value */
firstch(संरचना parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  अत्थाय (i = 0; i < css; i++)
    सचे (CHIN(cs, i))
      पटिनिवत्त ((वर्ण)i);
  प्रतिज्ञा(never);
  पटिनिवत्त (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
स्थिर पूर्णाङ्क nch(संरचना parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  पूर्णाङ्क n = 0;

  अत्थाय (i = 0; i < css; i++)
    सचे (CHIN(cs, i))
      n++;
  पटिनिवत्त (n);
}

/*
 - mcadd - add a collating element to a cset
 */
स्थिर शून्य mcadd(संरचना parse *p, cset *cs, ध्रुव वर्ण *cp) {
  size_t oldend = cs->smultis;
  शून्य *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  सचे (np == रिक्त) {
    सचे (cs->multis)
      मुक्त(cs->multis);
    cs->multis = रिक्त;
    SETERROR(REG_ESPACE);
    पटिनिवत्त;
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
स्थिर शून्य mcinvert(संरचना parse *p, cset *cs) {
  प्रतिज्ञा(cs->multis == रिक्त); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
स्थिर शून्य mccase(संरचना parse *p, cset *cs) {
  प्रतिज्ञा(cs->multis == रिक्त); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
स्थिर पूर्णाङ्क /* predicate */
isinsets(संरचना re_guts *g, पूर्णाङ्क c) {
  uch *col;
  पूर्णाङ्क i;
  पूर्णाङ्क ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  अत्थाय (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    सचे (col[uc] != 0)
      पटिनिवत्त (1);
  पटिनिवत्त (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
स्थिर पूर्णाङ्क /* predicate */
samesets(संरचना re_guts *g, पूर्णाङ्क c1, पूर्णाङ्क c2) {
  uch *col;
  पूर्णाङ्क i;
  पूर्णाङ्क ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  अत्थाय (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    सचे (col[uc1] != col[uc2])
      पटिनिवत्त (0);
  पटिनिवत्त (1);
}

/*
 - categorize - sort out character categories
 */
स्थिर शून्य categorize(संरचना parse *p, संरचना re_guts *g) {
  cat_t *cats = g->categories;
  पूर्णाङ्क c;
  पूर्णाङ्क c2;
  cat_t cat;

  /* avoid making error situations worse */
  सचे (p->error != 0)
    पटिनिवत्त;

  अत्थाय (c = CHAR_MIN; c <= CHAR_MAX; c++)
    सचे (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      अत्थाय (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        सचे (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
स्थिर sopno                       /* start of duplicate */
dupl(संरचना parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  प्रतिज्ञा(finish >= start);
  सचे (len == 0)
    पटिनिवत्त (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  प्रतिज्ञा(p->ssize >= p->slen + len);
  (शून्य)memmove((वर्ण *)(p->strip + p->slen), (वर्ण *)(p->strip + start),
                (size_t)len * परिमाण(sop));
  p->slen += len;
  पटिनिवत्त (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
स्थिर शून्य doemit(संरचना parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  सचे (p->error != 0)
    पटिनिवत्त;

  /* deal with oversize operands ("can't happen", more or less) */
  प्रतिज्ञा(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  सचे (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  प्रतिज्ञा(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
स्थिर शून्य doinsert(संरचना parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  पूर्णाङ्क i;

  /* avoid making error situations worse */
  सचे (p->error != 0)
    पटिनिवत्त;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  प्रतिज्ञा(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  प्रतिज्ञा(pos > 0);
  अत्थाय (i = 1; i < NPAREN; i++) {
    सचे (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    सचे (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((वर्ण *)&p->strip[pos + 1], (वर्ण *)&p->strip[pos],
          (HERE() - pos - 1) * परिमाण(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
स्थिर शून्य dofwd(संरचना parse *p, sopno pos, sop मानम्_2) {
  /* avoid making error situations worse */
  सचे (p->error != 0)
    पटिनिवत्त;

  प्रतिज्ञा(मानम्_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | मानम्_2;
}

/*
 - enlarge - enlarge the strip
 */
स्थिर शून्य enlarge(संरचना parse *p, sopno size) {
  sop *sp;

  सचे (p->ssize >= size)
    पटिनिवत्त;

  सचे ((uintptr_t)size > SIZE_MAX / परिमाण(sop)) {
    SETERROR(REG_ESPACE);
    पटिनिवत्त;
  }

  sp = (sop *)realloc(p->strip, size * परिमाण(sop));
  सचे (sp == रिक्त) {
    SETERROR(REG_ESPACE);
    पटिनिवत्त;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
स्थिर शून्य stripsnug(संरचना parse *p, संरचना re_guts *g) {
  g->nstates = p->slen;
  सचे ((uintptr_t)p->slen > SIZE_MAX / परिमाण(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    पटिनिवत्त;
  }

  g->strip = (sop *)realloc((वर्ण *)p->strip, p->slen * परिमाण(sop));
  सचे (g->strip == रिक्त) {
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
स्थिर शून्य findmust(संरचना parse *p, संरचना re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  वर्ण *cp;
  sopno i;

  /* avoid making error situations worse */
  सचे (p->error != 0)
    पटिनिवत्त;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  करोहि {
    s = *scan++;
    विकल्प (OP(s)) {
    पक्ष OCHAR:        /* sequence member */
      सचे (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      भङ्ग;
    पक्ष OPLUS_: /* things that don't break one */
    पक्ष OLPAREN:
    पक्ष ORPAREN:
      भङ्ग;
    पक्ष OQUEST_: /* things that must be skipped */
    पक्ष OCH_:
      scan--;
      करोहि {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        सचे (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          पटिनिवत्त;
        }
      } याव (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    स्वभाव:                  /* things that break a sequence */
      सचे (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      भङ्ग;
    }
  } याव (OP(s) != OEND);

  सचे (g->mlen == 0) /* there isn't one */
    पटिनिवत्त;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  सचे (g->must == रिक्त) { /* argh; just forget it */
    g->mlen = 0;
    पटिनिवत्त;
  }
  cp = g->must;
  scan = start;
  अत्थाय (i = g->mlen; i > 0; i--) {
    याव (OP(s = *scan++) != OCHAR)
      अनुवर्त;
    प्रतिज्ञा(cp < g->must + g->mlen);
    *cp++ = (वर्ण)OPND(s);
  }
  प्रतिज्ञा(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
स्थिर sopno /* nesting depth */
pluscount(संरचना parse *p, संरचना re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  सचे (p->error != 0)
    पटिनिवत्त (0); /* there may not be an OEND */

  scan = g->strip + 1;
  करोहि {
    s = *scan++;
    विकल्प (OP(s)) {
    पक्ष OPLUS_:
      plusnest++;
      भङ्ग;
    पक्ष O_PLUS:
      सचे (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      भङ्ग;
    }
  } याव (OP(s) != OEND);
  सचे (plusnest != 0)
    g->iflags |= REGEX_BAD;
  पटिनिवत्त (maxnest);
}
