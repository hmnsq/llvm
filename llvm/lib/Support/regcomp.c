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
  स्थिरांक अक्षर *name;
  स्थिरांक अक्षर *chars;
  स्थिरांक अक्षर *multis;
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
  स्थिरांक अक्षर *name;
  अक्षर code;
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
  स्थिरांक अक्षर *next; /* next character in RE */
  स्थिरांक अक्षर *end;  /* end of string (-> NUL normally) */
  पूर्णांक error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  पूर्णांक ncsalloc;     /* number of csets allocated */
  संरचना re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

स्थिर शून्य p_ere(संरचना parse *, पूर्णांक);
स्थिर शून्य p_ere_exp(संरचना parse *);
स्थिर शून्य p_str(संरचना parse *);
स्थिर शून्य p_bre(संरचना parse *, पूर्णांक, पूर्णांक);
स्थिर पूर्णांक p_simp_re(संरचना parse *, पूर्णांक);
स्थिर पूर्णांक p_मोजणी(संरचना parse *);
स्थिर शून्य p_bracket(संरचना parse *);
स्थिर शून्य p_b_term(संरचना parse *, cset *);
स्थिर शून्य p_b_cclass(संरचना parse *, cset *);
स्थिर शून्य p_b_eclass(संरचना parse *, cset *);
स्थिर अक्षर p_b_चिन्ह(संरचना parse *);
स्थिर अक्षर p_b_coll_elem(संरचना parse *, पूर्णांक);
स्थिर अक्षर othercase(पूर्णांक);
स्थिर शून्य bothcases(संरचना parse *, पूर्णांक);
स्थिर शून्य uncased(संरचना parse *, पूर्णांक);
स्थिर शून्य ordinary(संरचना parse *, पूर्णांक);
स्थिर शून्य nonnewline(संरचना parse *);
स्थिर शून्य repeat(संरचना parse *, sopno, पूर्णांक, पूर्णांक);
स्थिर पूर्णांक seterr(संरचना parse *, पूर्णांक);
स्थिर cset *allocset(संरचना parse *);
स्थिर शून्य freeset(संरचना parse *, cset *);
स्थिर पूर्णांक freezeset(संरचना parse *, cset *);
स्थिर पूर्णांक firstch(संरचना parse *, cset *);
स्थिर पूर्णांक nch(संरचना parse *, cset *);
स्थिर शून्य mcadd(संरचना parse *, cset *, स्थिरांक अक्षर *);
स्थिर शून्य mcinvert(संरचना parse *, cset *);
स्थिर शून्य mccase(संरचना parse *, cset *);
स्थिर पूर्णांक isinsets(संरचना re_guts *, पूर्णांक);
स्थिर पूर्णांक samesets(संरचना re_guts *, पूर्णांक, पूर्णांक);
स्थिर शून्य categorize(संरचना parse *, संरचना re_guts *);
स्थिर sopno dupl(संरचना parse *, sopno, sopno);
स्थिर शून्य doemit(संरचना parse *, sop, माप_प्रकार);
स्थिर शून्य doinsert(संरचना parse *, sop, माप_प्रकार, sopno);
स्थिर शून्य dofwd(संरचना parse *, sopno, sop);
स्थिर शून्य enlarge(संरचना parse *, sopno);
स्थिर शून्य stripsnug(संरचना parse *, संरचना re_guts *);
स्थिर शून्य findmust(संरचना parse *, संरचना re_guts *);
स्थिर sopno pluscount(संरचना parse *, संरचना re_guts *);

