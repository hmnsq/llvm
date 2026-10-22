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
ساکن ساخت cclass {
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
    {تہی, 0, ""}};

/* character-name table */
ساکن ساخت cname {
  ثابت حرف *name;
  حرف رمز_2;
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
              {تہی, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
ساخت parse {
  ثابت حرف *اگلا; /* next character in RE */
  ثابت حرف *انتہا;  /* end of string (-> NUL normally) */
  صحیح error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  صحیح ncsalloc;     /* number of csets allocated */
  ساخت re_guts *g;
#تعریف_کرو NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ساکن خالی p_ere(ساخت parse *, صحیح);
ساکن خالی p_ere_exp(ساخت parse *);
ساکن خالی p_str(ساخت parse *);
ساکن خالی p_bre(ساخت parse *, صحیح, صحیح);
ساکن صحیح p_simp_re(ساخت parse *, صحیح);
ساکن صحیح p_گنتی(ساخت parse *);
ساکن خالی p_bracket(ساخت parse *);
ساکن خالی p_b_رکن(ساخت parse *, cset *);
ساکن خالی p_b_cclass(ساخت parse *, cset *);
ساکن خالی p_b_eclass(ساخت parse *, cset *);
ساکن حرف p_b_علامت(ساخت parse *);
ساکن حرف p_b_coll_elem(ساخت parse *, صحیح);
ساکن حرف othercase(صحیح);
ساکن خالی bothcases(ساخت parse *, صحیح);
ساکن خالی uncased(ساخت parse *, صحیح);
ساکن خالی ordinary(ساخت parse *, صحیح);
ساکن خالی nonnewline(ساخت parse *);
ساکن خالی repeat(ساخت parse *, sopno, صحیح, صحیح);
ساکن صحیح seterr(ساخت parse *, صحیح);
ساکن cset *allocset(ساخت parse *);
ساکن خالی freeset(ساخت parse *, cset *);
ساکن صحیح freezeset(ساخت parse *, cset *);
ساکن صحیح firstch(ساخت parse *, cset *);
ساکن صحیح nch(ساخت parse *, cset *);
ساکن خالی mcadd(ساخت parse *, cset *, ثابت حرف *);
ساکن خالی mcinvert(ساخت parse *, cset *);
ساکن خالی mccase(ساخت parse *, cset *);
ساکن صحیح isinsets(ساخت re_guts *, صحیح);
ساکن صحیح samesets(ساخت re_guts *, صحیح, صحیح);
ساکن خالی categorize(ساخت parse *, ساخت re_guts *);
ساکن sopno dupl(ساخت parse *, sopno, sopno);
ساکن خالی doemit(ساخت parse *, sop, size_t);
ساکن خالی doinsert(ساخت parse *, sop, size_t, sopno);
ساکن خالی dofwd(ساخت parse *, sopno, sop);
ساکن خالی enlarge(ساخت parse *, sopno);
ساکن خالی stripsnug(ساخت parse *, ساخت re_guts *);
ساکن خالی findmust(ساخت parse *, ساخت re_guts *);
ساکن sopno pluscount(ساخت parse *, ساخت re_guts *);

ساکن حرف nuls[10]; /* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#تعریف_کرو PEEK() (*p->اگلا)
#تعریف_کرو PEEK2() (*(p->اگلا + 1))
#تعریف_کرو مزید() (p->انتہا - p->اگلا > 0)
#تعریف_کرو MORE2() (p->انتہا - p->اگلا > 1)
#تعریف_کرو SEE(c) (مزید() && PEEK() == (c))
#تعریف_کرو SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#تعریف_کرو EAT(c) ((SEE(c)) ? (اگلا_3(), 1) : 0)
#تعریف_کرو EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#تعریف_کرو اگلا_3() (p->اگلا++)
#تعریف_کرو NEXT2() (p->اگلا += 2)
#تعریف_کرو NEXTn(n) (p->اگلا += (n))
#تعریف_کرو GETNEXT() (*p->اگلا++)
#تعریف_کرو SETERROR(e) seterr(p, (e))
#تعریف_کرو REQUIRE(co, e) (خالی)((co) || SETERROR(e))
#تعریف_کرو MUSTSEE(c, e) (REQUIRE(مزید() && PEEK() == (c), e))
#تعریف_کرو MUSTEAT(c, e) (REQUIRE(مزید() && GETNEXT() == (c), e))
#تعریف_کرو MUSTNOTSEE(c, e) (REQUIRE(!مزید() || PEEK() != (c), e))
#تعریف_کرو EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#تعریف_کرو داخل_کرو_3(op, pos) doinsert(p, (sop)(op), یہاں() - (pos) + 1, pos)
#تعریف_کرو AHEAD(pos) dofwd(p, pos, یہاں() - (pos))
#تعریف_کرو ASTERN(sop, pos) EMIT(sop, یہاں() - pos)
#تعریف_کرو یہاں() (p->slen)
#تعریف_کرو THERE() (p->slen - 1)
#تعریف_کرو THERETHERE() (p->slen - 2)
#تعریف_کرو DROP(n) (p->slen -= (n))

#اگر_تعریف_شدہ _POSIX2_RE_DUP_MAX
#تعریف_کرو DUPMAX _POSIX2_RE_DUP_MAX
#ورنہ
#تعریف_کرو DUPMAX 255
#اگر_ختم
#تعریف_کرو REGINFINITY (DUPMAX + 1)

#اگر_غیر_تعریف_شدہ NDEBUG
ساکن صحیح never = 0; /* for use in asserts; shuts lint up */
#ورنہ
#تعریف_کرو never 0 /* some <assert.h>s have bugs too */
#اگر_ختم

/*
 - llvm_regcomp - interface for parser and compilation
 */
صحیح /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ثابت حرف *نمونہ_2, صحیح cflags) {
  ساخت parse pa;
  ساخت re_guts *g;
  ساخت parse *p = &pa;
  صحیح i;
  size_t len;
#اگر_تعریف_شدہ REDEBUG
#تعریف_کرو GOODFLAGS(f) (f)
#ورنہ
#تعریف_کرو GOODFLAGS(f) ((f) & ~REG_DUMP)
#اگر_ختم

  cflags = GOODFLAGS(cflags);
  اگر ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    واپس (REG_INVARG);

  اگر (cflags & REG_PEND) {
    اگر (preg->re_endp < نمونہ_2)
      واپس (REG_INVARG);
    len = preg->re_endp - نمونہ_2;
  } ورنہ {
    len = لڑی_لمبائی(نمونہ_2);
  }

  /* do the mallocs early so failure handling is easy */
  g = (ساخت re_guts *)حافظہ_دو(حجم(ساخت re_guts) +
                               (NC - 1) * حجم(cat_t));
  اگر (g == تہی)
    واپس (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)صفر_حافظہ_دو(p->ssize, حجم(sop));
  p->slen = 0;
  اگر (p->strip == تہی) {
    آزاد((حرف *)g);
    واپس (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->اگلا = نمونہ_2;
  p->انتہا = p->اگلا + len;
  p->error = 0;
  p->ncsalloc = 0;
  برائے (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = تہی;
  g->setbits = تہی;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = تہی;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (خالی)memset((حرف *)g->catspace, 0, NC * حجم(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  اگر (cflags & REG_EXTENDED)
    p_ere(p, برآمد_3);
  ورنہ اگر (cflags & REG_NOSPEC)
    p_str(p);
  ورنہ
    p_bre(p, برآمد_3, برآمد_3);
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
#اگر_غیر_تعریف_شدہ REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  اگر (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#اگر_ختم

  /* win or lose, we're done */
  اگر (p->error != 0) /* lose */
    llvm_regfree(preg);
  واپس (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
ساکن خالی p_ere(ساخت parse *p,
                  صحیح stop) /* character this ERE should end at */
{
  حرف c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  صحیح پہلا_2 = 1; /* is this the first alternative? */

  برائے (;;) {
    /* do a bunch of concatenated expressions */
    conc = یہاں();
    جب (مزید() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(یہاں() != conc, REG_خالی_خانے); /* require nonempty */

    اگر (!EAT('|'))
      توڑو; /* NOTE BREAK OUT */

    اگر (پہلا_2) {
      داخل_کرو_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      پہلا_2 = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = یہاں();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  اگر (!پہلا_2) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  أكد(!مزید() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ساکن خالی p_ere_exp(ساخت parse *p) {
  حرف c;
  sopno pos;
  صحیح گنتی;
  صحیح count2;
  صحیح backrefnum;
  sopno subno;
  صحیح wascaret = 0;

  أكد(مزید()); /* caller should have ensured this */
  c = GETNEXT();

  pos = یہاں();
  انتخاب (c) {
  حالت '(':
    REQUIRE(مزید(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    اگر (subno < NPAREN)
      p->pbegin[subno] = یہاں();
    EMIT(OLPAREN, subno);
    اگر (!SEE(')'))
      p_ere(p, ')');
    اگر (subno < NPAREN) {
      p->pend[subno] = یہاں();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    توڑو;
#اگر_غیر_تعریف_شدہ POSIX_MISTAKE
  حالت ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    توڑو;
#اگر_ختم
  حالت '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    توڑو;
  حالت '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    توڑو;
  حالت '|':
    SETERROR(REG_خالی_خانے);
    توڑو;
  حالت '*':
  حالت '+':
  حالت '?':
    SETERROR(REG_BADRPT);
    توڑو;
  حالت '.':
    اگر (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ورنہ
      EMIT(OANY, 0);
    توڑو;
  حالت '[':
    p_bracket(p);
    توڑو;
  حالت '\\':
    REQUIRE(مزید(), REG_EESCAPE);
    c = GETNEXT();
    اگر (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } ورنہ اگر (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      جب (مزید() && ہندسہ_ہے(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } ورنہ اگر (c == 'n') {
      ordinary(p, '\n');
      توڑو;
    } ورنہ اگر (c == 't') {
      ordinary(p, '\t');
      توڑو;
    } ورنہ اگر (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      حرف hexstr[3] = {0};
      حرف *hexp;
      صحیح val;
      اگر (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        اگر (*hexp == '\0') {
          NEXT2();
          uncased(p, (حرف)val);
          توڑو;
        }
      }
      ordinary(p, 'x');
      توڑو;
    } ورنہ {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      توڑو;
    }

    اگر (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      توڑو;
    }

    اگر (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      توڑو;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    أكد(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    أكد(p->pbegin[backrefnum] != 0);
    أكد(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    أكد(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (خالی)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_عقب, backrefnum);
    p->g->backrefs = 1;
    توڑو;
  حالت '{': /* okay as ordinary except if digit follows */
    REQUIRE(!مزید() || !ہندسہ_ہے((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  طےشدہ:
    ordinary(p, c);
    توڑو;
  }

  اگر (!مزید())
    واپس;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  اگر (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ہندسہ_ہے((uch)PEEK2()))))
    واپس; /* no repetition, we're done */
  اگلا_3();

  REQUIRE(!wascaret, REG_BADRPT);
  انتخاب (c) {
  حالت '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    داخل_کرو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    داخل_کرو_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    توڑو;
  حالت '+':
    داخل_کرو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    توڑو;
  حالت '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    داخل_کرو_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    توڑو;
  حالت '{':
    گنتی = p_گنتی(p);
    اگر (EAT(',')) {
      اگر (ہندسہ_ہے((uch)PEEK())) {
        count2 = p_گنتی(p);
        REQUIRE(گنتی <= count2, REG_BADBR);
      } ورنہ /* single number with comma */
        count2 = REGINFINITY;
    } ورنہ /* just a single number */
      count2 = گنتی;
    repeat(p, pos, گنتی, count2);
    اگر (!EAT('}')) { /* error heuristics */
      جب (مزید() && PEEK() != '}')
        اگلا_3();
      REQUIRE(مزید(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    توڑو;
  }

  اگر (!مزید())
    واپس;
  c = PEEK();
  اگر (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && ہندسہ_ہے((uch)PEEK2()))))
    واپس;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ساکن خالی p_str(ساخت parse *p) {
  REQUIRE(مزید(), REG_خالی_خانے);
  جب (مزید())
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
ساکن خالی p_bre(ساخت parse *p, صحیح end1, /* first terminating character */
                  صحیح end2)                  /* second terminating character */
{
  sopno آغاز_2 = یہاں();
  صحیح پہلا_2 = 1; /* first subexpression? */
  صحیح wasdollar = 0;

  اگر (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  جب (مزید() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, پہلا_2);
    پہلا_2 = 0;
  }
  اگر (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(یہاں() != آغاز_2, REG_خالی_خانے); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
ساکن صحیح /* was the simple RE an unbackslashed $? */
p_simp_re(ساخت parse *p,
          صحیح starordinary) /* is a leading * an ordinary character? */
{
  صحیح c;
  صحیح گنتی;
  صحیح count2;
  sopno pos;
  صحیح i;
  sopno subno;
#تعریف_کرو BACKSL (1 << CHAR_BIT)

  pos = یہاں(); /* repetition op, if any, covers from here */

  أكد(مزید()); /* caller should have ensured this */
  c = GETNEXT();
  اگر (c == '\\') {
    REQUIRE(مزید(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  انتخاب (c) {
  حالت '.':
    اگر (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    ورنہ
      EMIT(OANY, 0);
    توڑو;
  حالت '[':
    p_bracket(p);
    توڑو;
  حالت BACKSL | '{':
    SETERROR(REG_BADRPT);
    توڑو;
  حالت BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    اگر (subno < NPAREN)
      p->pbegin[subno] = یہاں();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    اگر (مزید() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    اگر (subno < NPAREN) {
      p->pend[subno] = یہاں();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    توڑو;
  حالت BACKSL | ')': /* should not get here -- must be user */
  حالت BACKSL | '}':
    SETERROR(REG_EPAREN);
    توڑو;
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
      (خالی)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_عقب, i);
    } ورنہ {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    توڑو;
  حالت '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  طےشدہ:
    ordinary(p, (حرف)c);
    توڑو;
  }

  اگر (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    داخل_کرو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    داخل_کرو_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } ورنہ اگر (EATTWO('\\', '{')) {
    گنتی = p_گنتی(p);
    اگر (EAT(',')) {
      اگر (مزید() && ہندسہ_ہے((uch)PEEK())) {
        count2 = p_گنتی(p);
        REQUIRE(گنتی <= count2, REG_BADBR);
      } ورنہ /* single number with comma */
        count2 = REGINFINITY;
    } ورنہ /* just a single number */
      count2 = گنتی;
    repeat(p, pos, گنتی, count2);
    اگر (!EATTWO('\\', '}')) { /* error heuristics */
      جب (مزید() && !SEETWO('\\', '}'))
        اگلا_3();
      REQUIRE(مزید(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } ورنہ اگر (c == '$') /* $ (but not \$) ends it */
    واپس (1);

  واپس (0);
}

/*
 - p_count - parse a repetition count
 */
ساکن صحیح /* the value */
p_گنتی(ساخت parse *p) {
  صحیح گنتی = 0;
  صحیح ndigits = 0;

  جب (مزید() && ہندسہ_ہے((uch)PEEK()) && گنتی <= DUPMAX) {
    گنتی = گنتی * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && گنتی <= DUPMAX, REG_BADBR);
  واپس (گنتی);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
ساکن خالی p_bracket(ساخت parse *p) {
  cset *cs;
  صحیح invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  اگر (p->انتہا - p->اگلا > 5) {
    اگر (لڑی_موازنہ(p->اگلا, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      واپس;
    }
    اگر (لڑی_موازنہ(p->اگلا, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      واپس;
    }
  }

  اگر ((cs = allocset(p)) == تہی) {
    /* allocset did set error status in p */
    واپس;
  }

  اگر (EAT('^'))
    invert++; /* make note to invert set at end */
  اگر (EAT(']'))
    CHadd(cs, ']');
  ورنہ اگر (EAT('-'))
    CHadd(cs, '-');
  جب (مزید() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_رکن(p, cs);
  اگر (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  اگر (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    واپس;
  }

  اگر (p->g->cflags & REG_ICASE) {
    صحیح i;
    صحیح ci;

    برائے (i = p->g->csetsize - 1; i >= 0; i--)
      اگر (CHIN(cs, i) && حرف_ہے(i)) {
        ci = othercase(i);
        اگر (ci != i)
          CHadd(cs, ci);
      }
    اگر (cs->multis != تہی)
      mccase(p, cs);
  }
  اگر (invert) {
    صحیح i;

    برائے (i = p->g->csetsize - 1; i >= 0; i--)
      اگر (CHIN(cs, i))
        CHsub(cs, i);
      ورنہ
        CHadd(cs, i);
    اگر (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    اگر (cs->multis != تہی)
      mcinvert(p, cs);
  }

  أكد(cs->multis == تہی); /* xxx */

  اگر (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } ورنہ {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ساکن خالی p_b_رکن(ساخت parse *p, cset *cs) {
  حرف c;
  حرف آغاز_2, اختتام_2;
  صحیح i;

  /* classify what we've got */
  انتخاب ((مزید()) ? PEEK() : '\0') {
  حالت '[':
    c = (MORE2()) ? PEEK2() : '\0';
    توڑو;
  حالت '-':
    SETERROR(REG_ERANGE);
    واپس; /* NOTE RETURN */
    توڑو;
  طےشدہ:
    c = '\0';
    توڑو;
  }

  انتخاب (c) {
  حالت ':': /* character class */
    NEXT2();
    REQUIRE(مزید(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(مزید(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    توڑو;
  حالت '=': /* equivalence class */
    NEXT2();
    REQUIRE(مزید(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(مزید(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    توڑو;
  طےشدہ: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    آغاز_2 = p_b_علامت(p);
    اگر (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      اگلا_3();
      اگر (EAT('-'))
        اختتام_2 = '-';
      ورنہ
        اختتام_2 = p_b_علامت(p);
    } ورنہ {
      اختتام_2 = آغاز_2;
    }
    /* xxx what about signed chars here... */
    REQUIRE(آغاز_2 <= اختتام_2, REG_ERANGE);
    برائے (i = آغاز_2; i <= اختتام_2; i++)
      CHadd(cs, i);
    توڑو;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ساکن خالی p_b_cclass(ساخت parse *p, cset *cs) {
  ثابت حرف *sp = p->اگلا;
  ساخت cclass *cp;
  size_t len;
  ثابت حرف *u;
  حرف c;

  جب (مزید() && حرف_ہے((uch)PEEK()))
    اگلا_3();
  len = p->اگلا - sp;
  برائے (cp = cclasses; cp->name != تہی; cp++)
    اگر (لڑی_موازنہ(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      توڑو;
  اگر (cp->name == تہی) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    واپس;
  }

  u = cp->chars;
  جب ((c = *u++) != '\0')
    CHadd(cs, c);
  برائے (u = cp->multis; *u != '\0'; u += لڑی_لمبائی(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
ساکن خالی p_b_eclass(ساخت parse *p, cset *cs) {
  حرف c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
ساکن حرف /* value of symbol */
p_b_علامت(ساخت parse *p) {
  حرف قدر_2;

  REQUIRE(مزید(), REG_EBRACK);
  اگر (!EATTWO('[', '.'))
    واپس (GETNEXT());

  /* collating symbol */
  قدر_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  واپس (قدر_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ساکن حرف                              /* value of collating element */
p_b_coll_elem(ساخت parse *p, صحیح endc) /* name ended by endc,']' */
{
  ثابت حرف *sp = p->اگلا;
  ساخت cname *cp;
  size_t len;

  جب (مزید() && !SEETWO(endc, ']'))
    اگلا_3();
  اگر (!مزید()) {
    SETERROR(REG_EBRACK);
    واپس (0);
  }
  len = p->اگلا - sp;
  برائے (cp = cnames; cp->name != تہی; cp++)
    اگر (لڑی_موازنہ(cp->name, sp, len) == 0 && لڑی_لمبائی(cp->name) == len)
      واپس (cp->رمز_2); /* known name */
  اگر (len == 1)
    واپس (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  واپس (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ساکن حرف /* if no counterpart, return ch */
othercase(صحیح ch) {
  ch = (uch)ch;
  أكد(حرف_ہے(ch));
  اگر (isupper(ch))
    واپس ((uch)tolower(ch));
  ورنہ اگر (islower(ch))
    واپس ((uch)بڑا_حرف(ch));
  ورنہ /* peculiar, but could happen */
    واپس (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ساکن خالی bothcases(ساخت parse *p, صحیح ch) {
  ثابت حرف *oldnext = p->اگلا;
  ثابت حرف *oldend = p->انتہا;
  حرف bracket[3];

  ch = (uch)ch;
  أكد(othercase(ch) != ch); /* p_bracket() would recurse */
  p->اگلا = bracket;
  p->انتہا = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  أكد(p->اگلا == bracket + 2);
  p->اگلا = oldnext;
  p->انتہا = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ساکن خالی uncased(ساخت parse *p, صحیح ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  اگر (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ساکن خالی ordinary(ساخت parse *p, صحیح ch) {
  اگر ((p->g->cflags & REG_ICASE) && حرف_ہے((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  ورنہ
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ساکن خالی nonnewline(ساخت parse *p) {
  ثابت حرف *oldnext = p->اگلا;
  ثابت حرف *oldend = p->انتہا;
  ساکن ثابت حرف bracket[4] = {'^', '\n', ']', '\0'};

  p->اگلا = bracket;
  p->انتہا = bracket + 3;
  p_bracket(p);
  أكد(p->اگلا == bracket + 3);
  p->اگلا = oldnext;
  p->انتہا = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ساکن خالی repeat(ساخت parse *p,
                   sopno آغاز_2, /* operand from here to end of strip */
                   صحیح from,    /* repeated from this number */
                   صحیح مقصد_2)      /* to this number of times (maybe INFINITY) */
{
  sopno اختتام_2 = یہاں();
#تعریف_کرو N 2
#تعریف_کرو INF 3
#تعریف_کرو REP(f, t) ((f) * 8 + (t))
#تعریف_کرو MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  اگر (p->error != 0) /* head off possible runaway recursion */
    واپس;

  أكد(from <= مقصد_2);

  انتخاب (REP(MAP(from), MAP(مقصد_2))) {
  حالت REP(0, 0):         /* must be user doing this */
    DROP(اختتام_2 - آغاز_2); /* drop the operand */
    توڑو;
  حالت REP(0, 1):   /* as x{1,1}? */
  حالت REP(0, N):   /* as x{1,n}? */
  حالت REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    داخل_کرو_3(OCH_, آغاز_2); /* offset is wrong... */
    repeat(p, آغاز_2 + 1, 1, مقصد_2);
    ASTERN(OOR1, آغاز_2);
    AHEAD(آغاز_2); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    توڑو;
  حالت REP(1, 1): /* trivial case */
    /*مکمل*/
    توڑو;
  حالت REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    داخل_کرو_3(OCH_, آغاز_2);
    ASTERN(OOR1, آغاز_2);
    AHEAD(آغاز_2);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, آغاز_2 + 1, اختتام_2 + 1);
    أكد(copy == اختتام_2 + 4);
    repeat(p, copy, 1, مقصد_2 - 1);
    توڑو;
  حالت REP(1, INF): /* as x+ */
    داخل_کرو_3(OPLUS_, آغاز_2);
    ASTERN(O_PLUS, آغاز_2);
    توڑو;
  حالت REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, آغاز_2, اختتام_2);
    repeat(p, copy, from - 1, مقصد_2 - 1);
    توڑو;
  حالت REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, آغاز_2, اختتام_2);
    repeat(p, copy, from - 1, مقصد_2);
    توڑو;
  طےشدہ:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    توڑو;
  }
}

/*
 - seterr - set an error condition
 */
ساکن صحیح /* useless but makes type checking happy */
seterr(ساخت parse *p, صحیح e) {
  اگر (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->اگلا = nuls; /* try to bring things to a halt */
  p->انتہا = nuls;
  واپس (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
ساکن cset *allocset(ساخت parse *p) {
  صحیح no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  صحیح i;

  اگر (no >= p->ncsalloc) { /* need another column of space */
    خالی *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    اگر (nc > SIZE_MAX / حجم(cset))
      جاؤ nomem;
    أكد(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((حرف *)p->g->sets, nc * حجم(cset));
    اگر (ptr == تہی)
      جاؤ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((حرف *)p->g->setbits, nbytes);
    اگر (ptr == تہی)
      جاؤ nomem;
    p->g->setbits = ptr;

    برائے (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (خالی)memset((حرف *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  اگر (p->g->sets == تہی || p->g->setbits == تہی)
    جاؤ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->نقاب_2 = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = تہی;

  واپس (cs);
nomem:
  آزاد(p->g->sets);
  p->g->sets = تہی;
  آزاد(p->g->setbits);
  p->g->setbits = تہی;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  واپس (تہی);
}

/*
 - freeset - free a now-unused set
 */
ساکن خالی freeset(ساخت parse *p, cset *cs) {
  size_t i;
  cset *چوٹی_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  برائے (i = 0; i < css; i++)
    CHsub(cs, i);
  اگر (cs == چوٹی_2 - 1) /* recover only the easy case */
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
ساکن صحیح /* set number */
freezeset(ساخت parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *چوٹی_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  برائے (cs2 = &p->g->sets[0]; cs2 < چوٹی_2; cs2++)
    اگر (cs2->hash == h && cs2 != cs) {
      /* maybe */
      برائے (i = 0; i < css; i++)
        اگر (!!CHIN(cs2, i) != !!CHIN(cs, i))
          توڑو; /* no */
      اگر (i == css)
        توڑو; /* yes */
    }

  اگر (cs2 < چوٹی_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  واپس ((صحیح)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ساکن صحیح /* character; there is no "none" value */
firstch(ساخت parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  برائے (i = 0; i < css; i++)
    اگر (CHIN(cs, i))
      واپس ((حرف)i);
  أكد(never);
  واپس (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ساکن صحیح nch(ساخت parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  صحیح n = 0;

  برائے (i = 0; i < css; i++)
    اگر (CHIN(cs, i))
      n++;
  واپس (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ساکن خالی mcadd(ساخت parse *p, cset *cs, ثابت حرف *cp) {
  size_t oldend = cs->smultis;
  خالی *np;

  cs->smultis += لڑی_لمبائی(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  اگر (np == تہی) {
    اگر (cs->multis)
      آزاد(cs->multis);
    cs->multis = تہی;
    SETERROR(REG_ESPACE);
    واپس;
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
ساکن خالی mcinvert(ساخت parse *p, cset *cs) {
  أكد(cs->multis == تہی); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
ساکن خالی mccase(ساخت parse *p, cset *cs) {
  أكد(cs->multis == تہی); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
ساکن صحیح /* predicate */
isinsets(ساخت re_guts *g, صحیح c) {
  uch *ستون_2;
  صحیح i;
  صحیح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  غیرعلامتی uc = (uch)c;

  برائے (i = 0, ستون_2 = g->setbits; i < ncols; i++, ستون_2 += g->csetsize)
    اگر (ستون_2[uc] != 0)
      واپس (1);
  واپس (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ساکن صحیح /* predicate */
samesets(ساخت re_guts *g, صحیح c1, صحیح c2) {
  uch *ستون_2;
  صحیح i;
  صحیح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  غیرعلامتی uc1 = (uch)c1;
  غیرعلامتی uc2 = (uch)c2;

  برائے (i = 0, ستون_2 = g->setbits; i < ncols; i++, ستون_2 += g->csetsize)
    اگر (ستون_2[uc1] != ستون_2[uc2])
      واپس (0);
  واپس (1);
}

/*
 - categorize - sort out character categories
 */
ساکن خالی categorize(ساخت parse *p, ساخت re_guts *g) {
  cat_t *cats = g->categories;
  صحیح c;
  صحیح c2;
  cat_t cat;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    واپس;

  برائے (c = CHAR_MIN; c <= CHAR_MAX; c++)
    اگر (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      برائے (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        اگر (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ساکن sopno                       /* start of duplicate */
dupl(ساخت parse *p, sopno آغاز_2, /* from here */
     sopno اختتام_2)                 /* to this less one */
{
  sopno ret = یہاں();
  sopno len = اختتام_2 - آغاز_2;

  أكد(اختتام_2 >= آغاز_2);
  اگر (len == 0)
    واپس (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  أكد(p->ssize >= p->slen + len);
  (خالی)memmove((حرف *)(p->strip + p->slen), (حرف *)(p->strip + آغاز_2),
                (size_t)len * حجم(sop));
  p->slen += len;
  واپس (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
ساکن خالی doemit(ساخت parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  اگر (p->error != 0)
    واپس;

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
ساکن خالی doinsert(ساخت parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  صحیح i;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    واپس;

  sn = یہاں();
  EMIT(op, opnd); /* do checks, ensure space */
  أكد(یہاں() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  أكد(pos > 0);
  برائے (i = 1; i < NPAREN; i++) {
    اگر (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    اگر (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((حرف *)&p->strip[pos + 1], (حرف *)&p->strip[pos],
          (یہاں() - pos - 1) * حجم(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ساکن خالی dofwd(ساخت parse *p, sopno pos, sop قدر_2) {
  /* avoid making error situations worse */
  اگر (p->error != 0)
    واپس;

  أكد(قدر_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | قدر_2;
}

/*
 - enlarge - enlarge the strip
 */
ساکن خالی enlarge(ساخت parse *p, sopno سائز_2) {
  sop *sp;

  اگر (p->ssize >= سائز_2)
    واپس;

  اگر ((uintptr_t)سائز_2 > SIZE_MAX / حجم(sop)) {
    SETERROR(REG_ESPACE);
    واپس;
  }

  sp = (sop *)realloc(p->strip, سائز_2 * حجم(sop));
  اگر (sp == تہی) {
    SETERROR(REG_ESPACE);
    واپس;
  }
  p->strip = sp;
  p->ssize = سائز_2;
}

/*
 - stripsnug - compact the strip
 */
ساکن خالی stripsnug(ساخت parse *p, ساخت re_guts *g) {
  g->nstates = p->slen;
  اگر ((uintptr_t)p->slen > SIZE_MAX / حجم(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    واپس;
  }

  g->strip = (sop *)realloc((حرف *)p->strip, p->slen * حجم(sop));
  اگر (g->strip == تہی) {
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
ساکن خالی findmust(ساخت parse *p, ساخت re_guts *g) {
  sop *جائزہ;
  sop *آغاز_2 = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  حرف *cp;
  sopno i;

  /* avoid making error situations worse */
  اگر (p->error != 0)
    واپس;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  جائزہ = g->strip + 1;
  کرو {
    s = *جائزہ++;
    انتخاب (OP(s)) {
    حالت OCHAR:        /* sequence member */
      اگر (newlen == 0) /* new sequence */
        newstart = جائزہ - 1;
      newlen++;
      توڑو;
    حالت OPLUS_: /* things that don't break one */
    حالت OLPAREN:
    حالت ORPAREN:
      توڑو;
    حالت OQUEST_: /* things that must be skipped */
    حالت OCH_:
      جائزہ--;
      کرو {
        جائزہ += OPND(s);
        s = *جائزہ;
        /* assert() interferes w debug printouts */
        اگر (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          واپس;
        }
      } جب (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    طےشدہ:                  /* things that break a sequence */
      اگر (newlen > g->mlen) { /* ends one */
        آغاز_2 = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      توڑو;
    }
  } جب (OP(s) != OEND);

  اگر (g->mlen == 0) /* there isn't one */
    واپس;

  /* turn it into a character string */
  g->must = حافظہ_دو((size_t)g->mlen + 1);
  اگر (g->must == تہی) { /* argh; just forget it */
    g->mlen = 0;
    واپس;
  }
  cp = g->must;
  جائزہ = آغاز_2;
  برائے (i = g->mlen; i > 0; i--) {
    جب (OP(s = *جائزہ++) != OCHAR)
      جاری;
    أكد(cp < g->must + g->mlen);
    *cp++ = (حرف)OPND(s);
  }
  أكد(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
ساکن sopno /* nesting depth */
pluscount(ساخت parse *p, ساخت re_guts *g) {
  sop *جائزہ;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  اگر (p->error != 0)
    واپس (0); /* there may not be an OEND */

  جائزہ = g->strip + 1;
  کرو {
    s = *جائزہ++;
    انتخاب (OP(s)) {
    حالت OPLUS_:
      plusnest++;
      توڑو;
    حالت O_PLUS:
      اگر (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      توڑو;
    }
  } جب (OP(s) != OEND);
  اگر (plusnest != 0)
    g->iflags |= REGEX_BAD;
  واپس (maxnest);
}
