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

#అన్తర్భావ "regex_impl.h"
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "regex2.h"
#అన్తర్భావ "regutils.h"

#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Compiler.h"

/* character-class table */
స్థిర నిర్మాణం cclass {
  స్థిరరాశి అక్షరం *name;
  స్థిరరాశి అక్షరం *chars;
  స్థిరరాశి అక్షరం *multis;
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
    {ఖాళీ, 0, ""}};

/* character-name table */
స్థిర నిర్మాణం cname {
  స్థిరరాశి అక్షరం *name;
  అక్షరం సంకేతం_3;
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
              {ఖాళీ, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
నిర్మాణం parse {
  స్థిరరాశి అక్షరం *తదుపరి; /* next character in RE */
  స్థిరరాశి అక్షరం *చివర;  /* end of string (-> NUL normally) */
  పూర్ణాంకం error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  పూర్ణాంకం ncsalloc;     /* number of csets allocated */
  నిర్మాణం re_guts *g;
#నిర్వచించు NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

స్థిర శూన్య p_ere(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర శూన్య p_ere_exp(నిర్మాణం parse *);
స్థిర శూన్య p_str(నిర్మాణం parse *);
స్థిర శూన్య p_bre(నిర్మాణం parse *, పూర్ణాంకం, పూర్ణాంకం);
స్థిర పూర్ణాంకం p_simp_re(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర పూర్ణాంకం p_లెక్క(నిర్మాణం parse *);
స్థిర శూన్య p_bracket(నిర్మాణం parse *);
స్థిర శూన్య p_b_పదం(నిర్మాణం parse *, cset *);
స్థిర శూన్య p_b_cclass(నిర్మాణం parse *, cset *);
స్థిర శూన్య p_b_eclass(నిర్మాణం parse *, cset *);
స్థిర అక్షరం p_b_చిహ్నం(నిర్మాణం parse *);
స్థిర అక్షరం p_b_coll_elem(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర అక్షరం othercase(పూర్ణాంకం);
స్థిర శూన్య bothcases(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర శూన్య uncased(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర శూన్య ordinary(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర శూన్య nonnewline(నిర్మాణం parse *);
స్థిర శూన్య repeat(నిర్మాణం parse *, sopno, పూర్ణాంకం, పూర్ణాంకం);
స్థిర పూర్ణాంకం seterr(నిర్మాణం parse *, పూర్ణాంకం);
స్థిర cset *allocset(నిర్మాణం parse *);
స్థిర శూన్య freeset(నిర్మాణం parse *, cset *);
స్థిర పూర్ణాంకం freezeset(నిర్మాణం parse *, cset *);
స్థిర పూర్ణాంకం firstch(నిర్మాణం parse *, cset *);
స్థిర పూర్ణాంకం nch(నిర్మాణం parse *, cset *);
స్థిర శూన్య mcadd(నిర్మాణం parse *, cset *, స్థిరరాశి అక్షరం *);
స్థిర శూన్య mcinvert(నిర్మాణం parse *, cset *);
స్థిర శూన్య mccase(నిర్మాణం parse *, cset *);
స్థిర పూర్ణాంకం isinsets(నిర్మాణం re_guts *, పూర్ణాంకం);
స్థిర పూర్ణాంకం samesets(నిర్మాణం re_guts *, పూర్ణాంకం, పూర్ణాంకం);
స్థిర శూన్య categorize(నిర్మాణం parse *, నిర్మాణం re_guts *);
స్థిర sopno dupl(నిర్మాణం parse *, sopno, sopno);
స్థిర శూన్య doemit(నిర్మాణం parse *, sop, size_t);
స్థిర శూన్య doinsert(నిర్మాణం parse *, sop, size_t, sopno);
స్థిర శూన్య dofwd(నిర్మాణం parse *, sopno, sop);
స్థిర శూన్య enlarge(నిర్మాణం parse *, sopno);
స్థిర శూన్య stripsnug(నిర్మాణం parse *, నిర్మాణం re_guts *);
స్థిర శూన్య findmust(నిర్మాణం parse *, నిర్మాణం re_guts *);
స్థిర sopno pluscount(నిర్మాణం parse *, నిర్మాణం re_guts *);

స్థిర అక్షరం nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#నిర్వచించు PEEK() (*p->తదుపరి)
#నిర్వచించు PEEK2() (*(p->తదుపరి + 1))
#నిర్వచించు మరిన్ని() (p->చివర - p->తదుపరి > 0)
#నిర్వచించు MORE2() (p->చివర - p->తదుపరి > 1)
#నిర్వచించు SEE(c) (మరిన్ని() && PEEK() == (c))
#నిర్వచించు SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#నిర్వచించు EAT(c) ((SEE(c)) ? (తదుపరి_3(), 1) : 0)
#నిర్వచించు EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#నిర్వచించు తదుపరి_3() (p->తదుపరి++)
#నిర్వచించు NEXT2() (p->తదుపరి += 2)
#నిర్వచించు NEXTn(n) (p->తదుపరి += (n))
#నిర్వచించు GETNEXT() (*p->తదుపరి++)
#నిర్వచించు SETERROR(e) seterr(p, (e))
#నిర్వచించు REQUIRE(co, e) (శూన్య)((co) || SETERROR(e))
#నిర్వచించు MUSTSEE(c, e) (REQUIRE(మరిన్ని() && PEEK() == (c), e))
#నిర్వచించు MUSTEAT(c, e) (REQUIRE(మరిన్ని() && GETNEXT() == (c), e))
#నిర్వచించు MUSTNOTSEE(c, e) (REQUIRE(!మరిన్ని() || PEEK() != (c), e))
#నిర్వచించు EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#నిర్వచించు చొప్పించు_3(op, pos) doinsert(p, (sop)(op), ఇక్కడ() - (pos) + 1, pos)
#నిర్వచించు AHEAD(pos) dofwd(p, pos, ఇక్కడ() - (pos))
#నిర్వచించు ASTERN(sop, pos) EMIT(sop, ఇక్కడ() - pos)
#నిర్వచించు ఇక్కడ() (p->slen)
#నిర్వచించు THERE() (p->slen - 1)
#నిర్వచించు THERETHERE() (p->slen - 2)
#నిర్వచించు DROP(n) (p->slen -= (n))

#నిర్వచితమైతే _POSIX2_RE_DUP_MAX
#నిర్వచించు DUPMAX _POSIX2_RE_DUP_MAX
#లేకపోతే
#నిర్వచించు DUPMAX 255
#అయితే_ముగింపు
#నిర్వచించు REGINFINITY (DUPMAX + 1)

#నిర్వచితం_కాకపోతే NDEBUG
స్థిర పూర్ణాంకం never = 0; /* for use in asserts; shuts lint up */
#లేకపోతే
#నిర్వచించు never 0 /* some <assert.h>s have bugs too */
#అయితే_ముగింపు

/*
 - llvm_regcomp - interface for parser and compilation
 */
పూర్ణాంకం /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, స్థిరరాశి అక్షరం *నమూనా_2, పూర్ణాంకం cflags) {
  నిర్మాణం parse pa;
  నిర్మాణం re_guts *g;
  నిర్మాణం parse *p = &pa;
  పూర్ణాంకం i;
  size_t len;
#నిర్వచితమైతే REDEBUG
#నిర్వచించు GOODFLAGS(f) (f)
#లేకపోతే
#నిర్వచించు GOODFLAGS(f) ((f) & ~REG_DUMP)
#అయితే_ముగింపు

  cflags = GOODFLAGS(cflags);
  ఒకవేళ ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    తిరిగి (REG_INVARG);

  ఒకవేళ (cflags & REG_PEND) {
    ఒకవేళ (preg->re_endp < నమూనా_2)
      తిరిగి (REG_INVARG);
    len = preg->re_endp - నమూనా_2;
  } లేకపోతే {
    len = అక్షరశ్రేణి_పొడవు(నమూనా_2);
  }

  /* do the mallocs early so failure handling is easy */
  g = (నిర్మాణం re_guts *)జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం re_guts) +
                               (NC - 1) * పరిమాణం(cat_t));
  ఒకవేళ (g == ఖాళీ)
    తిరిగి (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)సున్నా_జ్ఞాపకం_ఇవ్వు(p->ssize, పరిమాణం(sop));
  p->slen = 0;
  ఒకవేళ (p->strip == ఖాళీ) {
    విడుదల((అక్షరం *)g);
    తిరిగి (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->తదుపరి = నమూనా_2;
  p->చివర = p->తదుపరి + len;
  p->error = 0;
  p->ncsalloc = 0;
  కొరకు (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ఖాళీ;
  g->setbits = ఖాళీ;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ఖాళీ;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (శూన్య)memset((అక్షరం *)g->catspace, 0, NC * పరిమాణం(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  ఒకవేళ (cflags & REG_EXTENDED)
    p_ere(p, బహిర్గతం_3);
  లేకపోతే ఒకవేళ (cflags & REG_NOSPEC)
    p_str(p);
  లేకపోతే
    p_bre(p, బహిర్గతం_3, బహిర్గతం_3);
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
#నిర్వచితం_కాకపోతే REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  ఒకవేళ (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#అయితే_ముగింపు

  /* win or lose, we're done */
  ఒకవేళ (p->error != 0) /* lose */
    llvm_regfree(preg);
  తిరిగి (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
స్థిర శూన్య p_ere(నిర్మాణం parse *p,
                  పూర్ణాంకం stop) /* character this ERE should end at */
{
  అక్షరం c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  పూర్ణాంకం మొదటి_2 = 1; /* is this the first alternative? */

  కొరకు (;;) {
    /* do a bunch of concatenated expressions */
    conc = ఇక్కడ();
    అయితే (మరిన్ని() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(ఇక్కడ() != conc, REG_ఖాళీలు); /* require nonempty */

    ఒకవేళ (!EAT('|'))
      ఆపు; /* NOTE BREAK OUT */

    ఒకవేళ (మొదటి_2) {
      చొప్పించు_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      మొదటి_2 = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = ఇక్కడ();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  ఒకవేళ (!మొదటి_2) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  ప్రతిజ్ఞా(!మరిన్ని() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
స్థిర శూన్య p_ere_exp(నిర్మాణం parse *p) {
  అక్షరం c;
  sopno pos;
  పూర్ణాంకం లెక్క;
  పూర్ణాంకం count2;
  పూర్ణాంకం backrefnum;
  sopno subno;
  పూర్ణాంకం wascaret = 0;

  ప్రతిజ్ఞా(మరిన్ని()); /* caller should have ensured this */
  c = GETNEXT();

  pos = ఇక్కడ();
  ఎంపిక (c) {
  స్థితి '(':
    REQUIRE(మరిన్ని(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    ఒకవేళ (subno < NPAREN)
      p->pbegin[subno] = ఇక్కడ();
    EMIT(OLPAREN, subno);
    ఒకవేళ (!SEE(')'))
      p_ere(p, ')');
    ఒకవేళ (subno < NPAREN) {
      p->pend[subno] = ఇక్కడ();
      ప్రతిజ్ఞా(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    ఆపు;
#నిర్వచితం_కాకపోతే POSIX_MISTAKE
  స్థితి ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    ఆపు;
#అయితే_ముగింపు
  స్థితి '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    ఆపు;
  స్థితి '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    ఆపు;
  స్థితి '|':
    SETERROR(REG_ఖాళీలు);
    ఆపు;
  స్థితి '*':
  స్థితి '+':
  స్థితి '?':
    SETERROR(REG_BADRPT);
    ఆపు;
  స్థితి '.':
    ఒకవేళ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    లేకపోతే
      EMIT(OANY, 0);
    ఆపు;
  స్థితి '[':
    p_bracket(p);
    ఆపు;
  స్థితి '\\':
    REQUIRE(మరిన్ని(), REG_EESCAPE);
    c = GETNEXT();
    ఒకవేళ (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } లేకపోతే ఒకవేళ (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      అయితే (మరిన్ని() && అంకెనా(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } లేకపోతే ఒకవేళ (c == 'n') {
      ordinary(p, '\n');
      ఆపు;
    } లేకపోతే ఒకవేళ (c == 't') {
      ordinary(p, '\t');
      ఆపు;
    } లేకపోతే ఒకవేళ (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      అక్షరం hexstr[3] = {0};
      అక్షరం *hexp;
      పూర్ణాంకం val;
      ఒకవేళ (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        ఒకవేళ (*hexp == '\0') {
          NEXT2();
          uncased(p, (అక్షరం)val);
          ఆపు;
        }
      }
      ordinary(p, 'x');
      ఆపు;
    } లేకపోతే {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      ఆపు;
    }

    ఒకవేళ (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      ఆపు;
    }

    ఒకవేళ (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      ఆపు;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    ప్రతిజ్ఞా(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    ప్రతిజ్ఞా(p->pbegin[backrefnum] != 0);
    ప్రతిజ్ఞా(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    ప్రతిజ్ఞా(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (శూన్య)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_వెనుక, backrefnum);
    p->g->backrefs = 1;
    ఆపు;
  స్థితి '{': /* okay as ordinary except if digit follows */
    REQUIRE(!మరిన్ని() || !అంకెనా((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  అప్రమేయ:
    ordinary(p, c);
    ఆపు;
  }

  ఒకవేళ (!మరిన్ని())
    తిరిగి;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  ఒకవేళ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && అంకెనా((uch)PEEK2()))))
    తిరిగి; /* no repetition, we're done */
  తదుపరి_3();

  REQUIRE(!wascaret, REG_BADRPT);
  ఎంపిక (c) {
  స్థితి '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    చొప్పించు_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    చొప్పించు_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    ఆపు;
  స్థితి '+':
    చొప్పించు_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ఆపు;
  స్థితి '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    చొప్పించు_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    ఆపు;
  స్థితి '{':
    లెక్క = p_లెక్క(p);
    ఒకవేళ (EAT(',')) {
      ఒకవేళ (అంకెనా((uch)PEEK())) {
        count2 = p_లెక్క(p);
        REQUIRE(లెక్క <= count2, REG_BADBR);
      } లేకపోతే /* single number with comma */
        count2 = REGINFINITY;
    } లేకపోతే /* just a single number */
      count2 = లెక్క;
    repeat(p, pos, లెక్క, count2);
    ఒకవేళ (!EAT('}')) { /* error heuristics */
      అయితే (మరిన్ని() && PEEK() != '}')
        తదుపరి_3();
      REQUIRE(మరిన్ని(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    ఆపు;
  }

  ఒకవేళ (!మరిన్ని())
    తిరిగి;
  c = PEEK();
  ఒకవేళ (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && అంకెనా((uch)PEEK2()))))
    తిరిగి;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
స్థిర శూన్య p_str(నిర్మాణం parse *p) {
  REQUIRE(మరిన్ని(), REG_ఖాళీలు);
  అయితే (మరిన్ని())
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
స్థిర శూన్య p_bre(నిర్మాణం parse *p, పూర్ణాంకం end1, /* first terminating character */
                  పూర్ణాంకం end2)                  /* second terminating character */
{
  sopno ప్రారంభం_2 = ఇక్కడ();
  పూర్ణాంకం మొదటి_2 = 1; /* first subexpression? */
  పూర్ణాంకం wasdollar = 0;

  ఒకవేళ (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  అయితే (మరిన్ని() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, మొదటి_2);
    మొదటి_2 = 0;
  }
  ఒకవేళ (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(ఇక్కడ() != ప్రారంభం_2, REG_ఖాళీలు); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
స్థిర పూర్ణాంకం /* was the simple RE an unbackslashed $? */
p_simp_re(నిర్మాణం parse *p,
          పూర్ణాంకం starordinary) /* is a leading * an ordinary character? */
{
  పూర్ణాంకం c;
  పూర్ణాంకం లెక్క;
  పూర్ణాంకం count2;
  sopno pos;
  పూర్ణాంకం i;
  sopno subno;
#నిర్వచించు BACKSL (1 << CHAR_BIT)

  pos = ఇక్కడ(); /* repetition op, if any, covers from here */

  ప్రతిజ్ఞా(మరిన్ని()); /* caller should have ensured this */
  c = GETNEXT();
  ఒకవేళ (c == '\\') {
    REQUIRE(మరిన్ని(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  ఎంపిక (c) {
  స్థితి '.':
    ఒకవేళ (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    లేకపోతే
      EMIT(OANY, 0);
    ఆపు;
  స్థితి '[':
    p_bracket(p);
    ఆపు;
  స్థితి BACKSL | '{':
    SETERROR(REG_BADRPT);
    ఆపు;
  స్థితి BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    ఒకవేళ (subno < NPAREN)
      p->pbegin[subno] = ఇక్కడ();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    ఒకవేళ (మరిన్ని() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    ఒకవేళ (subno < NPAREN) {
      p->pend[subno] = ఇక్కడ();
      ప్రతిజ్ఞా(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    ఆపు;
  స్థితి BACKSL | ')': /* should not get here -- must be user */
  స్థితి BACKSL | '}':
    SETERROR(REG_EPAREN);
    ఆపు;
  స్థితి BACKSL | '1':
  స్థితి BACKSL | '2':
  స్థితి BACKSL | '3':
  స్థితి BACKSL | '4':
  స్థితి BACKSL | '5':
  స్థితి BACKSL | '6':
  స్థితి BACKSL | '7':
  స్థితి BACKSL | '8':
  స్థితి BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    ప్రతిజ్ఞా(i < NPAREN);
    ఒకవేళ (p->pend[i] != 0) {
      ప్రతిజ్ఞా(i <= p->g->nsub);
      EMIT(OBACK_, i);
      ప్రతిజ్ఞా(p->pbegin[i] != 0);
      ప్రతిజ్ఞా(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      ప్రతిజ్ఞా(OP(p->strip[p->pend[i]]) == ORPAREN);
      (శూన్య)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_వెనుక, i);
    } లేకపోతే {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    ఆపు;
  స్థితి '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  అప్రమేయ:
    ordinary(p, (అక్షరం)c);
    ఆపు;
  }

  ఒకవేళ (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    చొప్పించు_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    చొప్పించు_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } లేకపోతే ఒకవేళ (EATTWO('\\', '{')) {
    లెక్క = p_లెక్క(p);
    ఒకవేళ (EAT(',')) {
      ఒకవేళ (మరిన్ని() && అంకెనా((uch)PEEK())) {
        count2 = p_లెక్క(p);
        REQUIRE(లెక్క <= count2, REG_BADBR);
      } లేకపోతే /* single number with comma */
        count2 = REGINFINITY;
    } లేకపోతే /* just a single number */
      count2 = లెక్క;
    repeat(p, pos, లెక్క, count2);
    ఒకవేళ (!EATTWO('\\', '}')) { /* error heuristics */
      అయితే (మరిన్ని() && !SEETWO('\\', '}'))
        తదుపరి_3();
      REQUIRE(మరిన్ని(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } లేకపోతే ఒకవేళ (c == '$') /* $ (but not \$) ends it */
    తిరిగి (1);

  తిరిగి (0);
}

/*
 - p_count - parse a repetition count
 */
స్థిర పూర్ణాంకం /* the value */
p_లెక్క(నిర్మాణం parse *p) {
  పూర్ణాంకం లెక్క = 0;
  పూర్ణాంకం ndigits = 0;

  అయితే (మరిన్ని() && అంకెనా((uch)PEEK()) && లెక్క <= DUPMAX) {
    లెక్క = లెక్క * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && లెక్క <= DUPMAX, REG_BADBR);
  తిరిగి (లెక్క);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
స్థిర శూన్య p_bracket(నిర్మాణం parse *p) {
  cset *cs;
  పూర్ణాంకం invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  ఒకవేళ (p->చివర - p->తదుపరి > 5) {
    ఒకవేళ (అక్షరశ్రేణి_పోలిక(p->తదుపరి, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      తిరిగి;
    }
    ఒకవేళ (అక్షరశ్రేణి_పోలిక(p->తదుపరి, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      తిరిగి;
    }
  }

  ఒకవేళ ((cs = allocset(p)) == ఖాళీ) {
    /* allocset did set error status in p */
    తిరిగి;
  }

  ఒకవేళ (EAT('^'))
    invert++; /* make note to invert set at end */
  ఒకవేళ (EAT(']'))
    CHadd(cs, ']');
  లేకపోతే ఒకవేళ (EAT('-'))
    CHadd(cs, '-');
  అయితే (మరిన్ని() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_పదం(p, cs);
  ఒకవేళ (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  ఒకవేళ (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    తిరిగి;
  }

  ఒకవేళ (p->g->cflags & REG_ICASE) {
    పూర్ణాంకం i;
    పూర్ణాంకం ci;

    కొరకు (i = p->g->csetsize - 1; i >= 0; i--)
      ఒకవేళ (CHIN(cs, i) && అక్షరమా(i)) {
        ci = othercase(i);
        ఒకవేళ (ci != i)
          CHadd(cs, ci);
      }
    ఒకవేళ (cs->multis != ఖాళీ)
      mccase(p, cs);
  }
  ఒకవేళ (invert) {
    పూర్ణాంకం i;

    కొరకు (i = p->g->csetsize - 1; i >= 0; i--)
      ఒకవేళ (CHIN(cs, i))
        CHsub(cs, i);
      లేకపోతే
        CHadd(cs, i);
    ఒకవేళ (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    ఒకవేళ (cs->multis != ఖాళీ)
      mcinvert(p, cs);
  }

  ప్రతిజ్ఞా(cs->multis == ఖాళీ); /* xxx */

  ఒకవేళ (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } లేకపోతే {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
స్థిర శూన్య p_b_పదం(నిర్మాణం parse *p, cset *cs) {
  అక్షరం c;
  అక్షరం ప్రారంభం_2, ముగింపు_2;
  పూర్ణాంకం i;

  /* classify what we've got */
  ఎంపిక ((మరిన్ని()) ? PEEK() : '\0') {
  స్థితి '[':
    c = (MORE2()) ? PEEK2() : '\0';
    ఆపు;
  స్థితి '-':
    SETERROR(REG_ERANGE);
    తిరిగి; /* NOTE RETURN */
    ఆపు;
  అప్రమేయ:
    c = '\0';
    ఆపు;
  }

  ఎంపిక (c) {
  స్థితి ':': /* character class */
    NEXT2();
    REQUIRE(మరిన్ని(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(మరిన్ని(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    ఆపు;
  స్థితి '=': /* equivalence class */
    NEXT2();
    REQUIRE(మరిన్ని(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(మరిన్ని(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    ఆపు;
  అప్రమేయ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    ప్రారంభం_2 = p_b_చిహ్నం(p);
    ఒకవేళ (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      తదుపరి_3();
      ఒకవేళ (EAT('-'))
        ముగింపు_2 = '-';
      లేకపోతే
        ముగింపు_2 = p_b_చిహ్నం(p);
    } లేకపోతే {
      ముగింపు_2 = ప్రారంభం_2;
    }
    /* xxx what about signed chars here... */
    REQUIRE(ప్రారంభం_2 <= ముగింపు_2, REG_ERANGE);
    కొరకు (i = ప్రారంభం_2; i <= ముగింపు_2; i++)
      CHadd(cs, i);
    ఆపు;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
స్థిర శూన్య p_b_cclass(నిర్మాణం parse *p, cset *cs) {
  స్థిరరాశి అక్షరం *sp = p->తదుపరి;
  నిర్మాణం cclass *cp;
  size_t len;
  స్థిరరాశి అక్షరం *u;
  అక్షరం c;

  అయితే (మరిన్ని() && అక్షరమా((uch)PEEK()))
    తదుపరి_3();
  len = p->తదుపరి - sp;
  కొరకు (cp = cclasses; cp->name != ఖాళీ; cp++)
    ఒకవేళ (అక్షరశ్రేణి_పోలిక(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      ఆపు;
  ఒకవేళ (cp->name == ఖాళీ) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    తిరిగి;
  }

  u = cp->chars;
  అయితే ((c = *u++) != '\0')
    CHadd(cs, c);
  కొరకు (u = cp->multis; *u != '\0'; u += అక్షరశ్రేణి_పొడవు(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
స్థిర శూన్య p_b_eclass(నిర్మాణం parse *p, cset *cs) {
  అక్షరం c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
స్థిర అక్షరం /* value of symbol */
p_b_చిహ్నం(నిర్మాణం parse *p) {
  అక్షరం విలువ_2;

  REQUIRE(మరిన్ని(), REG_EBRACK);
  ఒకవేళ (!EATTWO('[', '.'))
    తిరిగి (GETNEXT());

  /* collating symbol */
  విలువ_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  తిరిగి (విలువ_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
స్థిర అక్షరం                              /* value of collating element */
p_b_coll_elem(నిర్మాణం parse *p, పూర్ణాంకం endc) /* name ended by endc,']' */
{
  స్థిరరాశి అక్షరం *sp = p->తదుపరి;
  నిర్మాణం cname *cp;
  size_t len;

  అయితే (మరిన్ని() && !SEETWO(endc, ']'))
    తదుపరి_3();
  ఒకవేళ (!మరిన్ని()) {
    SETERROR(REG_EBRACK);
    తిరిగి (0);
  }
  len = p->తదుపరి - sp;
  కొరకు (cp = cnames; cp->name != ఖాళీ; cp++)
    ఒకవేళ (అక్షరశ్రేణి_పోలిక(cp->name, sp, len) == 0 && అక్షరశ్రేణి_పొడవు(cp->name) == len)
      తిరిగి (cp->సంకేతం_3); /* known name */
  ఒకవేళ (len == 1)
    తిరిగి (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  తిరిగి (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
స్థిర అక్షరం /* if no counterpart, return ch */
othercase(పూర్ణాంకం ch) {
  ch = (uch)ch;
  ప్రతిజ్ఞా(అక్షరమా(ch));
  ఒకవేళ (isupper(ch))
    తిరిగి ((uch)tolower(ch));
  లేకపోతే ఒకవేళ (islower(ch))
    తిరిగి ((uch)పెద్ద_అక్షరం(ch));
  లేకపోతే /* peculiar, but could happen */
    తిరిగి (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
స్థిర శూన్య bothcases(నిర్మాణం parse *p, పూర్ణాంకం ch) {
  స్థిరరాశి అక్షరం *oldnext = p->తదుపరి;
  స్థిరరాశి అక్షరం *oldend = p->చివర;
  అక్షరం bracket[3];

  ch = (uch)ch;
  ప్రతిజ్ఞా(othercase(ch) != ch); /* p_bracket() would recurse */
  p->తదుపరి = bracket;
  p->చివర = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  ప్రతిజ్ఞా(p->తదుపరి == bracket + 2);
  p->తదుపరి = oldnext;
  p->చివర = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
స్థిర శూన్య uncased(నిర్మాణం parse *p, పూర్ణాంకం ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  ఒకవేళ (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
స్థిర శూన్య ordinary(నిర్మాణం parse *p, పూర్ణాంకం ch) {
  ఒకవేళ ((p->g->cflags & REG_ICASE) && అక్షరమా((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  లేకపోతే
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
స్థిర శూన్య nonnewline(నిర్మాణం parse *p) {
  స్థిరరాశి అక్షరం *oldnext = p->తదుపరి;
  స్థిరరాశి అక్షరం *oldend = p->చివర;
  స్థిర స్థిరరాశి అక్షరం bracket[4] = {'^', '\n', ']', '\0'};

  p->తదుపరి = bracket;
  p->చివర = bracket + 3;
  p_bracket(p);
  ప్రతిజ్ఞా(p->తదుపరి == bracket + 3);
  p->తదుపరి = oldnext;
  p->చివర = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
స్థిర శూన్య repeat(నిర్మాణం parse *p,
                   sopno ప్రారంభం_2, /* operand from here to end of strip */
                   పూర్ణాంకం from,    /* repeated from this number */
                   పూర్ణాంకం గమ్యం_2)      /* to this number of times (maybe INFINITY) */
{
  sopno ముగింపు_2 = ఇక్కడ();
#నిర్వచించు N 2
#నిర్వచించు INF 3
#నిర్వచించు REP(f, t) ((f) * 8 + (t))
#నిర్వచించు MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  ఒకవేళ (p->error != 0) /* head off possible runaway recursion */
    తిరిగి;

  ప్రతిజ్ఞా(from <= గమ్యం_2);

  ఎంపిక (REP(MAP(from), MAP(గమ్యం_2))) {
  స్థితి REP(0, 0):         /* must be user doing this */
    DROP(ముగింపు_2 - ప్రారంభం_2); /* drop the operand */
    ఆపు;
  స్థితి REP(0, 1):   /* as x{1,1}? */
  స్థితి REP(0, N):   /* as x{1,n}? */
  స్థితి REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    చొప్పించు_3(OCH_, ప్రారంభం_2); /* offset is wrong... */
    repeat(p, ప్రారంభం_2 + 1, 1, గమ్యం_2);
    ASTERN(OOR1, ప్రారంభం_2);
    AHEAD(ప్రారంభం_2); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    ఆపు;
  స్థితి REP(1, 1): /* trivial case */
    /*పూర్తి*/
    ఆపు;
  స్థితి REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    చొప్పించు_3(OCH_, ప్రారంభం_2);
    ASTERN(OOR1, ప్రారంభం_2);
    AHEAD(ప్రారంభం_2);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, ప్రారంభం_2 + 1, ముగింపు_2 + 1);
    ప్రతిజ్ఞా(copy == ముగింపు_2 + 4);
    repeat(p, copy, 1, గమ్యం_2 - 1);
    ఆపు;
  స్థితి REP(1, INF): /* as x+ */
    చొప్పించు_3(OPLUS_, ప్రారంభం_2);
    ASTERN(O_PLUS, ప్రారంభం_2);
    ఆపు;
  స్థితి REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, ప్రారంభం_2, ముగింపు_2);
    repeat(p, copy, from - 1, గమ్యం_2 - 1);
    ఆపు;
  స్థితి REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, ప్రారంభం_2, ముగింపు_2);
    repeat(p, copy, from - 1, గమ్యం_2);
    ఆపు;
  అప్రమేయ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    ఆపు;
  }
}

/*
 - seterr - set an error condition
 */
స్థిర పూర్ణాంకం /* useless but makes type checking happy */
seterr(నిర్మాణం parse *p, పూర్ణాంకం e) {
  ఒకవేళ (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->తదుపరి = nuls; /* try to bring things to a halt */
  p->చివర = nuls;
  తిరిగి (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
స్థిర cset *allocset(నిర్మాణం parse *p) {
  పూర్ణాంకం no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  పూర్ణాంకం i;

  ఒకవేళ (no >= p->ncsalloc) { /* need another column of space */
    శూన్య *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    ఒకవేళ (nc > SIZE_MAX / పరిమాణం(cset))
      వెళ్ళు nomem;
    ప్రతిజ్ఞా(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((అక్షరం *)p->g->sets, nc * పరిమాణం(cset));
    ఒకవేళ (ptr == ఖాళీ)
      వెళ్ళు nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((అక్షరం *)p->g->setbits, nbytes);
    ఒకవేళ (ptr == ఖాళీ)
      వెళ్ళు nomem;
    p->g->setbits = ptr;

    కొరకు (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (శూన్య)memset((అక్షరం *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  ఒకవేళ (p->g->sets == ఖాళీ || p->g->setbits == ఖాళీ)
    వెళ్ళు nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->ముసుగు_2 = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ఖాళీ;

  తిరిగి (cs);
nomem:
  విడుదల(p->g->sets);
  p->g->sets = ఖాళీ;
  విడుదల(p->g->setbits);
  p->g->setbits = ఖాళీ;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  తిరిగి (ఖాళీ);
}

/*
 - freeset - free a now-unused set
 */
స్థిర శూన్య freeset(నిర్మాణం parse *p, cset *cs) {
  size_t i;
  cset *శిఖరం_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  కొరకు (i = 0; i < css; i++)
    CHsub(cs, i);
  ఒకవేళ (cs == శిఖరం_2 - 1) /* recover only the easy case */
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
స్థిర పూర్ణాంకం /* set number */
freezeset(నిర్మాణం parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *శిఖరం_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  కొరకు (cs2 = &p->g->sets[0]; cs2 < శిఖరం_2; cs2++)
    ఒకవేళ (cs2->hash == h && cs2 != cs) {
      /* maybe */
      కొరకు (i = 0; i < css; i++)
        ఒకవేళ (!!CHIN(cs2, i) != !!CHIN(cs, i))
          ఆపు; /* no */
      ఒకవేళ (i == css)
        ఆపు; /* yes */
    }

  ఒకవేళ (cs2 < శిఖరం_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  తిరిగి ((పూర్ణాంకం)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
స్థిర పూర్ణాంకం /* character; there is no "none" value */
firstch(నిర్మాణం parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  కొరకు (i = 0; i < css; i++)
    ఒకవేళ (CHIN(cs, i))
      తిరిగి ((అక్షరం)i);
  ప్రతిజ్ఞా(never);
  తిరిగి (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
స్థిర పూర్ణాంకం nch(నిర్మాణం parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  పూర్ణాంకం n = 0;

  కొరకు (i = 0; i < css; i++)
    ఒకవేళ (CHIN(cs, i))
      n++;
  తిరిగి (n);
}

/*
 - mcadd - add a collating element to a cset
 */
స్థిర శూన్య mcadd(నిర్మాణం parse *p, cset *cs, స్థిరరాశి అక్షరం *cp) {
  size_t oldend = cs->smultis;
  శూన్య *np;

  cs->smultis += అక్షరశ్రేణి_పొడవు(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  ఒకవేళ (np == ఖాళీ) {
    ఒకవేళ (cs->multis)
      విడుదల(cs->multis);
    cs->multis = ఖాళీ;
    SETERROR(REG_ESPACE);
    తిరిగి;
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
స్థిర శూన్య mcinvert(నిర్మాణం parse *p, cset *cs) {
  ప్రతిజ్ఞా(cs->multis == ఖాళీ); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
స్థిర శూన్య mccase(నిర్మాణం parse *p, cset *cs) {
  ప్రతిజ్ఞా(cs->multis == ఖాళీ); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
స్థిర పూర్ణాంకం /* predicate */
isinsets(నిర్మాణం re_guts *g, పూర్ణాంకం c) {
  uch *నిలువు_2;
  పూర్ణాంకం i;
  పూర్ణాంకం ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  గుర్తులేని uc = (uch)c;

  కొరకు (i = 0, నిలువు_2 = g->setbits; i < ncols; i++, నిలువు_2 += g->csetsize)
    ఒకవేళ (నిలువు_2[uc] != 0)
      తిరిగి (1);
  తిరిగి (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
స్థిర పూర్ణాంకం /* predicate */
samesets(నిర్మాణం re_guts *g, పూర్ణాంకం c1, పూర్ణాంకం c2) {
  uch *నిలువు_2;
  పూర్ణాంకం i;
  పూర్ణాంకం ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  గుర్తులేని uc1 = (uch)c1;
  గుర్తులేని uc2 = (uch)c2;

  కొరకు (i = 0, నిలువు_2 = g->setbits; i < ncols; i++, నిలువు_2 += g->csetsize)
    ఒకవేళ (నిలువు_2[uc1] != నిలువు_2[uc2])
      తిరిగి (0);
  తిరిగి (1);
}

/*
 - categorize - sort out character categories
 */
స్థిర శూన్య categorize(నిర్మాణం parse *p, నిర్మాణం re_guts *g) {
  cat_t *cats = g->categories;
  పూర్ణాంకం c;
  పూర్ణాంకం c2;
  cat_t cat;

  /* avoid making error situations worse */
  ఒకవేళ (p->error != 0)
    తిరిగి;

  కొరకు (c = CHAR_MIN; c <= CHAR_MAX; c++)
    ఒకవేళ (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      కొరకు (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        ఒకవేళ (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
స్థిర sopno                       /* start of duplicate */
dupl(నిర్మాణం parse *p, sopno ప్రారంభం_2, /* from here */
     sopno ముగింపు_2)                 /* to this less one */
{
  sopno ret = ఇక్కడ();
  sopno len = ముగింపు_2 - ప్రారంభం_2;

  ప్రతిజ్ఞా(ముగింపు_2 >= ప్రారంభం_2);
  ఒకవేళ (len == 0)
    తిరిగి (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  ప్రతిజ్ఞా(p->ssize >= p->slen + len);
  (శూన్య)memmove((అక్షరం *)(p->strip + p->slen), (అక్షరం *)(p->strip + ప్రారంభం_2),
                (size_t)len * పరిమాణం(sop));
  p->slen += len;
  తిరిగి (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
స్థిర శూన్య doemit(నిర్మాణం parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  ఒకవేళ (p->error != 0)
    తిరిగి;

  /* deal with oversize operands ("can't happen", more or less) */
  ప్రతిజ్ఞా(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  ఒకవేళ (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  ప్రతిజ్ఞా(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
స్థిర శూన్య doinsert(నిర్మాణం parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  పూర్ణాంకం i;

  /* avoid making error situations worse */
  ఒకవేళ (p->error != 0)
    తిరిగి;

  sn = ఇక్కడ();
  EMIT(op, opnd); /* do checks, ensure space */
  ప్రతిజ్ఞా(ఇక్కడ() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  ప్రతిజ్ఞా(pos > 0);
  కొరకు (i = 1; i < NPAREN; i++) {
    ఒకవేళ (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    ఒకవేళ (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((అక్షరం *)&p->strip[pos + 1], (అక్షరం *)&p->strip[pos],
          (ఇక్కడ() - pos - 1) * పరిమాణం(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
స్థిర శూన్య dofwd(నిర్మాణం parse *p, sopno pos, sop విలువ_2) {
  /* avoid making error situations worse */
  ఒకవేళ (p->error != 0)
    తిరిగి;

  ప్రతిజ్ఞా(విలువ_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | విలువ_2;
}

/*
 - enlarge - enlarge the strip
 */
స్థిర శూన్య enlarge(నిర్మాణం parse *p, sopno కొలత_2) {
  sop *sp;

  ఒకవేళ (p->ssize >= కొలత_2)
    తిరిగి;

  ఒకవేళ ((uintptr_t)కొలత_2 > SIZE_MAX / పరిమాణం(sop)) {
    SETERROR(REG_ESPACE);
    తిరిగి;
  }

  sp = (sop *)realloc(p->strip, కొలత_2 * పరిమాణం(sop));
  ఒకవేళ (sp == ఖాళీ) {
    SETERROR(REG_ESPACE);
    తిరిగి;
  }
  p->strip = sp;
  p->ssize = కొలత_2;
}

/*
 - stripsnug - compact the strip
 */
స్థిర శూన్య stripsnug(నిర్మాణం parse *p, నిర్మాణం re_guts *g) {
  g->nstates = p->slen;
  ఒకవేళ ((uintptr_t)p->slen > SIZE_MAX / పరిమాణం(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    తిరిగి;
  }

  g->strip = (sop *)realloc((అక్షరం *)p->strip, p->slen * పరిమాణం(sop));
  ఒకవేళ (g->strip == ఖాళీ) {
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
స్థిర శూన్య findmust(నిర్మాణం parse *p, నిర్మాణం re_guts *g) {
  sop *పరిశీలన;
  sop *ప్రారంభం_2 = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  అక్షరం *cp;
  sopno i;

  /* avoid making error situations worse */
  ఒకవేళ (p->error != 0)
    తిరిగి;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  పరిశీలన = g->strip + 1;
  చేయి {
    s = *పరిశీలన++;
    ఎంపిక (OP(s)) {
    స్థితి OCHAR:        /* sequence member */
      ఒకవేళ (newlen == 0) /* new sequence */
        newstart = పరిశీలన - 1;
      newlen++;
      ఆపు;
    స్థితి OPLUS_: /* things that don't break one */
    స్థితి OLPAREN:
    స్థితి ORPAREN:
      ఆపు;
    స్థితి OQUEST_: /* things that must be skipped */
    స్థితి OCH_:
      పరిశీలన--;
      చేయి {
        పరిశీలన += OPND(s);
        s = *పరిశీలన;
        /* assert() interferes w debug printouts */
        ఒకవేళ (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          తిరిగి;
        }
      } అయితే (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    అప్రమేయ:                  /* things that break a sequence */
      ఒకవేళ (newlen > g->mlen) { /* ends one */
        ప్రారంభం_2 = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      ఆపు;
    }
  } అయితే (OP(s) != OEND);

  ఒకవేళ (g->mlen == 0) /* there isn't one */
    తిరిగి;

  /* turn it into a character string */
  g->must = జ్ఞాపకం_ఇవ్వు((size_t)g->mlen + 1);
  ఒకవేళ (g->must == ఖాళీ) { /* argh; just forget it */
    g->mlen = 0;
    తిరిగి;
  }
  cp = g->must;
  పరిశీలన = ప్రారంభం_2;
  కొరకు (i = g->mlen; i > 0; i--) {
    అయితే (OP(s = *పరిశీలన++) != OCHAR)
      కొనసాగు;
    ప్రతిజ్ఞా(cp < g->must + g->mlen);
    *cp++ = (అక్షరం)OPND(s);
  }
  ప్రతిజ్ఞా(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
స్థిర sopno /* nesting depth */
pluscount(నిర్మాణం parse *p, నిర్మాణం re_guts *g) {
  sop *పరిశీలన;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  ఒకవేళ (p->error != 0)
    తిరిగి (0); /* there may not be an OEND */

  పరిశీలన = g->strip + 1;
  చేయి {
    s = *పరిశీలన++;
    ఎంపిక (OP(s)) {
    స్థితి OPLUS_:
      plusnest++;
      ఆపు;
    స్థితి O_PLUS:
      ఒకవేళ (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      ఆపు;
    }
  } అయితే (OP(s) != OEND);
  ఒకవేళ (plusnest != 0)
    g->iflags |= REGEX_BAD;
  తిరిగి (maxnest);
}
