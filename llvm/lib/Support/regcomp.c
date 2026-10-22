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
  अचर वर्ण *name;
  अचर वर्ण *chars;
  अचर वर्ण *multis;
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
  अचर वर्ण *name;
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
  अचर वर्ण *next; /* next character in RE */
  अचर वर्ण *end;  /* end of string (-> NUL normally) */
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
स्थिर पूर्णाङ्क p_गन्ती(संरचना parse *);
स्थिर शून्य p_bracket(संरचना parse *);
स्थिर शून्य p_b_term(संरचना parse *, cset *);
स्थिर शून्य p_b_cclass(संरचना parse *, cset *);
स्थिर शून्य p_b_eclass(संरचना parse *, cset *);
स्थिर वर्ण p_b_चिह्न(संरचना parse *);
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
स्थिर शून्य mcadd(संरचना parse *, cset *, अचर वर्ण *);
स्थिर शून्य mcinvert(संरचना parse *, cset *);
स्थिर शून्य mccase(संरचना parse *, cset *);
स्थिर पूर्णाङ्क isinsets(संरचना re_guts *, पूर्णाङ्क);
स्थिर पूर्णाङ्क samesets(संरचना re_guts *, पूर्णाङ्क, पूर्णाङ्क);
स्थिर शून्य categorize(संरचना parse *, संरचना re_guts *);
स्थिर sopno dupl(संरचना parse *, sopno, sopno);
स्थिर शून्य doemit(संरचना parse *, sop, माप_प्रकार);
स्थिर शून्य doinsert(संरचना parse *, sop, माप_प्रकार, sopno);
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
#define EMIT(op, sopnd) doemit(p, (sop)(op), (माप_प्रकार)(sopnd))
#define घुसाउ_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
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
स्थिर पूर्णाङ्क never = 0; /* for use in asserts; shuts lint up */
#अन्यथा
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
पूर्णाङ्क /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, अचर वर्ण *pattern, पूर्णाङ्क cflags) {
  संरचना parse pa;
  संरचना re_guts *g;
  संरचना parse *p = &pa;
  पूर्णाङ्क i;
  माप_प्रकार len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#अन्यथा
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  यदि ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    फर्काऊ (REG_INVARG);

  यदि (cflags & REG_PEND) {
    यदि (preg->re_endp < pattern)
      फर्काऊ (REG_INVARG);
    len = preg->re_endp - pattern;
  } अन्यथा {
    len = म_माप(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (संरचना re_guts *)दो_स्मृति(परिमाण(संरचना re_guts) +
                               (NC - 1) * परिमाण(cat_t));
  यदि (g == रिक्त)
    फर्काऊ (REG_ESPACE);
  p->ssize = len / (माप_प्रकार)2 * (माप_प्रकार)3 + (माप_प्रकार)1; /* ugh */
  p->strip = (sop *)सुस्मृति(p->ssize, परिमाण(sop));
  p->slen = 0;
  यदि (p->strip == रिक्त) {
    मुक्त((वर्ण *)g);
    फर्काऊ (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  लागि (i = 0; i < NPAREN; i++) {
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
  (शून्य)स_रखो((वर्ण *)g->catspace, 0, NC * परिमाण(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  यदि (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  अन्यथा यदि (cflags & REG_NOSPEC)
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
  यदि (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  यदि (p->error != 0) /* lose */
    llvm_regfree(preg);
  फर्काऊ (p->error);
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

  लागि (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    जबसम्म (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    यदि (!EAT('|'))
      रोक; /* NOTE BREAK OUT */

    यदि (first) {
      घुसाउ_3(OCH_, conc); /* offset is wrong */
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

  यदि (!first) { /* tail-end fixups */
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
  पूर्णाङ्क गन्ती;
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
    यदि (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    यदि (!SEE(')'))
      p_ere(p, ')');
    यदि (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    रोक;
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
    रोक;
#endif
  पक्ष '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    रोक;
  पक्ष '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    रोक;
  पक्ष '|':
    SETERROR(REG_EMPTY);
    रोक;
  पक्ष '*':
  पक्ष '+':
  पक्ष '?':
    SETERROR(REG_BADRPT);
    रोक;
  पक्ष '.':
    यदि (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    रोक;
  पक्ष '[':
    p_bracket(p);
    रोक;
  पक्ष '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    यदि (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } अन्यथा यदि (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      जबसम्म (MORE() && है_अंक(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } अन्यथा यदि (c == 'n') {
      ordinary(p, '\n');
      रोक;
    } अन्यथा यदि (c == 't') {
      ordinary(p, '\t');
      रोक;
    } अन्यथा यदि (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      वर्ण hexstr[3] = {0};
      वर्ण *hexp;
      पूर्णाङ्क val;
      यदि (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = म_से_दीर्घ(hexstr, &hexp, 16);
        यदि (*hexp == '\0') {
          NEXT2();
          uncased(p, (वर्ण)val);
          रोक;
        }
      }
      ordinary(p, 'x');
      रोक;
    } अन्यथा {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      रोक;
    }

    यदि (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      रोक;
    }

    यदि (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      रोक;
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
    रोक;
  पक्ष '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !है_अंक((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  स्वभाव:
    ordinary(p, c);
    रोक;
  }

  यदि (!MORE())
    फर्काऊ;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  यदि (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    फर्काऊ; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  विकल्प (c) {
  पक्ष '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    घुसाउ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    घुसाउ_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    रोक;
  पक्ष '+':
    घुसाउ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    रोक;
  पक्ष '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घुसाउ_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    रोक;
  पक्ष '{':
    गन्ती = p_गन्ती(p);
    यदि (EAT(',')) {
      यदि (है_अंक((uch)PEEK())) {
        count2 = p_गन्ती(p);
        REQUIRE(गन्ती <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = गन्ती;
    repeat(p, pos, गन्ती, count2);
    यदि (!EAT('}')) { /* error heuristics */
      जबसम्म (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    रोक;
  }

  यदि (!MORE())
    फर्काऊ;
  c = PEEK();
  यदि (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    फर्काऊ;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
स्थिर शून्य p_str(संरचना parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  जबसम्म (MORE())
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

  यदि (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  जबसम्म (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  यदि (wasdollar) { /* oops, that was a trailing anchor */
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
  पूर्णाङ्क गन्ती;
  पूर्णाङ्क count2;
  sopno pos;
  पूर्णाङ्क i;
  sopno subno;
#define BACKSL (1 << अक्षर_बिट)

  pos = HERE(); /* repetition op, if any, covers from here */

  प्रतिज्ञा(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  यदि (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  विकल्प (c) {
  पक्ष '.':
    यदि (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    रोक;
  पक्ष '[':
    p_bracket(p);
    रोक;
  पक्ष BACKSL | '{':
    SETERROR(REG_BADRPT);
    रोक;
  पक्ष BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    यदि (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    यदि (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    यदि (subno < NPAREN) {
      p->pend[subno] = HERE();
      प्रतिज्ञा(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    रोक;
  पक्ष BACKSL | ')': /* should not get here -- must be user */
  पक्ष BACKSL | '}':
    SETERROR(REG_EPAREN);
    रोक;
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
    यदि (p->pend[i] != 0) {
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
    रोक;
  पक्ष '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  स्वभाव:
    ordinary(p, (वर्ण)c);
    रोक;
  }

  यदि (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    घुसाउ_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    घुसाउ_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } अन्यथा यदि (EATTWO('\\', '{')) {
    गन्ती = p_गन्ती(p);
    यदि (EAT(',')) {
      यदि (MORE() && है_अंक((uch)PEEK())) {
        count2 = p_गन्ती(p);
        REQUIRE(गन्ती <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = गन्ती;
    repeat(p, pos, गन्ती, count2);
    यदि (!EATTWO('\\', '}')) { /* error heuristics */
      जबसम्म (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } अन्यथा यदि (c == '$') /* $ (but not \$) ends it */
    फर्काऊ (1);

  फर्काऊ (0);
}

/*
 - p_count - parse a repetition count
 */
स्थिर पूर्णाङ्क /* the value */
p_गन्ती(संरचना parse *p) {
  पूर्णाङ्क गन्ती = 0;
  पूर्णाङ्क ndigits = 0;

  जबसम्म (MORE() && है_अंक((uch)PEEK()) && गन्ती <= DUPMAX) {
    गन्ती = गन्ती * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && गन्ती <= DUPMAX, REG_BADBR);
  फर्काऊ (गन्ती);
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
  यदि (p->end - p->next > 5) {
    यदि (म_भेदन(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      फर्काऊ;
    }
    यदि (म_भेदन(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      फर्काऊ;
    }
  }

  यदि ((cs = allocset(p)) == रिक्त) {
    /* allocset did set error status in p */
    फर्काऊ;
  }

  यदि (EAT('^'))
    invert++; /* make note to invert set at end */
  यदि (EAT(']'))
    CHadd(cs, ']');
  अन्यथा यदि (EAT('-'))
    CHadd(cs, '-');
  जबसम्म (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  यदि (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  यदि (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    फर्काऊ;
  }

  यदि (p->g->cflags & REG_ICASE) {
    पूर्णाङ्क i;
    पूर्णाङ्क ci;

    लागि (i = p->g->csetsize - 1; i >= 0; i--)
      यदि (CHIN(cs, i) && है_अक्षर(i)) {
        ci = othercase(i);
        यदि (ci != i)
          CHadd(cs, ci);
      }
    यदि (cs->multis != रिक्त)
      mccase(p, cs);
  }
  यदि (invert) {
    पूर्णाङ्क i;

    लागि (i = p->g->csetsize - 1; i >= 0; i--)
      यदि (CHIN(cs, i))
        CHsub(cs, i);
      अन्यथा
        CHadd(cs, i);
    यदि (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    यदि (cs->multis != रिक्त)
      mcinvert(p, cs);
  }

  प्रतिज्ञा(cs->multis == रिक्त); /* xxx */

  यदि (nch(p, cs) == 1) { /* optimize singleton sets */
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
  वर्ण c;
  वर्ण start, finish;
  पूर्णाङ्क i;

  /* classify what we've got */
  विकल्प ((MORE()) ? PEEK() : '\0') {
  पक्ष '[':
    c = (MORE2()) ? PEEK2() : '\0';
    रोक;
  पक्ष '-':
    SETERROR(REG_ERANGE);
    फर्काऊ; /* NOTE RETURN */
    रोक;
  स्वभाव:
    c = '\0';
    रोक;
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
    रोक;
  पक्ष '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    रोक;
  स्वभाव: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_चिह्न(p);
    यदि (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      यदि (EAT('-'))
        finish = '-';
      अन्यथा
        finish = p_b_चिह्न(p);
    } अन्यथा {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    लागि (i = start; i <= finish; i++)
      CHadd(cs, i);
    रोक;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
स्थिर शून्य p_b_cclass(संरचना parse *p, cset *cs) {
  अचर वर्ण *sp = p->next;
  संरचना cclass *cp;
  माप_प्रकार len;
  अचर वर्ण *u;
  वर्ण c;

  जबसम्म (MORE() && है_अक्षर((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  लागि (cp = cclasses; cp->name != रिक्त; cp++)
    यदि (म_भेदन(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      रोक;
  यदि (cp->name == रिक्त) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    फर्काऊ;
  }

  u = cp->chars;
  जबसम्म ((c = *u++) != '\0')
    CHadd(cs, c);
  लागि (u = cp->multis; *u != '\0'; u += म_माप(u) + 1)
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
p_b_चिह्न(संरचना parse *p) {
  वर्ण मान_2;

  REQUIRE(MORE(), REG_EBRACK);
  यदि (!EATTWO('[', '.'))
    फर्काऊ (GETNEXT());

  /* collating symbol */
  मान_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  फर्काऊ (मान_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
स्थिर वर्ण                              /* value of collating element */
p_b_coll_elem(संरचना parse *p, पूर्णाङ्क endc) /* name ended by endc,']' */
{
  अचर वर्ण *sp = p->next;
  संरचना cname *cp;
  माप_प्रकार len;

  जबसम्म (MORE() && !SEETWO(endc, ']'))
    NEXT();
  यदि (!MORE()) {
    SETERROR(REG_EBRACK);
    फर्काऊ (0);
  }
  len = p->next - sp;
  लागि (cp = cnames; cp->name != रिक्त; cp++)
    यदि (म_भेदन(cp->name, sp, len) == 0 && म_माप(cp->name) == len)
      फर्काऊ (cp->code); /* known name */
  यदि (len == 1)
    फर्काऊ (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  फर्काऊ (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
स्थिर वर्ण /* if no counterpart, return ch */
othercase(पूर्णाङ्क ch) {
  ch = (uch)ch;
  प्रतिज्ञा(है_अक्षर(ch));
  यदि (है_बड़ा(ch))
    फर्काऊ ((uch)छोटे(ch));
  अन्यथा यदि (है_छोटा(ch))
    फर्काऊ ((uch)बड़े(ch));
  अन्यथा /* peculiar, but could happen */
    फर्काऊ (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
स्थिर शून्य bothcases(संरचना parse *p, पूर्णाङ्क ch) {
  अचर वर्ण *oldnext = p->next;
  अचर वर्ण *oldend = p->end;
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
  यदि (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
स्थिर शून्य ordinary(संरचना parse *p, पूर्णाङ्क ch) {
  यदि ((p->g->cflags & REG_ICASE) && है_अक्षर((uch)ch) && othercase(ch) != ch)
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
  अचर वर्ण *oldnext = p->next;
  अचर वर्ण *oldend = p->end;
  स्थिर अचर वर्ण bracket[4] = {'^', '\n', ']', '\0'};

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

  यदि (p->error != 0) /* head off possible runaway recursion */
    फर्काऊ;

  प्रतिज्ञा(from <= to);

  विकल्प (REP(MAP(from), MAP(to))) {
  पक्ष REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    रोक;
  पक्ष REP(0, 1):   /* as x{1,1}? */
  पक्ष REP(0, N):   /* as x{1,n}? */
  पक्ष REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घुसाउ_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    रोक;
  पक्ष REP(1, 1): /* trivial case */
    /* done */
    रोक;
  पक्ष REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    घुसाउ_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    प्रतिज्ञा(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    रोक;
  पक्ष REP(1, INF): /* as x+ */
    घुसाउ_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    रोक;
  पक्ष REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    रोक;
  पक्ष REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    रोक;
  स्वभाव:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    रोक;
  }
}

/*
 - seterr - set an error condition
 */
स्थिर पूर्णाङ्क /* useless but makes type checking happy */
seterr(संरचना parse *p, पूर्णाङ्क e) {
  यदि (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  फर्काऊ (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
स्थिर cset *allocset(संरचना parse *p) {
  पूर्णाङ्क no = p->g->ncsets++;
  माप_प्रकार nc;
  माप_प्रकार nbytes;
  cset *cs;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णाङ्क i;

  यदि (no >= p->ncsalloc) { /* need another column of space */
    शून्य *ptr;

    p->ncsalloc += अक्षर_बिट;
    nc = p->ncsalloc;
    यदि (nc > SIZE_MAX / परिमाण(cset))
      गच्छ nomem;
    प्रतिज्ञा(nc % अक्षर_बिट == 0);
    nbytes = nc / अक्षर_बिट * css;

    ptr = (cset *)पुनः_स्मृति((वर्ण *)p->g->sets, nc * परिमाण(cset));
    यदि (ptr == रिक्त)
      गच्छ nomem;
    p->g->sets = ptr;

    ptr = (uch *)पुनः_स्मृति((वर्ण *)p->g->setbits, nbytes);
    यदि (ptr == रिक्त)
      गच्छ nomem;
    p->g->setbits = ptr;

    लागि (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / अक्षर_बिट);

    (शून्य)स_रखो((वर्ण *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  यदि (p->g->sets == रिक्त || p->g->setbits == रिक्त)
    गच्छ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / अक्षर_बिट);
  cs->mask = 1 << ((no) % अक्षर_बिट);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = रिक्त;

  फर्काऊ (cs);
nomem:
  मुक्त(p->g->sets);
  p->g->sets = रिक्त;
  मुक्त(p->g->setbits);
  p->g->setbits = रिक्त;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  फर्काऊ (रिक्त);
}

/*
 - freeset - free a now-unused set
 */
स्थिर शून्य freeset(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  cset *शीर्ष_2 = &p->g->sets[p->g->ncsets];
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  लागि (i = 0; i < css; i++)
    CHsub(cs, i);
  यदि (cs == शीर्ष_2 - 1) /* recover only the easy case */
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
  माप_प्रकार i;
  cset *शीर्ष_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  /* look for an earlier one which is the same */
  लागि (cs2 = &p->g->sets[0]; cs2 < शीर्ष_2; cs2++)
    यदि (cs2->hash == h && cs2 != cs) {
      /* maybe */
      लागि (i = 0; i < css; i++)
        यदि (!!CHIN(cs2, i) != !!CHIN(cs, i))
          रोक; /* no */
      यदि (i == css)
        रोक; /* yes */
    }

  यदि (cs2 < शीर्ष_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  फर्काऊ ((पूर्णाङ्क)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
स्थिर पूर्णाङ्क /* character; there is no "none" value */
firstch(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  लागि (i = 0; i < css; i++)
    यदि (CHIN(cs, i))
      फर्काऊ ((वर्ण)i);
  प्रतिज्ञा(never);
  फर्काऊ (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
स्थिर पूर्णाङ्क nch(संरचना parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णाङ्क n = 0;

  लागि (i = 0; i < css; i++)
    यदि (CHIN(cs, i))
      n++;
  फर्काऊ (n);
}

/*
 - mcadd - add a collating element to a cset
 */
स्थिर शून्य mcadd(संरचना parse *p, cset *cs, अचर वर्ण *cp) {
  माप_प्रकार oldend = cs->smultis;
  शून्य *np;

  cs->smultis += म_माप(cp) + 1;
  np = पुनः_स्मृति(cs->multis, cs->smultis);
  यदि (np == रिक्त) {
    यदि (cs->multis)
      मुक्त(cs->multis);
    cs->multis = रिक्त;
    SETERROR(REG_ESPACE);
    फर्काऊ;
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
  पूर्णाङ्क ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc = (uch)c;

  लागि (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    यदि (col[uc] != 0)
      फर्काऊ (1);
  फर्काऊ (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
स्थिर पूर्णाङ्क /* predicate */
samesets(संरचना re_guts *g, पूर्णाङ्क c1, पूर्णाङ्क c2) {
  uch *col;
  पूर्णाङ्क i;
  पूर्णाङ्क ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc1 = (uch)c1;
  अचिन्हित uc2 = (uch)c2;

  लागि (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    यदि (col[uc1] != col[uc2])
      फर्काऊ (0);
  फर्काऊ (1);
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
  यदि (p->error != 0)
    फर्काऊ;

  लागि (c = अक्षर_न्यून; c <= अक्षर_उच्च; c++)
    यदि (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      लागि (c2 = c + 1; c2 <= अक्षर_उच्च; c2++)
        यदि (cats[c2] == 0 && samesets(g, c, c2))
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
  यदि (len == 0)
    फर्काऊ (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  प्रतिज्ञा(p->ssize >= p->slen + len);
  (शून्य)स_हटाओ((वर्ण *)(p->strip + p->slen), (वर्ण *)(p->strip + start),
                (माप_प्रकार)len * परिमाण(sop));
  p->slen += len;
  फर्काऊ (ret);
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
  यदि (p->error != 0)
    फर्काऊ;

  /* deal with oversize operands ("can't happen", more or less) */
  प्रतिज्ञा(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  यदि (p->slen >= p->ssize)
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
  पूर्णाङ्क i;

  /* avoid making error situations worse */
  यदि (p->error != 0)
    फर्काऊ;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  प्रतिज्ञा(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  प्रतिज्ञा(pos > 0);
  लागि (i = 1; i < NPAREN; i++) {
    यदि (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    यदि (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  स_हटाओ((वर्ण *)&p->strip[pos + 1], (वर्ण *)&p->strip[pos],
          (HERE() - pos - 1) * परिमाण(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
स्थिर शून्य dofwd(संरचना parse *p, sopno pos, sop मान_2) {
  /* avoid making error situations worse */
  यदि (p->error != 0)
    फर्काऊ;

  प्रतिज्ञा(मान_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | मान_2;
}

/*
 - enlarge - enlarge the strip
 */
स्थिर शून्य enlarge(संरचना parse *p, sopno size) {
  sop *sp;

  यदि (p->ssize >= size)
    फर्काऊ;

  यदि ((uintptr_t)size > SIZE_MAX / परिमाण(sop)) {
    SETERROR(REG_ESPACE);
    फर्काऊ;
  }

  sp = (sop *)पुनः_स्मृति(p->strip, size * परिमाण(sop));
  यदि (sp == रिक्त) {
    SETERROR(REG_ESPACE);
    फर्काऊ;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
स्थिर शून्य stripsnug(संरचना parse *p, संरचना re_guts *g) {
  g->nstates = p->slen;
  यदि ((uintptr_t)p->slen > SIZE_MAX / परिमाण(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    फर्काऊ;
  }

  g->strip = (sop *)पुनः_स्मृति((वर्ण *)p->strip, p->slen * परिमाण(sop));
  यदि (g->strip == रिक्त) {
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
  यदि (p->error != 0)
    फर्काऊ;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  गर {
    s = *scan++;
    विकल्प (OP(s)) {
    पक्ष OCHAR:        /* sequence member */
      यदि (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      रोक;
    पक्ष OPLUS_: /* things that don't break one */
    पक्ष OLPAREN:
    पक्ष ORPAREN:
      रोक;
    पक्ष OQUEST_: /* things that must be skipped */
    पक्ष OCH_:
      scan--;
      गर {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        यदि (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          फर्काऊ;
        }
      } जबसम्म (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    स्वभाव:                  /* things that break a sequence */
      यदि (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      रोक;
    }
  } जबसम्म (OP(s) != OEND);

  यदि (g->mlen == 0) /* there isn't one */
    फर्काऊ;

  /* turn it into a character string */
  g->must = दो_स्मृति((माप_प्रकार)g->mlen + 1);
  यदि (g->must == रिक्त) { /* argh; just forget it */
    g->mlen = 0;
    फर्काऊ;
  }
  cp = g->must;
  scan = start;
  लागि (i = g->mlen; i > 0; i--) {
    जबसम्म (OP(s = *scan++) != OCHAR)
      जारी;
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

  यदि (p->error != 0)
    फर्काऊ (0); /* there may not be an OEND */

  scan = g->strip + 1;
  गर {
    s = *scan++;
    विकल्प (OP(s)) {
    पक्ष OPLUS_:
      plusnest++;
      रोक;
    पक्ष O_PLUS:
      यदि (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      रोक;
    }
  } जबसम्म (OP(s) != OEND);
  यदि (plusnest != 0)
    g->iflags |= REGEX_BAD;
  फर्काऊ (maxnest);
}
