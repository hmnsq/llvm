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

#समावेश "regex_impl.h"
#समावेश <ctype.h>
#समावेश <limits.h>
#समावेश <stdint.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/types.h>

#समावेश "regex2.h"
#समावेश "regutils.h"

#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Compiler.h"

/* character-class table */
अटल काष्ठा cclass {
  स्थिर अक्षर *name;
  स्थिर अक्षर *chars;
  स्थिर अक्षर *multis;
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
    {शून्य, 0, ""}};

/* character-name table */
अटल काष्ठा cname {
  स्थिर अक्षर *name;
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
              {शून्य, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
काष्ठा parse {
  स्थिर अक्षर *अग्र; /* next character in RE */
  स्थिर अक्षर *end;  /* end of string (-> NUL normally) */
  पूर्णांक error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  पूर्णांक ncsalloc;     /* number of csets allocated */
  काष्ठा re_guts *g;
#घोषणा NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

अटल व्योम p_ere(काष्ठा parse *, पूर्णांक);
अटल व्योम p_ere_exp(काष्ठा parse *);
अटल व्योम p_str(काष्ठा parse *);
अटल व्योम p_bre(काष्ठा parse *, पूर्णांक, पूर्णांक);
अटल पूर्णांक p_simp_re(काष्ठा parse *, पूर्णांक);
अटल पूर्णांक p_गिनती(काष्ठा parse *);
अटल व्योम p_bracket(काष्ठा parse *);
अटल व्योम p_b_term(काष्ठा parse *, cset *);
अटल व्योम p_b_cclass(काष्ठा parse *, cset *);
अटल व्योम p_b_eclass(काष्ठा parse *, cset *);
अटल अक्षर p_b_प्रतीक(काष्ठा parse *);
अटल अक्षर p_b_coll_elem(काष्ठा parse *, पूर्णांक);
अटल अक्षर othercase(पूर्णांक);
अटल व्योम bothcases(काष्ठा parse *, पूर्णांक);
अटल व्योम uncased(काष्ठा parse *, पूर्णांक);
अटल व्योम ordinary(काष्ठा parse *, पूर्णांक);
अटल व्योम nonnewline(काष्ठा parse *);
अटल व्योम repeat(काष्ठा parse *, sopno, पूर्णांक, पूर्णांक);
अटल पूर्णांक seterr(काष्ठा parse *, पूर्णांक);
अटल cset *allocset(काष्ठा parse *);
अटल व्योम freeset(काष्ठा parse *, cset *);
अटल पूर्णांक freezeset(काष्ठा parse *, cset *);
अटल पूर्णांक firstch(काष्ठा parse *, cset *);
अटल पूर्णांक nch(काष्ठा parse *, cset *);
अटल व्योम mcadd(काष्ठा parse *, cset *, स्थिर अक्षर *);
अटल व्योम mcinvert(काष्ठा parse *, cset *);
अटल व्योम mccase(काष्ठा parse *, cset *);
अटल पूर्णांक isinsets(काष्ठा re_guts *, पूर्णांक);
अटल पूर्णांक samesets(काष्ठा re_guts *, पूर्णांक, पूर्णांक);
अटल व्योम categorize(काष्ठा parse *, काष्ठा re_guts *);
अटल sopno dupl(काष्ठा parse *, sopno, sopno);
अटल व्योम doemit(काष्ठा parse *, sop, माप_प्रकार);
अटल व्योम doinsert(काष्ठा parse *, sop, माप_प्रकार, sopno);
अटल व्योम dofwd(काष्ठा parse *, sopno, sop);
अटल व्योम enlarge(काष्ठा parse *, sopno);
अटल व्योम stripsnug(काष्ठा parse *, काष्ठा re_guts *);
अटल व्योम findmust(काष्ठा parse *, काष्ठा re_guts *);
अटल sopno pluscount(काष्ठा parse *, काष्ठा re_guts *);

अटल अक्षर nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#घोषणा PEEK() (*p->अग्र)
#घोषणा PEEK2() (*(p->अग्र + 1))
#घोषणा MORE() (p->end - p->अग्र > 0)
#घोषणा MORE2() (p->end - p->अग्र > 1)
#घोषणा SEE(c) (MORE() && PEEK() == (c))
#घोषणा SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#घोषणा EAT(c) ((SEE(c)) ? (अग्र_3(), 1) : 0)
#घोषणा EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#घोषणा अग्र_3() (p->अग्र++)
#घोषणा NEXT2() (p->अग्र += 2)
#घोषणा NEXTn(n) (p->अग्र += (n))
#घोषणा GETNEXT() (*p->अग्र++)
#घोषणा SETERROR(e) seterr(p, (e))
#घोषणा REQUIRE(co, e) (व्योम)((co) || SETERROR(e))
#घोषणा MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#घोषणा MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#घोषणा MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#घोषणा EMIT(op, sopnd) doemit(p, (sop)(op), (माप_प्रकार)(sopnd))
#घोषणा जोड़ो_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#घोषणा AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#घोषणा ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#घोषणा HERE() (p->slen)
#घोषणा THERE() (p->slen - 1)
#घोषणा THERETHERE() (p->slen - 2)
#घोषणा DROP(n) (p->slen -= (n))

#अगर_घोषित _POSIX2_RE_DUP_MAX
#घोषणा DUPMAX _POSIX2_RE_DUP_MAX
#अन्यथा
#घोषणा DUPMAX 255
#पूर्ण_अगर
#घोषणा REGINFINITY (DUPMAX + 1)

#अगर_अघोषित न_संशोधन
अटल पूर्णांक never = 0; /* for use in asserts; shuts lint up */
#अन्यथा
#घोषणा never 0 /* some <assert.h>s have bugs too */
#पूर्ण_अगर

/*
 - llvm_regcomp - interface for parser and compilation
 */
पूर्णांक /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, स्थिर अक्षर *pattern, पूर्णांक cflags) {
  काष्ठा parse pa;
  काष्ठा re_guts *g;
  काष्ठा parse *p = &pa;
  पूर्णांक i;
  माप_प्रकार len;
#अगर_घोषित REDEBUG
#घोषणा GOODFLAGS(f) (f)
#अन्यथा
#घोषणा GOODFLAGS(f) ((f) & ~REG_DUMP)
#पूर्ण_अगर

  cflags = GOODFLAGS(cflags);
  अगर ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    वापस (REG_INVARG);

  अगर (cflags & REG_PEND) {
    अगर (preg->re_endp < pattern)
      वापस (REG_INVARG);
    len = preg->re_endp - pattern;
  } अन्यथा {
    len = म_माप(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (काष्ठा re_guts *)दो_स्मृति(माप(काष्ठा re_guts) +
                               (NC - 1) * माप(cat_t));
  अगर (g == शून्य)
    वापस (REG_ESPACE);
  p->ssize = len / (माप_प्रकार)2 * (माप_प्रकार)3 + (माप_प्रकार)1; /* ugh */
  p->strip = (sop *)सुस्मृति(p->ssize, माप(sop));
  p->slen = 0;
  अगर (p->strip == शून्य) {
    मुक्त((अक्षर *)g);
    वापस (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->अग्र = pattern;
  p->end = p->अग्र + len;
  p->error = 0;
  p->ncsalloc = 0;
  क्रम (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = शून्य;
  g->setbits = शून्य;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = शून्य;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(अक्षर_न्यून)];
  (व्योम)स_रखो((अक्षर *)g->catspace, 0, NC * माप(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  अगर (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  अन्यथा अगर (cflags & REG_NOSPEC)
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
#अगर_अघोषित REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  अगर (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#पूर्ण_अगर

  /* win or lose, we're done */
  अगर (p->error != 0) /* lose */
    llvm_regfree(preg);
  वापस (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
अटल व्योम p_ere(काष्ठा parse *p,
                  पूर्णांक stop) /* character this ERE should end at */
{
  अक्षर c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  पूर्णांक first = 1; /* is this the first alternative? */

  क्रम (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    जबतक (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    अगर (!EAT('|'))
      अवरोध; /* NOTE BREAK OUT */

    अगर (first) {
      जोड़ो_3(OCH_, conc); /* offset is wrong */
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

  अगर (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  निश्चित(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
अटल व्योम p_ere_exp(काष्ठा parse *p) {
  अक्षर c;
  sopno pos;
  पूर्णांक गिनती;
  पूर्णांक count2;
  पूर्णांक backrefnum;
  sopno subno;
  पूर्णांक wascaret = 0;

  निश्चित(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  चयन (c) {
  हाल '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    अगर (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    अगर (!SEE(')'))
      p_ere(p, ')');
    अगर (subno < NPAREN) {
      p->pend[subno] = HERE();
      निश्चित(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    अवरोध;
#अगर_अघोषित POSIX_MISTAKE
  हाल ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    अवरोध;
#पूर्ण_अगर
  हाल '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    अवरोध;
  हाल '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    अवरोध;
  हाल '|':
    SETERROR(REG_EMPTY);
    अवरोध;
  हाल '*':
  हाल '+':
  हाल '?':
    SETERROR(REG_BADRPT);
    अवरोध;
  हाल '.':
    अगर (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    अवरोध;
  हाल '[':
    p_bracket(p);
    अवरोध;
  हाल '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    अगर (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } अन्यथा अगर (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      जबतक (MORE() && है_अंक(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } अन्यथा अगर (c == 'n') {
      ordinary(p, '\n');
      अवरोध;
    } अन्यथा अगर (c == 't') {
      ordinary(p, '\t');
      अवरोध;
    } अन्यथा अगर (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      अक्षर hexstr[3] = {0};
      अक्षर *hexp;
      पूर्णांक val;
      अगर (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = म_से_दीर्घ(hexstr, &hexp, 16);
        अगर (*hexp == '\0') {
          NEXT2();
          uncased(p, (अक्षर)val);
          अवरोध;
        }
      }
      ordinary(p, 'x');
      अवरोध;
    } अन्यथा {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      अवरोध;
    }

    अगर (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      अवरोध;
    }

    अगर (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      अवरोध;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    निश्चित(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    निश्चित(p->pbegin[backrefnum] != 0);
    निश्चित(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    निश्चित(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (व्योम)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    अवरोध;
  हाल '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !है_अंक((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  शेष:
    ordinary(p, c);
    अवरोध;
  }

  अगर (!MORE())
    वापस;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  अगर (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    वापस; /* no repetition, we're done */
  अग्र_3();

  REQUIRE(!wascaret, REG_BADRPT);
  चयन (c) {
  हाल '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    जोड़ो_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    जोड़ो_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    अवरोध;
  हाल '+':
    जोड़ो_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    अवरोध;
  हाल '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    जोड़ो_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    अवरोध;
  हाल '{':
    गिनती = p_गिनती(p);
    अगर (EAT(',')) {
      अगर (है_अंक((uch)PEEK())) {
        count2 = p_गिनती(p);
        REQUIRE(गिनती <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = गिनती;
    repeat(p, pos, गिनती, count2);
    अगर (!EAT('}')) { /* error heuristics */
      जबतक (MORE() && PEEK() != '}')
        अग्र_3();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    अवरोध;
  }

  अगर (!MORE())
    वापस;
  c = PEEK();
  अगर (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && है_अंक((uch)PEEK2()))))
    वापस;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
अटल व्योम p_str(काष्ठा parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  जबतक (MORE())
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
अटल व्योम p_bre(काष्ठा parse *p, पूर्णांक end1, /* first terminating character */
                  पूर्णांक end2)                  /* second terminating character */
{
  sopno start = HERE();
  पूर्णांक first = 1; /* first subexpression? */
  पूर्णांक wasdollar = 0;

  अगर (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  जबतक (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  अगर (wasdollar) { /* oops, that was a trailing anchor */
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
अटल पूर्णांक /* was the simple RE an unbackslashed $? */
p_simp_re(काष्ठा parse *p,
          पूर्णांक starordinary) /* is a leading * an ordinary character? */
{
  पूर्णांक c;
  पूर्णांक गिनती;
  पूर्णांक count2;
  sopno pos;
  पूर्णांक i;
  sopno subno;
#घोषणा BACKSL (1 << अक्षर_बिट)

  pos = HERE(); /* repetition op, if any, covers from here */

  निश्चित(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  अगर (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  चयन (c) {
  हाल '.':
    अगर (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    अन्यथा
      EMIT(OANY, 0);
    अवरोध;
  हाल '[':
    p_bracket(p);
    अवरोध;
  हाल BACKSL | '{':
    SETERROR(REG_BADRPT);
    अवरोध;
  हाल BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    अगर (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    अगर (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    अगर (subno < NPAREN) {
      p->pend[subno] = HERE();
      निश्चित(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    अवरोध;
  हाल BACKSL | ')': /* should not get here -- must be user */
  हाल BACKSL | '}':
    SETERROR(REG_EPAREN);
    अवरोध;
  हाल BACKSL | '1':
  हाल BACKSL | '2':
  हाल BACKSL | '3':
  हाल BACKSL | '4':
  हाल BACKSL | '5':
  हाल BACKSL | '6':
  हाल BACKSL | '7':
  हाल BACKSL | '8':
  हाल BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    निश्चित(i < NPAREN);
    अगर (p->pend[i] != 0) {
      निश्चित(i <= p->g->nsub);
      EMIT(OBACK_, i);
      निश्चित(p->pbegin[i] != 0);
      निश्चित(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      निश्चित(OP(p->strip[p->pend[i]]) == ORPAREN);
      (व्योम)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } अन्यथा {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    अवरोध;
  हाल '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  शेष:
    ordinary(p, (अक्षर)c);
    अवरोध;
  }

  अगर (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    जोड़ो_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    जोड़ो_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } अन्यथा अगर (EATTWO('\\', '{')) {
    गिनती = p_गिनती(p);
    अगर (EAT(',')) {
      अगर (MORE() && है_अंक((uch)PEEK())) {
        count2 = p_गिनती(p);
        REQUIRE(गिनती <= count2, REG_BADBR);
      } अन्यथा /* single number with comma */
        count2 = REGINFINITY;
    } अन्यथा /* just a single number */
      count2 = गिनती;
    repeat(p, pos, गिनती, count2);
    अगर (!EATTWO('\\', '}')) { /* error heuristics */
      जबतक (MORE() && !SEETWO('\\', '}'))
        अग्र_3();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } अन्यथा अगर (c == '$') /* $ (but not \$) ends it */
    वापस (1);

  वापस (0);
}

/*
 - p_count - parse a repetition count
 */
अटल पूर्णांक /* the value */
p_गिनती(काष्ठा parse *p) {
  पूर्णांक गिनती = 0;
  पूर्णांक ndigits = 0;

  जबतक (MORE() && है_अंक((uch)PEEK()) && गिनती <= DUPMAX) {
    गिनती = गिनती * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && गिनती <= DUPMAX, REG_BADBR);
  वापस (गिनती);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
अटल व्योम p_bracket(काष्ठा parse *p) {
  cset *cs;
  पूर्णांक invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  अगर (p->end - p->अग्र > 5) {
    अगर (म_भेदन(p->अग्र, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      वापस;
    }
    अगर (म_भेदन(p->अग्र, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      वापस;
    }
  }

  अगर ((cs = allocset(p)) == शून्य) {
    /* allocset did set error status in p */
    वापस;
  }

  अगर (EAT('^'))
    invert++; /* make note to invert set at end */
  अगर (EAT(']'))
    CHadd(cs, ']');
  अन्यथा अगर (EAT('-'))
    CHadd(cs, '-');
  जबतक (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  अगर (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  अगर (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    वापस;
  }

  अगर (p->g->cflags & REG_ICASE) {
    पूर्णांक i;
    पूर्णांक ci;

    क्रम (i = p->g->csetsize - 1; i >= 0; i--)
      अगर (CHIN(cs, i) && है_अक्षर(i)) {
        ci = othercase(i);
        अगर (ci != i)
          CHadd(cs, ci);
      }
    अगर (cs->multis != शून्य)
      mccase(p, cs);
  }
  अगर (invert) {
    पूर्णांक i;

    क्रम (i = p->g->csetsize - 1; i >= 0; i--)
      अगर (CHIN(cs, i))
        CHsub(cs, i);
      अन्यथा
        CHadd(cs, i);
    अगर (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    अगर (cs->multis != शून्य)
      mcinvert(p, cs);
  }

  निश्चित(cs->multis == शून्य); /* xxx */

  अगर (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } अन्यथा {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
अटल व्योम p_b_term(काष्ठा parse *p, cset *cs) {
  अक्षर c;
  अक्षर start, finish;
  पूर्णांक i;

  /* classify what we've got */
  चयन ((MORE()) ? PEEK() : '\0') {
  हाल '[':
    c = (MORE2()) ? PEEK2() : '\0';
    अवरोध;
  हाल '-':
    SETERROR(REG_ERANGE);
    वापस; /* NOTE RETURN */
    अवरोध;
  शेष:
    c = '\0';
    अवरोध;
  }

  चयन (c) {
  हाल ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    अवरोध;
  हाल '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    अवरोध;
  शेष: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_प्रतीक(p);
    अगर (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      अग्र_3();
      अगर (EAT('-'))
        finish = '-';
      अन्यथा
        finish = p_b_प्रतीक(p);
    } अन्यथा {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    क्रम (i = start; i <= finish; i++)
      CHadd(cs, i);
    अवरोध;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
अटल व्योम p_b_cclass(काष्ठा parse *p, cset *cs) {
  स्थिर अक्षर *sp = p->अग्र;
  काष्ठा cclass *cp;
  माप_प्रकार len;
  स्थिर अक्षर *u;
  अक्षर c;

  जबतक (MORE() && है_अक्षर((uch)PEEK()))
    अग्र_3();
  len = p->अग्र - sp;
  क्रम (cp = cclasses; cp->name != शून्य; cp++)
    अगर (म_भेदन(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      अवरोध;
  अगर (cp->name == शून्य) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    वापस;
  }

  u = cp->chars;
  जबतक ((c = *u++) != '\0')
    CHadd(cs, c);
  क्रम (u = cp->multis; *u != '\0'; u += म_माप(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
अटल व्योम p_b_eclass(काष्ठा parse *p, cset *cs) {
  अक्षर c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
अटल अक्षर /* value of symbol */
p_b_प्रतीक(काष्ठा parse *p) {
  अक्षर मूल्य_2;

  REQUIRE(MORE(), REG_EBRACK);
  अगर (!EATTWO('[', '.'))
    वापस (GETNEXT());

  /* collating symbol */
  मूल्य_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  वापस (मूल्य_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
अटल अक्षर                              /* value of collating element */
p_b_coll_elem(काष्ठा parse *p, पूर्णांक endc) /* name ended by endc,']' */
{
  स्थिर अक्षर *sp = p->अग्र;
  काष्ठा cname *cp;
  माप_प्रकार len;

  जबतक (MORE() && !SEETWO(endc, ']'))
    अग्र_3();
  अगर (!MORE()) {
    SETERROR(REG_EBRACK);
    वापस (0);
  }
  len = p->अग्र - sp;
  क्रम (cp = cnames; cp->name != शून्य; cp++)
    अगर (म_भेदन(cp->name, sp, len) == 0 && म_माप(cp->name) == len)
      वापस (cp->code); /* known name */
  अगर (len == 1)
    वापस (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  वापस (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
अटल अक्षर /* if no counterpart, return ch */
othercase(पूर्णांक ch) {
  ch = (uch)ch;
  निश्चित(है_अक्षर(ch));
  अगर (है_बड़ा(ch))
    वापस ((uch)छोटे(ch));
  अन्यथा अगर (है_छोटा(ch))
    वापस ((uch)बड़े(ch));
  अन्यथा /* peculiar, but could happen */
    वापस (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
अटल व्योम bothcases(काष्ठा parse *p, पूर्णांक ch) {
  स्थिर अक्षर *oldnext = p->अग्र;
  स्थिर अक्षर *oldend = p->end;
  अक्षर bracket[3];

  ch = (uch)ch;
  निश्चित(othercase(ch) != ch); /* p_bracket() would recurse */
  p->अग्र = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  निश्चित(p->अग्र == bracket + 2);
  p->अग्र = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
अटल व्योम uncased(काष्ठा parse *p, पूर्णांक ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  अगर (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
अटल व्योम ordinary(काष्ठा parse *p, पूर्णांक ch) {
  अगर ((p->g->cflags & REG_ICASE) && है_अक्षर((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  अन्यथा
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
अटल व्योम nonnewline(काष्ठा parse *p) {
  स्थिर अक्षर *oldnext = p->अग्र;
  स्थिर अक्षर *oldend = p->end;
  अटल स्थिर अक्षर bracket[4] = {'^', '\n', ']', '\0'};

  p->अग्र = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  निश्चित(p->अग्र == bracket + 3);
  p->अग्र = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
अटल व्योम repeat(काष्ठा parse *p,
                   sopno start, /* operand from here to end of strip */
                   पूर्णांक from,    /* repeated from this number */
                   पूर्णांक to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#घोषणा N 2
#घोषणा INF 3
#घोषणा REP(f, t) ((f) * 8 + (t))
#घोषणा MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  अगर (p->error != 0) /* head off possible runaway recursion */
    वापस;

  निश्चित(from <= to);

  चयन (REP(MAP(from), MAP(to))) {
  हाल REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    अवरोध;
  हाल REP(0, 1):   /* as x{1,1}? */
  हाल REP(0, N):   /* as x{1,n}? */
  हाल REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    जोड़ो_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    अवरोध;
  हाल REP(1, 1): /* trivial case */
    /* done */
    अवरोध;
  हाल REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    जोड़ो_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    निश्चित(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    अवरोध;
  हाल REP(1, INF): /* as x+ */
    जोड़ो_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    अवरोध;
  हाल REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    अवरोध;
  हाल REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    अवरोध;
  शेष:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    अवरोध;
  }
}

/*
 - seterr - set an error condition
 */
अटल पूर्णांक /* useless but makes type checking happy */
seterr(काष्ठा parse *p, पूर्णांक e) {
  अगर (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->अग्र = nuls; /* try to bring things to a halt */
  p->end = nuls;
  वापस (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
अटल cset *allocset(काष्ठा parse *p) {
  पूर्णांक no = p->g->ncsets++;
  माप_प्रकार nc;
  माप_प्रकार nbytes;
  cset *cs;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णांक i;

  अगर (no >= p->ncsalloc) { /* need another column of space */
    व्योम *ptr;

    p->ncsalloc += अक्षर_बिट;
    nc = p->ncsalloc;
    अगर (nc > SIZE_MAX / माप(cset))
      जाओ nomem;
    निश्चित(nc % अक्षर_बिट == 0);
    nbytes = nc / अक्षर_बिट * css;

    ptr = (cset *)पुनः_स्मृति((अक्षर *)p->g->sets, nc * माप(cset));
    अगर (ptr == शून्य)
      जाओ nomem;
    p->g->sets = ptr;

    ptr = (uch *)पुनः_स्मृति((अक्षर *)p->g->setbits, nbytes);
    अगर (ptr == शून्य)
      जाओ nomem;
    p->g->setbits = ptr;

    क्रम (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / अक्षर_बिट);

    (व्योम)स_रखो((अक्षर *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  अगर (p->g->sets == शून्य || p->g->setbits == शून्य)
    जाओ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / अक्षर_बिट);
  cs->mask = 1 << ((no) % अक्षर_बिट);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = शून्य;

  वापस (cs);
nomem:
  मुक्त(p->g->sets);
  p->g->sets = शून्य;
  मुक्त(p->g->setbits);
  p->g->setbits = शून्य;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  वापस (शून्य);
}

/*
 - freeset - free a now-unused set
 */
अटल व्योम freeset(काष्ठा parse *p, cset *cs) {
  माप_प्रकार i;
  cset *शीर्ष_4 = &p->g->sets[p->g->ncsets];
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  क्रम (i = 0; i < css; i++)
    CHsub(cs, i);
  अगर (cs == शीर्ष_4 - 1) /* recover only the easy case */
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
अटल पूर्णांक /* set number */
freezeset(काष्ठा parse *p, cset *cs) {
  uch h = cs->hash;
  माप_प्रकार i;
  cset *शीर्ष_4 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  /* look for an earlier one which is the same */
  क्रम (cs2 = &p->g->sets[0]; cs2 < शीर्ष_4; cs2++)
    अगर (cs2->hash == h && cs2 != cs) {
      /* maybe */
      क्रम (i = 0; i < css; i++)
        अगर (!!CHIN(cs2, i) != !!CHIN(cs, i))
          अवरोध; /* no */
      अगर (i == css)
        अवरोध; /* yes */
    }

  अगर (cs2 < शीर्ष_4) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  वापस ((पूर्णांक)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
अटल पूर्णांक /* character; there is no "none" value */
firstch(काष्ठा parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;

  क्रम (i = 0; i < css; i++)
    अगर (CHIN(cs, i))
      वापस ((अक्षर)i);
  निश्चित(never);
  वापस (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
अटल पूर्णांक nch(काष्ठा parse *p, cset *cs) {
  माप_प्रकार i;
  माप_प्रकार css = (माप_प्रकार)p->g->csetsize;
  पूर्णांक n = 0;

  क्रम (i = 0; i < css; i++)
    अगर (CHIN(cs, i))
      n++;
  वापस (n);
}

/*
 - mcadd - add a collating element to a cset
 */
अटल व्योम mcadd(काष्ठा parse *p, cset *cs, स्थिर अक्षर *cp) {
  माप_प्रकार oldend = cs->smultis;
  व्योम *np;

  cs->smultis += म_माप(cp) + 1;
  np = पुनः_स्मृति(cs->multis, cs->smultis);
  अगर (np == शून्य) {
    अगर (cs->multis)
      मुक्त(cs->multis);
    cs->multis = शून्य;
    SETERROR(REG_ESPACE);
    वापस;
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
अटल व्योम mcinvert(काष्ठा parse *p, cset *cs) {
  निश्चित(cs->multis == शून्य); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
अटल व्योम mccase(काष्ठा parse *p, cset *cs) {
  निश्चित(cs->multis == शून्य); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
अटल पूर्णांक /* predicate */
isinsets(काष्ठा re_guts *g, पूर्णांक c) {
  uch *col;
  पूर्णांक i;
  पूर्णांक ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc = (uch)c;

  क्रम (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    अगर (col[uc] != 0)
      वापस (1);
  वापस (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
अटल पूर्णांक /* predicate */
samesets(काष्ठा re_guts *g, पूर्णांक c1, पूर्णांक c2) {
  uch *col;
  पूर्णांक i;
  पूर्णांक ncols = (g->ncsets + (अक्षर_बिट - 1)) / अक्षर_बिट;
  अचिन्हित uc1 = (uch)c1;
  अचिन्हित uc2 = (uch)c2;

  क्रम (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    अगर (col[uc1] != col[uc2])
      वापस (0);
  वापस (1);
}

/*
 - categorize - sort out character categories
 */
अटल व्योम categorize(काष्ठा parse *p, काष्ठा re_guts *g) {
  cat_t *cats = g->categories;
  पूर्णांक c;
  पूर्णांक c2;
  cat_t cat;

  /* avoid making error situations worse */
  अगर (p->error != 0)
    वापस;

  क्रम (c = अक्षर_न्यून; c <= अक्षर_उच्च; c++)
    अगर (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      क्रम (c2 = c + 1; c2 <= अक्षर_उच्च; c2++)
        अगर (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
अटल sopno                       /* start of duplicate */
dupl(काष्ठा parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  निश्चित(finish >= start);
  अगर (len == 0)
    वापस (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  निश्चित(p->ssize >= p->slen + len);
  (व्योम)स_हटाओ((अक्षर *)(p->strip + p->slen), (अक्षर *)(p->strip + start),
                (माप_प्रकार)len * माप(sop));
  p->slen += len;
  वापस (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
अटल व्योम doemit(काष्ठा parse *p, sop op, माप_प्रकार opnd) {
  /* avoid making error situations worse */
  अगर (p->error != 0)
    वापस;

  /* deal with oversize operands ("can't happen", more or less) */
  निश्चित(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  अगर (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  निश्चित(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
अटल व्योम doinsert(काष्ठा parse *p, sop op, माप_प्रकार opnd, sopno pos) {
  sopno sn;
  sop s;
  पूर्णांक i;

  /* avoid making error situations worse */
  अगर (p->error != 0)
    वापस;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  निश्चित(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  निश्चित(pos > 0);
  क्रम (i = 1; i < NPAREN; i++) {
    अगर (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    अगर (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  स_हटाओ((अक्षर *)&p->strip[pos + 1], (अक्षर *)&p->strip[pos],
          (HERE() - pos - 1) * माप(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
अटल व्योम dofwd(काष्ठा parse *p, sopno pos, sop मूल्य_2) {
  /* avoid making error situations worse */
  अगर (p->error != 0)
    वापस;

  निश्चित(मूल्य_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | मूल्य_2;
}

/*
 - enlarge - enlarge the strip
 */
अटल व्योम enlarge(काष्ठा parse *p, sopno आकार_2) {
  sop *sp;

  अगर (p->ssize >= आकार_2)
    वापस;

  अगर ((uintptr_t)आकार_2 > SIZE_MAX / माप(sop)) {
    SETERROR(REG_ESPACE);
    वापस;
  }

  sp = (sop *)पुनः_स्मृति(p->strip, आकार_2 * माप(sop));
  अगर (sp == शून्य) {
    SETERROR(REG_ESPACE);
    वापस;
  }
  p->strip = sp;
  p->ssize = आकार_2;
}

/*
 - stripsnug - compact the strip
 */
अटल व्योम stripsnug(काष्ठा parse *p, काष्ठा re_guts *g) {
  g->nstates = p->slen;
  अगर ((uintptr_t)p->slen > SIZE_MAX / माप(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    वापस;
  }

  g->strip = (sop *)पुनः_स्मृति((अक्षर *)p->strip, p->slen * माप(sop));
  अगर (g->strip == शून्य) {
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
अटल व्योम findmust(काष्ठा parse *p, काष्ठा re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  अक्षर *cp;
  sopno i;

  /* avoid making error situations worse */
  अगर (p->error != 0)
    वापस;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  करो {
    s = *scan++;
    चयन (OP(s)) {
    हाल OCHAR:        /* sequence member */
      अगर (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      अवरोध;
    हाल OPLUS_: /* things that don't break one */
    हाल OLPAREN:
    हाल ORPAREN:
      अवरोध;
    हाल OQUEST_: /* things that must be skipped */
    हाल OCH_:
      scan--;
      करो {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        अगर (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          वापस;
        }
      } जबतक (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    शेष:                  /* things that break a sequence */
      अगर (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      अवरोध;
    }
  } जबतक (OP(s) != OEND);

  अगर (g->mlen == 0) /* there isn't one */
    वापस;

  /* turn it into a character string */
  g->must = दो_स्मृति((माप_प्रकार)g->mlen + 1);
  अगर (g->must == शून्य) { /* argh; just forget it */
    g->mlen = 0;
    वापस;
  }
  cp = g->must;
  scan = start;
  क्रम (i = g->mlen; i > 0; i--) {
    जबतक (OP(s = *scan++) != OCHAR)
      जारी;
    निश्चित(cp < g->must + g->mlen);
    *cp++ = (अक्षर)OPND(s);
  }
  निश्चित(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
अटल sopno /* nesting depth */
pluscount(काष्ठा parse *p, काष्ठा re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  अगर (p->error != 0)
    वापस (0); /* there may not be an OEND */

  scan = g->strip + 1;
  करो {
    s = *scan++;
    चयन (OP(s)) {
    हाल OPLUS_:
      plusnest++;
      अवरोध;
    हाल O_PLUS:
      अगर (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      अवरोध;
    }
  } जबतक (OP(s) != OEND);
  अगर (plusnest != 0)
    g->iflags |= REGEX_BAD;
  वापस (maxnest);
}