स्थिर अक्षर nuls[10]; /* place to point scanner in event of error */

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
#define EMIT(op, sopnd) doemit(p, (sop)(op), (माप_प्रकार)(sopnd))
#define घाला_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#अन्यथा
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef न_संशोधन
स्थिर पूर्णांक never = 0; /* for use in asserts; shuts lint up */
#अन्यथा
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
पूर्णांक /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, स्थिरांक अक्षर *pattern, पूर्णांक cflags) {
  संरचना parse pa;
  संरचना re_guts *g;
  संरचना parse *p = &pa;
  पूर्णांक i;
  माप_प्रकार len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#अन्यथा
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  जर ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    परत (REG_INVARG);

  जर (cflags & REG_PEND) {
    जर (preg->re_endp < pattern)
      परत (REG_INVARG);
    len = preg->re_endp - pattern;
  } अन्यथा {
    len = म_माप(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (संरचना re_guts *)दो_स्मृति(आकार(संरचना re_guts) +
                               (NC - 1) * आकार(cat_t));
  जर (g == रिक्त)
    परत (REG_ESPACE);
  p->ssize = len / (माप_प्रकार)2 * (माप_प्रकार)3 + (माप_प्रकार)1; /* ugh */
  p->strip = (sop *)सुस्मृति(p->ssize, आकार(sop));
  p->slen = 0;
  जर (p->strip == रिक्त) {
    मुक्त((अक्षर *)g);
    परत (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  साठी (i = 0; i < NPAREN; i++) {
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
  g->categories = &g->catspace[-(अक्षर_न्यून)];
  (शून्य)स_रखो((अक्षर *)g->catspace, 0, NC * आकार(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  जर (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  अन्यथा जर (cflags & REG_NOSPEC)
    p_str(p);
  अन्यथा
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
  जर (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  जर (p->error != 0) /* lose */
    llvm_regfree(preg);
  परत (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
स्थिर शून्य p_ere(संरचना parse *p,
                  पूर्णांक stop) /* character this ERE should end at */
{
  अक्षर c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  पूर्णांक first = 1; /* is this the first alternative? */

  साठी (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    पर्यंत (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    जर (!EAT('|'))
      थांबा; /* NOTE BREAK OUT */

    जर (first) {
      घाला_3(OCH_, conc); /* offset is wrong */
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

  जर (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  प्रतिज्ञा(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
स्थिर शून्य p_ere_exp(संरचना parse *p) {
  अक्षर c;
  sopno pos;
  पूर्णांक मोजणी;
  पूर्णांक count2;
  पूर्णांक backrefnum;
  sopno subno;
  पूर्णांक wascaret = 0;

  प्रतिज्ञा(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  निवड (c) {
  स्थिती '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    जर (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    जर (!SEE(')'))
      p_ere(p, ')');
    जर (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    थांबा;
#ifndef POSIX_MISTAKE
  स्थिती ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    थांबा;
#endif
  स्थिती '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    थांबा;
  स्थिती '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    थांबा;
  स्थिती '|':
    SETERROR(REG_EMPTY);
    थांबा;
  स्थिती '*':
  स्थिती '+':
  स्थिती '?':
    SETERROR(REG_BADRPT);
    थांबा;
  स्थिती '.':
    जर (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    थांबा;
  स्थिती '[':
    p_bracket(p);
    थांबा;
  स्थिती '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    जर (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } अन्यथा जर (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      पर्यंत (MORE() && है_अंक(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } अन्यथा जर (c == 'n') {
      ordinary(p, '\n');
      थांबा;
    } अन्यथा जर (c == 't') {
      ordinary(p, '\t');
      थांबा;
    } अन्यथा जर (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      अक्षर hexstr[3] = {0};
      अक्षर *hexp;
      पूर्णांक val;
      जर (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = म_से_दीर्घ(hexstr, &hexp, 16);
        जर (*hexp == '\0') {
          NEXT2();
          uncased(p, (अक्षर)val);
          थांबा;
        }
      }
      ordinary(p, 'x');
      थांबा;
    } अन्यथा {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      थांबा;
    }

    जर (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      थांबा;
    }

    जर (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      थांबा;
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
    थांबा;
  स्थिती '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !है_अंक((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  पूर्वनिर्धारित:
    ordinary(p, c);
    थांबा;
  }

  जर (!MORE())
    परत;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  जर (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    परत; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  निवड (c) {
  स्थिती '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    घाला_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    घाला_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    थांबा;
  स्थिती '+':
    घाला_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    थांबा;
  स्थिती '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घाला_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    थांबा;
  स्थिती '{':
    मोजणी = p_मोजणी(p);
    जर (EAT(',')) {
      जर (है_अंक((uch)PEEK())) {
        count2 = p_मोजणी(p);
        REQUIRE(मोजणी <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = मोजणी;
    repeat(p, pos, मोजणी, count2);
    जर (!EAT('}')) { /* error heuristics */
      पर्यंत (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    थांबा;
  }

  जर (!MORE())
    परत;
  c = PEEK();
  जर (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    परत;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
स्थिर शून्य p_str(संरचना parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  पर्यंत (MORE())
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
स्थिर शून्य p_bre(संरचना parse *p, पूर्णांक end1, /* first terminating character */
                  पूर्णांक end2)                  /* second terminating character */
{
  sopno start = HERE();
  पूर्णांक first = 1; /* first subexpression? */
  पूर्णांक wasdollar = 0;

  जर (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  पर्यंत (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  जर (wasdollar) { /* oops, that was a trailing anchor */
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
स्थिर पूर्णांक /* was the simple RE an unbackslashed $? */
p_simp_re(संरचना parse *p,
          पूर्णांक starordinary) /* is a leading * an ordinary character? */
{
  पूर्णांक c;
  पूर्णांक मोजणी;
  पूर्णांक count2;
  sopno pos;
  पूर्णांक i;
  sopno subno;
#define BACKSL (1 << अक्षर_बिट)

  pos = HERE(); /* repetition op, if any, covers from here */

  प्रतिज्ञा(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  जर (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  निवड (c) {
  स्थिती '.':
    जर (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    थांबा;
  स्थिती '[':
    p_bracket(p);
    थांबा;
  स्थिती BACKSL | '{':
    SETERROR(REG_BADRPT);
    थांबा;
  स्थिती BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    जर (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    जर (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    जर (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    थांबा;
  स्थिती BACKSL | ')': /* should not get here -- must be user */
  स्थिती BACKSL | '}':
    SETERROR(REG_EPAREN);
    थांबा;
  स्थिती BACKSL | '1':
  स्थिती BACKSL | '2':
  स्थिती BACKSL | '3':
  स्थिती BACKSL | '4':
  स्थिती BACKSL | '5':
  स्थिती BACKSL | '6':
  स्थिती BACKSL | '7':
  स्थिती BACKSL | '8':
  स्थिती BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    प्रतिज्ञा(i < NPAREN);
    जर (p->pend[i] != 0) {
      प्रतिज्ञा(i <= p->g->nsub);
      EMIT(OBACK_, i);
      प्रतिज्ञा(p->pbegin[i] != 0);
      प्रतिज्ञा(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      प्रतिज्ञा(OP(p->strip[p->pend[i]]) == ORPAREN);
      (शून्य)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } अन्यथा {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    थांबा;
  स्थिती '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  पूर्वनिर्धारित:
    ordinary(p, (अक्षर)c);
    थांबा;
  }

  जर (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    घाला_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    घाला_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } अन्यथा जर (EATTWO('\\', '{')) {
    मोजणी = p_मोजणी(p);
    जर (EAT(',')) {
      जर (MORE() && है_अंक((uch)PEEK())) {
        count2 = p_मोजणी(p);
        REQUIRE(मोजणी <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = मोजणी;
    repeat(p, pos, मोजणी, count2);
    जर (!EATTWO('\\', '}')) { /* error heuristics */
      पर्यंत (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } अन्यथा जर (c == '$') /* $ (but not \$) ends it */
    परत (1);

  परत (0);
}

/*
 - p_count - parse a repetition count
 */
स्थिर पूर्णांक /* the value */
p_मोजणी(संरचना parse *p) {
  पूर्णांक मोजणी = 0;
  पूर्णांक ndigits = 0;

  पर्यंत (MORE() && है_अंक((uch)PEEK()) && मोजणी <= DUPMAX) {
    मोजणी = मोजणी * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && मोजणी <= DUPMAX, REG_BADBR);
  परत (मोजणी);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
स्थिर शून्य p_bracket(संरचना parse *p) {
  cset *cs;
  पूर्णांक invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  जर (p->end - p->next > 5) {
    जर (म_भेदन(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      परत;
    }
    जर (म_भेदन(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      परत;
    }
  }

  जर ((cs = allocset(p)) == रिक्त) {
    /* allocset did set error status in p */
    परत;
  }

  जर (EAT('^'))
    invert++; /* make note to invert set at end */
  जर (EAT(']'))
    CHadd(cs, ']');
  अन्यथा जर (EAT('-'))
    CHadd(cs, '-');
  पर्यंत (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  जर (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  जर (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    परत;
  }

  जर (p->g->cflags & REG_ICASE) {
    पूर्णांक i;
    पूर्णांक ci;

    साठी (i = p->g->csetsize - 1; i >= 0; i--)
      जर (CHIN(cs, i) && है_अक्षर(i)) {
        ci = othercase(i);
        जर (ci != i)
          CHadd(cs, ci);
      }
    जर (cs->multis != रिक्त)
      mccase(p, cs);
  }
  जर (invert) {
    पूर्णांक i;

    साठी (i = p->g->csetsize - 1; i >= 0; i--)
      जर (CHIN(cs, i))
        CHsub(cs, i);
      अन्यथा
        CHadd(cs, i);
    जर (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    जर (cs->multis != रिक्त)
      mcinvert(p, cs);
  }

  प्रतिज्ञा(cs->multis == रिक्त); /* xxx */

  जर (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } अन्यथा {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
स्थिर शून्य p_b_term(संरचना parse *p, cset *cs) {
  अक्षर c;
  अक्षर start, finish;
  पूर्णांक i;

  /* classify what we've got */
  निवड ((MORE()) ? PEEK() : '\0') {
  स्थिती '[':
    c = (MORE2()) ? PEEK2() : '\0';
    थांबा;
  स्थिती '-':
    SETERROR(REG_ERANGE);
    परत; /* NOTE RETURN */
    थांबा;
  पूर्वनिर्धारित:
    c = '\0';
    थांबा;
  }

  निवड (c) {
  स्थिती ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    थांबा;
  स्थिती '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    थांबा;
  पूर्वनिर्धारित: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_चिन्ह(p);
    जर (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      जर (EAT('-'))
        finish = '-';
      अन्यथा
        finish = p_b_चिन्ह(p);
    } अन्यथा {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    साठी (i = start; i <= finish; i++)
      CHadd(cs, i);
    थांबा;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
स्थिर शून्य p_b_cclass(संरचना parse *p, cset *cs) {
  स्थिरांक अक्षर *sp = p->next;
  संरचना cclass *cp;
  माप_प्रकार len;
  स्थिरांक अक्षर *u;
  अक्षर c;

  पर्यंत (MORE() && है_अक्षर((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  साठी (cp = cclasses; cp->name != रिक्त; cp++)
    जर (म_भेदन(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      थांबा;
  जर (cp->name == रिक्त) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    परत;
  }

  u = cp->chars;
  पर्यंत ((c = *u++) != '\0')
    CHadd(cs, c);
  साठी (u = cp->multis; *u != '\0'; u += म_माप(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
स्थिर शून्य p_b_eclass(संरचना parse *p, cset *cs) {
  अक्षर c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
स्थिर अक्षर /* value of symbol */
p_b_चिन्ह(संरचना parse *p) {
  अक्षर मूल्य_2;

  REQUIRE(MORE(), REG_EBRACK);
  जर (!EATTWO('[', '.'))
    परत (GETNEXT());

  /* collating symbol */
  मूल्य_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  परत (मूल्य_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
स्थिर अक्षर                              /* value of collating element */
p_b_coll_elem(संरचना parse *p, पूर्णांक endc) /* name ended by endc,']' */
{
  स्थिरांक अक्षर *sp = p->next;
  संरचना cname *cp;
  माप_प्रकार len;

  पर्यंत (MORE() && !SEETWO(endc, ']'))
    NEXT();
  जर (!MORE()) {
    SETERROR(REG_EBRACK);
    परत (0);
  }
  len = p->next - sp;
  साठी (cp = cnames; cp->name != रिक्त; cp++)
    जर (म_भेदन(cp->name, sp, len) == 0 && म_माप(cp->name) == len)
      परत (cp->code); /* known name */
  जर (len == 1)
    परत (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  परत (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
स्थिर अक्षर /* if no counterpart, return ch */
othercase(पूर्णांक ch) {
  ch = (uch)ch;
  प्रतिज्ञा(है_अक्षर(ch));
  जर (है_बड़ा(ch))
    परत ((uch)छोटे(ch));
  अन्यथा जर (है_छोटा(ch))
    परत ((uch)बड़े(ch));
  अन्यथा /* peculiar, but could happen */
    परत (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
स्थिर शून्य bothcases(संरचना parse *p, पूर्णांक ch) {
  स्थिरांक अक्षर *oldnext = p->next;
  स्थिरांक अक्षर *oldend = p->end;
  अक्षर bracket[3];

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
स्थिर शून्य uncased(संरचना parse *p, पूर्णांक ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  जर (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
स्थिर शून्य ordinary(संरचना parse *p, पूर्णांक ch) {
  जर ((p->g->cflags & REG_ICASE) && है_अक्षर((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  अन्यथा
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
स्थिर शून्य nonnewline(संरचना parse *p) {
  स्थिरांक अक्षर *oldnext = p->next;
  स्थिरांक अक्षर *oldend = p->end;
  स्थिर स्थिरांक अक्षर bracket[4] = {'^', '\n', ']', '\0'};

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
                   पूर्णांक from,    /* repeated from this number */
                   पूर्णांक to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  जर (p->error != 0) /* head off possible runaway recursion */
    परत;

  प्रतिज्ञा(from <= to);

  निवड (REP(MAP(from), MAP(to))) {
  स्थिती REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    थांबा;
  स्थिती REP(0, 1):   /* as x{1,1}? */
  स्थिती REP(0, N):   /* as x{1,n}? */
  स्थिती REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घाला_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    थांबा;
  स्थिती REP(1, 1): /* trivial case */
    /* done */
    थांबा;
  स्थिती REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घाला_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    प्रतिज्ञा(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    थांबा;
  स्थिती REP(1, INF): /* as x+ */
    घाला_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    थांबा;
  स्थिती REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    थांबा;
  स्थिती REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    थांबा;
  पूर्वनिर्धारित:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    थांबा;
  }
}

/*
 - seterr - set an error condition
 */
स्थिर पूर्णांक /* useless but makes type checking happy */
seterr(संरचना parse *p, पूर्णांक e) {
  जर (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  परत (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
स्थिर cset *allocset(संरचना parse *p) {
  पूर्णांक no = p->g->ncsets++;
  माप_प्रकार nc;
  माप_प्रकार nbytes;
  cset *cs;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णांक i;

  जर (no >= p->ncsalloc) { /* need another column of space */
    शून्य *ptr;

    p->ncsalloc += अक्षर_बिट;
    nc = p->ncsalloc;
    जर (nc > SIZE_MAX / आकार(cset))
      जा nomem;
    प्रतिज्ञा(nc % अक्षर_बिट == 0);
    nbytes = nc / अक्षर_बिट * css;

    ptr = (cset *)पुनः_स्मृति((अक्षर *)p->g->sets, nc * आकार(cset));
    जर (ptr == रिक्त)
      जा nomem;
    p->g->sets = ptr;

    ptr = (uch *)पुनः_स्मृति((अक्षर *)p->g->setbits, nbytes);
    जर (ptr == रिक्त)
      जा nomem;
    p->g->setbits = ptr;

    साठी (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / अक्षर_बिट);

    (शून्य)स_रखो((अक्षर *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  जर (p->g->sets == रिक्त || p->g->setbits == रिक्त)
    जा nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / अक्षर_बिट);
  cs->mask = 1 << ((no) % अक्षर_बिट);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = रिक्त;

  परत (cs);
nomem:
  मुक्त(p->g->sets);
  p->g->sets = रिक्त;
  मुक्त(p->g->setbits);
  p->g->setbits = रिक्त;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  परत (रिक्त);
}

/*
 - freeset - free a now-unused set
 */
स्थिर शून्य freeset(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  cset *शीर्ष_2 = &p->g->sets[p->g->ncsets];
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  साठी (i = 0; i < css; i++)
    CHsub(cs, i);
  जर (cs == शीर्ष_2 - 1) /* recover only the easy case */
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
स्थिर पूर्णांक /* set number */
freezeset(संरचना parse *p, cset *cs) {
  uch h = cs->hash;
  माप_प्रकार i;
  cset *शीर्ष_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  /* look for an earlier one which is the same */
  साठी (cs2 = &p->g->sets[0]; cs2 < शीर्ष_2; cs2++)
    जर (cs2->hash == h && cs2 != cs) {
      /* maybe */
      साठी (i = 0; i < css; i++)
        जर (!!CHIN(cs2, i) != !!CHIN(cs, i))
          थांबा; /* no */
      जर (i == css)
        थांबा; /* yes */
    }

  जर (cs2 < शीर्ष_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  परत ((पूर्णांक)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
स्थिर पूर्णांक /* character; there is no "none" value */
firstch(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  साठी (i = 0; i < css; i++)
    जर (CHIN(cs, i))
      परत ((अक्षर)i);
  प्रतिज्ञा(never);
  परत (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
स्थिर पूर्णांक nch(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णांक n = 0;

  साठी (i = 0; i < css; i++)
    जर (CHIN(cs, i))
      n++;
  परत (n);
}

/*
 - mcadd - add a collating element to a cset
 */
स्थिर शून्य mcadd(संरचना parse *p, cset *cs, स्थिरांक अक्षर *cp) {
  माप_प्रकार oldend = cs->smultis;
  शून्य *np;

  cs->smultis += म_माप(cp) + 1;
  np = पुनः_स्मृति(cs->multis, cs->smultis);
  जर (np == रिक्त) {
    जर (cs->multis)
      मुक्त(cs->multis);
    cs->multis = रिक्त;
    SETERROR(REG_ESPACE);
    परत;
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
स्थिर पूर्णांक /* predicate */
isinsets(संरचना re_guts *g, पूर्णांक c) {
  uch *col;
  पूर्णांक i;
  पूर्णांक ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc = (uch)c;

  साठी (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    जर (col[uc] != 0)
      परत (1);
  परत (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
स्थिर पूर्णांक /* predicate */
samesets(संरचना re_guts *g, पूर्णांक c1, पूर्णांक c2) {
  uch *col;
  पूर्णांक i;
  पूर्णांक ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc1 = (uch)c1;
  अचिन्हित uc2 = (uch)c2;

  साठी (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    जर (col[uc1] != col[uc2])
      परत (0);
  परत (1);
}

/*
 - categorize - sort out character categories
 */
स्थिर शून्य categorize(संरचना parse *p, संरचना re_guts *g) {
  cat_t *cats = g->categories;
  पूर्णांक c;
  पूर्णांक c2;
  cat_t cat;

  /* avoid making error situations worse */
  जर (p->error != 0)
    परत;

  साठी (c = अक्षर_न्यून; c <= अक्षर_उच्च; c++)
    जर (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      साठी (c2 = c + 1; c2 <= अक्षर_उच्च; c2++)
        जर (cats[c2] == 0 && samesets(g, c, c2))
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
  जर (len == 0)
    परत (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  प्रतिज्ञा(p->ssize >= p->slen + len);
  (शून्य)स_हटाओ((अक्षर *)(p->strip + p->slen), (अक्षर *)(p->strip + start),
                (माप_प्रकार)len * आकार(sop));
  p->slen += len;
  परत (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
स्थिर शून्य doemit(संरचना parse *p, sop op, माप_प्रकार opnd) {
  /* avoid making error situations worse */
  जर (p->error != 0)
    परत;

  /* deal with oversize operands ("can't happen", more or less) */
  प्रतिज्ञा(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  जर (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  प्रतिज्ञा(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
स्थिर शून्य doinsert(संरचना parse *p, sop op, माप_प्रकार opnd, sopno pos) {
  sopno sn;
  sop s;
  पूर्णांक i;

  /* avoid making error situations worse */
  जर (p->error != 0)
    परत;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  प्रतिज्ञा(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  प्रतिज्ञा(pos > 0);
  साठी (i = 1; i < NPAREN; i++) {
    जर (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    जर (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  स_हटाओ((अक्षर *)&p->strip[pos + 1], (अक्षर *)&p->strip[pos],
          (HERE() - pos - 1) * आकार(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
स्थिर शून्य dofwd(संरचना parse *p, sopno pos, sop मूल्य_2) {
  /* avoid making error situations worse */
  जर (p->error != 0)
    परत;

  प्रतिज्ञा(मूल्य_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | मूल्य_2;
}

/*
 - enlarge - enlarge the strip
 */
स्थिर शून्य enlarge(संरचना parse *p, sopno size) {
  sop *sp;

  जर (p->ssize >= size)
    परत;

  जर ((uintptr_t)size > SIZE_MAX / आकार(sop)) {
    SETERROR(REG_ESPACE);
    परत;
  }

  sp = (sop *)पुनः_स्मृति(p->strip, size * आकार(sop));
  जर (sp == रिक्त) {
    SETERROR(REG_ESPACE);
    परत;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
स्थिर शून्य stripsnug(संरचना parse *p, संरचना re_guts *g) {
  g->nstates = p->slen;
  जर ((uintptr_t)p->slen > SIZE_MAX / आकार(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    परत;
  }

  g->strip = (sop *)पुनः_स्मृति((अक्षर *)p->strip, p->slen * आकार(sop));
  जर (g->strip == रिक्त) {
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
  अक्षर *cp;
  sopno i;

  /* avoid making error situations worse */
  जर (p->error != 0)
    परत;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  करा {
    s = *scan++;
    निवड (OP(s)) {
    स्थिती OCHAR:        /* sequence member */
      जर (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      थांबा;
    स्थिती OPLUS_: /* things that don't break one */
    स्थिती OLPAREN:
    स्थिती ORPAREN:
      थांबा;
    स्थिती OQUEST_: /* things that must be skipped */
    स्थिती OCH_:
      scan--;
      करा {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        जर (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          परत;
        }
      } पर्यंत (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    पूर्वनिर्धारित:                  /* things that break a sequence */
      जर (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      थांबा;
    }
  } पर्यंत (OP(s) != OEND);

  जर (g->mlen == 0) /* there isn't one */
    परत;

  /* turn it into a character string */
  g->must = दो_स्मृति((माप_प्रकार)g->mlen + 1);
  जर (g->must == रिक्त) { /* argh; just forget it */
    g->mlen = 0;
    परत;
  }
  cp = g->must;
  scan = start;
  साठी (i = g->mlen; i > 0; i--) {
    पर्यंत (OP(s = *scan++) != OCHAR)
      सुरूठेवा;
    प्रतिज्ञा(cp < g->must + g->mlen);
    *cp++ = (अक्षर)OPND(s);
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

  जर (p->error != 0)
    परत (0); /* there may not be an OEND */

  scan = g->strip + 1;
  करा {
    s = *scan++;
    निवड (OP(s)) {
    स्थिती OPLUS_:
      plusnest++;
      थांबा;
    स्थिती O_PLUS:
      जर (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      थांबा;
    }
  } पर्यंत (OP(s) != OEND);
  जर (plusnest != 0)
    g->iflags |= REGEX_BAD;
  परत (maxnest);
}
