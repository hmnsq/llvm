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
  حرف ڪوڊ_2;
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
  ثابت حرف *ايندڙ; /* next character in RE */
  ثابت حرف *پڇاڙي;  /* end of string (-> NUL normally) */
  صحيح error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  صحيح ncsalloc;     /* number of csets allocated */
  بنية re_guts *g;
#وصف_ڪريو NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

ساكن فراغ p_ere(بنية parse *, صحيح);
ساكن فراغ p_ere_exp(بنية parse *);
ساكن فراغ p_str(بنية parse *);
ساكن فراغ p_bre(بنية parse *, صحيح, صحيح);
ساكن صحيح p_simp_re(بنية parse *, صحيح);
ساكن صحيح p_ڳڻپ(بنية parse *);
ساكن فراغ p_bracket(بنية parse *);
ساكن فراغ p_b_رڪن(بنية parse *, cset *);
ساكن فراغ p_b_cclass(بنية parse *, cset *);
ساكن فراغ p_b_eclass(بنية parse *, cset *);
ساكن حرف p_b_نشان(بنية parse *);
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
#وصف_ڪريو PEEK() (*p->ايندڙ)
#وصف_ڪريو PEEK2() (*(p->ايندڙ + 1))
#وصف_ڪريو وڌيڪ() (p->پڇاڙي - p->ايندڙ > 0)
#وصف_ڪريو MORE2() (p->پڇاڙي - p->ايندڙ > 1)
#وصف_ڪريو SEE(c) (وڌيڪ() && PEEK() == (c))
#وصف_ڪريو SEETWO(a, b) (MORE2() && PEEK() == (a) && PEEK2() == (b))
#وصف_ڪريو EAT(c) ((SEE(c)) ? (ايندڙ_3(), 1) : 0)
#وصف_ڪريو EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#وصف_ڪريو ايندڙ_3() (p->ايندڙ++)
#وصف_ڪريو NEXT2() (p->ايندڙ += 2)
#وصف_ڪريو NEXTn(n) (p->ايندڙ += (n))
#وصف_ڪريو GETNEXT() (*p->ايندڙ++)
#وصف_ڪريو SETERROR(e) seterr(p, (e))
#وصف_ڪريو REQUIRE(co, e) (فراغ)((co) || SETERROR(e))
#وصف_ڪريو MUSTSEE(c, e) (REQUIRE(وڌيڪ() && PEEK() == (c), e))
#وصف_ڪريو MUSTEAT(c, e) (REQUIRE(وڌيڪ() && GETNEXT() == (c), e))
#وصف_ڪريو MUSTNOTSEE(c, e) (REQUIRE(!وڌيڪ() || PEEK() != (c), e))
#وصف_ڪريو EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#وصف_ڪريو وجهو_3(op, pos) doinsert(p, (sop)(op), هتي() - (pos) + 1, pos)
#وصف_ڪريو AHEAD(pos) dofwd(p, pos, هتي() - (pos))
#وصف_ڪريو ASTERN(sop, pos) EMIT(sop, هتي() - pos)
#وصف_ڪريو هتي() (p->slen)
#وصف_ڪريو THERE() (p->slen - 1)
#وصف_ڪريو THERETHERE() (p->slen - 2)
#وصف_ڪريو DROP(n) (p->slen -= (n))

#جيڪڏهن_وصف_ٿيل _POSIX2_RE_DUP_MAX
#وصف_ڪريو DUPMAX _POSIX2_RE_DUP_MAX
#وإلا
#وصف_ڪريو DUPMAX 255
#جيڪڏهن_پورو
#وصف_ڪريو REGINFINITY (DUPMAX + 1)

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
ساكن صحيح never = 0; /* for use in asserts; shuts lint up */
#وإلا
#وصف_ڪريو never 0 /* some <assert.h>s have bugs too */
#جيڪڏهن_پورو

/*
 - llvm_regcomp - interface for parser and compilation
 */
صحيح /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, ثابت حرف *نمونو_2, صحيح cflags) {
  بنية parse pa;
  بنية re_guts *g;
  بنية parse *p = &pa;
  صحيح i;
  size_t len;
#جيڪڏهن_وصف_ٿيل REDEBUG
#وصف_ڪريو GOODFLAGS(f) (f)
#وإلا
#وصف_ڪريو GOODFLAGS(f) ((f) & ~REG_DUMP)
#جيڪڏهن_پورو

  cflags = GOODFLAGS(cflags);
  إذا ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    أرجع (REG_INVARG);

  إذا (cflags & REG_PEND) {
    إذا (preg->re_endp < نمونو_2)
      أرجع (REG_INVARG);
    len = preg->re_endp - نمونو_2;
  } وإلا {
    len = لڙي_ڊيگهه(نمونو_2);
  }

  /* do the mallocs early so failure handling is easy */
  g = (بنية re_guts *)يادگيري_ڏيو(حجم(بنية re_guts) +
                               (NC - 1) * حجم(cat_t));
  إذا (g == عدم)
    أرجع (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)صفر_يادگيري_ڏيو(p->ssize, حجم(sop));
  p->slen = 0;
  إذا (p->strip == عدم) {
    حرر((حرف *)g);
    أرجع (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->ايندڙ = نمونو_2;
  p->پڇاڙي = p->ايندڙ + len;
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
  إذا (cflags & REG_EXTENDED)
    p_ere(p, ٻاهريون_3);
  وإلا إذا (cflags & REG_NOSPEC)
    p_str(p);
  وإلا
    p_bre(p, ٻاهريون_3, ٻاهريون_3);
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
#جيڪڏهن_وصف_نه_ٿيل REDEBUG
  /* not debugging, so can't rely on the assert() in llvm_regexec() */
  إذا (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#جيڪڏهن_پورو

  /* win or lose, we're done */
  إذا (p->error != 0) /* lose */
    llvm_regfree(preg);
  أرجع (p->error);
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
  صحيح پهريون_2 = 1; /* is this the first alternative? */

  لكل (;;) {
    /* do a bunch of concatenated expressions */
    conc = هتي();
    طالما (وڌيڪ() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(هتي() != conc, REG_خالي_جايون); /* require nonempty */

    إذا (!EAT('|'))
      اكسر; /* NOTE BREAK OUT */

    إذا (پهريون_2) {
      وجهو_3(OCH_, conc); /* offset is wrong */
      prevfwd = conc;
      prevback = conc;
      پهريون_2 = 0;
    }
    ASTERN(OOR1, prevback);
    prevback = THERE();
    AHEAD(prevfwd); /* fix previous offset */
    prevfwd = هتي();
    EMIT(OOR2, 0); /* offset is very wrong */
  }

  إذا (!پهريون_2) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  أكد(!وڌيڪ() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
ساكن فراغ p_ere_exp(بنية parse *p) {
  حرف c;
  sopno pos;
  صحيح ڳڻپ;
  صحيح count2;
  صحيح backrefnum;
  sopno subno;
  صحيح wascaret = 0;

  أكد(وڌيڪ()); /* caller should have ensured this */
  c = GETNEXT();

  pos = هتي();
  بدل (c) {
  حالة '(':
    REQUIRE(وڌيڪ(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    إذا (subno < NPAREN)
      p->pbegin[subno] = هتي();
    EMIT(OLPAREN, subno);
    إذا (!SEE(')'))
      p_ere(p, ')');
    إذا (subno < NPAREN) {
      p->pend[subno] = هتي();
      أكد(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    اكسر;
#جيڪڏهن_وصف_نه_ٿيل POSIX_MISTAKE
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
#جيڪڏهن_پورو
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
    SETERROR(REG_خالي_جايون);
    اكسر;
  حالة '*':
  حالة '+':
  حالة '?':
    SETERROR(REG_BADRPT);
    اكسر;
  حالة '.':
    إذا (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    وإلا
      EMIT(OANY, 0);
    اكسر;
  حالة '[':
    p_bracket(p);
    اكسر;
  حالة '\\':
    REQUIRE(وڌيڪ(), REG_EESCAPE);
    c = GETNEXT();
    إذا (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } وإلا إذا (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      طالما (وڌيڪ() && انگ_آهي(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } وإلا إذا (c == 'n') {
      ordinary(p, '\n');
      اكسر;
    } وإلا إذا (c == 't') {
      ordinary(p, '\t');
      اكسر;
    } وإلا إذا (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      حرف hexstr[3] = {0};
      حرف *hexp;
      صحيح val;
      إذا (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        إذا (*hexp == '\0') {
          NEXT2();
          uncased(p, (حرف)val);
          اكسر;
        }
      }
      ordinary(p, 'x');
      اكسر;
    } وإلا {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      اكسر;
    }

    إذا (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      اكسر;
    }

    إذا (p->pend[backrefnum] == 0) {
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
    EMIT(O_پٺ, backrefnum);
    p->g->backrefs = 1;
    اكسر;
  حالة '{': /* okay as ordinary except if digit follows */
    REQUIRE(!وڌيڪ() || !انگ_آهي((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  افتراضي:
    ordinary(p, c);
    اكسر;
  }

  إذا (!وڌيڪ())
    أرجع;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  إذا (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && انگ_آهي((uch)PEEK2()))))
    أرجع; /* no repetition, we're done */
  ايندڙ_3();

  REQUIRE(!wascaret, REG_BADRPT);
  بدل (c) {
  حالة '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    وجهو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    وجهو_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    اكسر;
  حالة '+':
    وجهو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    اكسر;
  حالة '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    وجهو_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    اكسر;
  حالة '{':
    ڳڻپ = p_ڳڻپ(p);
    إذا (EAT(',')) {
      إذا (انگ_آهي((uch)PEEK())) {
        count2 = p_ڳڻپ(p);
        REQUIRE(ڳڻپ <= count2, REG_BADBR);
      } وإلا /* single number with comma */
        count2 = REGINFINITY;
    } وإلا /* just a single number */
      count2 = ڳڻپ;
    repeat(p, pos, ڳڻپ, count2);
    إذا (!EAT('}')) { /* error heuristics */
      طالما (وڌيڪ() && PEEK() != '}')
        ايندڙ_3();
      REQUIRE(وڌيڪ(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    اكسر;
  }

  إذا (!وڌيڪ())
    أرجع;
  c = PEEK();
  إذا (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && انگ_آهي((uch)PEEK2()))))
    أرجع;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
ساكن فراغ p_str(بنية parse *p) {
  REQUIRE(وڌيڪ(), REG_خالي_جايون);
  طالما (وڌيڪ())
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
  sopno شروعات_2 = هتي();
  صحيح پهريون_2 = 1; /* first subexpression? */
  صحيح wasdollar = 0;

  إذا (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  طالما (وڌيڪ() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, پهريون_2);
    پهريون_2 = 0;
  }
  إذا (wasdollar) { /* oops, that was a trailing anchor */
    DROP(1);
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
  }

  REQUIRE(هتي() != شروعات_2, REG_خالي_جايون); /* require nonempty */
}

/*
 - p_simp_re - parse a simple RE, an atom possibly followed by a repetition
 */
ساكن صحيح /* was the simple RE an unbackslashed $? */
p_simp_re(بنية parse *p,
          صحيح starordinary) /* is a leading * an ordinary character? */
{
  صحيح c;
  صحيح ڳڻپ;
  صحيح count2;
  sopno pos;
  صحيح i;
  sopno subno;
#وصف_ڪريو BACKSL (1 << CHAR_BIT)

  pos = هتي(); /* repetition op, if any, covers from here */

  أكد(وڌيڪ()); /* caller should have ensured this */
  c = GETNEXT();
  إذا (c == '\\') {
    REQUIRE(وڌيڪ(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  بدل (c) {
  حالة '.':
    إذا (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    وإلا
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
    إذا (subno < NPAREN)
      p->pbegin[subno] = هتي();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    إذا (وڌيڪ() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    إذا (subno < NPAREN) {
      p->pend[subno] = هتي();
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
    إذا (p->pend[i] != 0) {
      أكد(i <= p->g->nsub);
      EMIT(OBACK_, i);
      أكد(p->pbegin[i] != 0);
      أكد(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      أكد(OP(p->strip[p->pend[i]]) == ORPAREN);
      (فراغ)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_پٺ, i);
    } وإلا {
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

  إذا (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    وجهو_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    وجهو_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } وإلا إذا (EATTWO('\\', '{')) {
    ڳڻپ = p_ڳڻپ(p);
    إذا (EAT(',')) {
      إذا (وڌيڪ() && انگ_آهي((uch)PEEK())) {
        count2 = p_ڳڻپ(p);
        REQUIRE(ڳڻپ <= count2, REG_BADBR);
      } وإلا /* single number with comma */
        count2 = REGINFINITY;
    } وإلا /* just a single number */
      count2 = ڳڻپ;
    repeat(p, pos, ڳڻپ, count2);
    إذا (!EATTWO('\\', '}')) { /* error heuristics */
      طالما (وڌيڪ() && !SEETWO('\\', '}'))
        ايندڙ_3();
      REQUIRE(وڌيڪ(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } وإلا إذا (c == '$') /* $ (but not \$) ends it */
    أرجع (1);

  أرجع (0);
}

/*
 - p_count - parse a repetition count
 */
ساكن صحيح /* the value */
p_ڳڻپ(بنية parse *p) {
  صحيح ڳڻپ = 0;
  صحيح ndigits = 0;

  طالما (وڌيڪ() && انگ_آهي((uch)PEEK()) && ڳڻپ <= DUPMAX) {
    ڳڻپ = ڳڻپ * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ڳڻپ <= DUPMAX, REG_BADBR);
  أرجع (ڳڻپ);
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
  إذا (p->پڇاڙي - p->ايندڙ > 5) {
    إذا (لڙي_ڀيٽ(p->ايندڙ, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      أرجع;
    }
    إذا (لڙي_ڀيٽ(p->ايندڙ, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      أرجع;
    }
  }

  إذا ((cs = allocset(p)) == عدم) {
    /* allocset did set error status in p */
    أرجع;
  }

  إذا (EAT('^'))
    invert++; /* make note to invert set at end */
  إذا (EAT(']'))
    CHadd(cs, ']');
  وإلا إذا (EAT('-'))
    CHadd(cs, '-');
  طالما (وڌيڪ() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_رڪن(p, cs);
  إذا (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  إذا (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    أرجع;
  }

  إذا (p->g->cflags & REG_ICASE) {
    صحيح i;
    صحيح ci;

    لكل (i = p->g->csetsize - 1; i >= 0; i--)
      إذا (CHIN(cs, i) && اکر_آهي(i)) {
        ci = othercase(i);
        إذا (ci != i)
          CHadd(cs, ci);
      }
    إذا (cs->multis != عدم)
      mccase(p, cs);
  }
  إذا (invert) {
    صحيح i;

    لكل (i = p->g->csetsize - 1; i >= 0; i--)
      إذا (CHIN(cs, i))
        CHsub(cs, i);
      وإلا
        CHadd(cs, i);
    إذا (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    إذا (cs->multis != عدم)
      mcinvert(p, cs);
  }

  أكد(cs->multis == عدم); /* xxx */

  إذا (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } وإلا {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
ساكن فراغ p_b_رڪن(بنية parse *p, cset *cs) {
  حرف c;
  حرف شروعات_2, پڄاڻي;
  صحيح i;

  /* classify what we've got */
  بدل ((وڌيڪ()) ? PEEK() : '\0') {
  حالة '[':
    c = (MORE2()) ? PEEK2() : '\0';
    اكسر;
  حالة '-':
    SETERROR(REG_ERANGE);
    أرجع; /* NOTE RETURN */
    اكسر;
  افتراضي:
    c = '\0';
    اكسر;
  }

  بدل (c) {
  حالة ':': /* character class */
    NEXT2();
    REQUIRE(وڌيڪ(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(وڌيڪ(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    اكسر;
  حالة '=': /* equivalence class */
    NEXT2();
    REQUIRE(وڌيڪ(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(وڌيڪ(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    اكسر;
  افتراضي: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    شروعات_2 = p_b_نشان(p);
    إذا (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      ايندڙ_3();
      إذا (EAT('-'))
        پڄاڻي = '-';
      وإلا
        پڄاڻي = p_b_نشان(p);
    } وإلا {
      پڄاڻي = شروعات_2;
    }
    /* xxx what about signed chars here... */
    REQUIRE(شروعات_2 <= پڄاڻي, REG_ERANGE);
    لكل (i = شروعات_2; i <= پڄاڻي; i++)
      CHadd(cs, i);
    اكسر;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
ساكن فراغ p_b_cclass(بنية parse *p, cset *cs) {
  ثابت حرف *sp = p->ايندڙ;
  بنية cclass *cp;
  size_t len;
  ثابت حرف *u;
  حرف c;

  طالما (وڌيڪ() && اکر_آهي((uch)PEEK()))
    ايندڙ_3();
  len = p->ايندڙ - sp;
  لكل (cp = cclasses; cp->name != عدم; cp++)
    إذا (لڙي_ڀيٽ(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      اكسر;
  إذا (cp->name == عدم) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    أرجع;
  }

  u = cp->chars;
  طالما ((c = *u++) != '\0')
    CHadd(cs, c);
  لكل (u = cp->multis; *u != '\0'; u += لڙي_ڊيگهه(u) + 1)
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
p_b_نشان(بنية parse *p) {
  حرف قدر_2;

  REQUIRE(وڌيڪ(), REG_EBRACK);
  إذا (!EATTWO('[', '.'))
    أرجع (GETNEXT());

  /* collating symbol */
  قدر_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  أرجع (قدر_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
ساكن حرف                              /* value of collating element */
p_b_coll_elem(بنية parse *p, صحيح endc) /* name ended by endc,']' */
{
  ثابت حرف *sp = p->ايندڙ;
  بنية cname *cp;
  size_t len;

  طالما (وڌيڪ() && !SEETWO(endc, ']'))
    ايندڙ_3();
  إذا (!وڌيڪ()) {
    SETERROR(REG_EBRACK);
    أرجع (0);
  }
  len = p->ايندڙ - sp;
  لكل (cp = cnames; cp->name != عدم; cp++)
    إذا (لڙي_ڀيٽ(cp->name, sp, len) == 0 && لڙي_ڊيگهه(cp->name) == len)
      أرجع (cp->ڪوڊ_2); /* known name */
  إذا (len == 1)
    أرجع (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  أرجع (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
ساكن حرف /* if no counterpart, return ch */
othercase(صحيح ch) {
  ch = (uch)ch;
  أكد(اکر_آهي(ch));
  إذا (isupper(ch))
    أرجع ((uch)tolower(ch));
  وإلا إذا (islower(ch))
    أرجع ((uch)وڏو_اکر(ch));
  وإلا /* peculiar, but could happen */
    أرجع (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
ساكن فراغ bothcases(بنية parse *p, صحيح ch) {
  ثابت حرف *oldnext = p->ايندڙ;
  ثابت حرف *oldend = p->پڇاڙي;
  حرف bracket[3];

  ch = (uch)ch;
  أكد(othercase(ch) != ch); /* p_bracket() would recurse */
  p->ايندڙ = bracket;
  p->پڇاڙي = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  أكد(p->ايندڙ == bracket + 2);
  p->ايندڙ = oldnext;
  p->پڇاڙي = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
ساكن فراغ uncased(بنية parse *p, صحيح ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  إذا (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
ساكن فراغ ordinary(بنية parse *p, صحيح ch) {
  إذا ((p->g->cflags & REG_ICASE) && اکر_آهي((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  وإلا
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
ساكن فراغ nonnewline(بنية parse *p) {
  ثابت حرف *oldnext = p->ايندڙ;
  ثابت حرف *oldend = p->پڇاڙي;
  ساكن ثابت حرف bracket[4] = {'^', '\n', ']', '\0'};

  p->ايندڙ = bracket;
  p->پڇاڙي = bracket + 3;
  p_bracket(p);
  أكد(p->ايندڙ == bracket + 3);
  p->ايندڙ = oldnext;
  p->پڇاڙي = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
ساكن فراغ repeat(بنية parse *p,
                   sopno شروعات_2, /* operand from here to end of strip */
                   صحيح from,    /* repeated from this number */
                   صحيح منزل_2)      /* to this number of times (maybe INFINITY) */
{
  sopno پڄاڻي = هتي();
#وصف_ڪريو N 2
#وصف_ڪريو INF 3
#وصف_ڪريو REP(f, t) ((f) * 8 + (t))
#وصف_ڪريو MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  إذا (p->error != 0) /* head off possible runaway recursion */
    أرجع;

  أكد(from <= منزل_2);

  بدل (REP(MAP(from), MAP(منزل_2))) {
  حالة REP(0, 0):         /* must be user doing this */
    DROP(پڄاڻي - شروعات_2); /* drop the operand */
    اكسر;
  حالة REP(0, 1):   /* as x{1,1}? */
  حالة REP(0, N):   /* as x{1,n}? */
  حالة REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    وجهو_3(OCH_, شروعات_2); /* offset is wrong... */
    repeat(p, شروعات_2 + 1, 1, منزل_2);
    ASTERN(OOR1, شروعات_2);
    AHEAD(شروعات_2); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    اكسر;
  حالة REP(1, 1): /* trivial case */
    /*پورو*/
    اكسر;
  حالة REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    وجهو_3(OCH_, شروعات_2);
    ASTERN(OOR1, شروعات_2);
    AHEAD(شروعات_2);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, شروعات_2 + 1, پڄاڻي + 1);
    أكد(copy == پڄاڻي + 4);
    repeat(p, copy, 1, منزل_2 - 1);
    اكسر;
  حالة REP(1, INF): /* as x+ */
    وجهو_3(OPLUS_, شروعات_2);
    ASTERN(O_PLUS, شروعات_2);
    اكسر;
  حالة REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, شروعات_2, پڄاڻي);
    repeat(p, copy, from - 1, منزل_2 - 1);
    اكسر;
  حالة REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, شروعات_2, پڄاڻي);
    repeat(p, copy, from - 1, منزل_2);
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
  إذا (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->ايندڙ = nuls; /* try to bring things to a halt */
  p->پڇاڙي = nuls;
  أرجع (0); /* make the return value well-defined */
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

  إذا (no >= p->ncsalloc) { /* need another column of space */
    فراغ *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    إذا (nc > SIZE_MAX / حجم(cset))
      اذهب nomem;
    أكد(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((حرف *)p->g->sets, nc * حجم(cset));
    إذا (ptr == عدم)
      اذهب nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((حرف *)p->g->setbits, nbytes);
    إذا (ptr == عدم)
      اذهب nomem;
    p->g->setbits = ptr;

    لكل (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (فراغ)memset((حرف *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  إذا (p->g->sets == عدم || p->g->setbits == عدم)
    اذهب nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->نقاب_2 = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = عدم;

  أرجع (cs);
nomem:
  حرر(p->g->sets);
  p->g->sets = عدم;
  حرر(p->g->setbits);
  p->g->setbits = عدم;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  أرجع (عدم);
}

/*
 - freeset - free a now-unused set
 */
ساكن فراغ freeset(بنية parse *p, cset *cs) {
  size_t i;
  cset *چوٽي_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  لكل (i = 0; i < css; i++)
    CHsub(cs, i);
  إذا (cs == چوٽي_2 - 1) /* recover only the easy case */
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
  cset *چوٽي_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  لكل (cs2 = &p->g->sets[0]; cs2 < چوٽي_2; cs2++)
    إذا (cs2->hash == h && cs2 != cs) {
      /* maybe */
      لكل (i = 0; i < css; i++)
        إذا (!!CHIN(cs2, i) != !!CHIN(cs, i))
          اكسر; /* no */
      إذا (i == css)
        اكسر; /* yes */
    }

  إذا (cs2 < چوٽي_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  أرجع ((صحيح)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
ساكن صحيح /* character; there is no "none" value */
firstch(بنية parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  لكل (i = 0; i < css; i++)
    إذا (CHIN(cs, i))
      أرجع ((حرف)i);
  أكد(never);
  أرجع (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
ساكن صحيح nch(بنية parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  صحيح n = 0;

  لكل (i = 0; i < css; i++)
    إذا (CHIN(cs, i))
      n++;
  أرجع (n);
}

/*
 - mcadd - add a collating element to a cset
 */
ساكن فراغ mcadd(بنية parse *p, cset *cs, ثابت حرف *cp) {
  size_t oldend = cs->smultis;
  فراغ *np;

  cs->smultis += لڙي_ڊيگهه(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  إذا (np == عدم) {
    إذا (cs->multis)
      حرر(cs->multis);
    cs->multis = عدم;
    SETERROR(REG_ESPACE);
    أرجع;
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
  uch *کانو_2;
  صحيح i;
  صحيح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  بنانشان uc = (uch)c;

  لكل (i = 0, کانو_2 = g->setbits; i < ncols; i++, کانو_2 += g->csetsize)
    إذا (کانو_2[uc] != 0)
      أرجع (1);
  أرجع (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
ساكن صحيح /* predicate */
samesets(بنية re_guts *g, صحيح c1, صحيح c2) {
  uch *کانو_2;
  صحيح i;
  صحيح ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  بنانشان uc1 = (uch)c1;
  بنانشان uc2 = (uch)c2;

  لكل (i = 0, کانو_2 = g->setbits; i < ncols; i++, کانو_2 += g->csetsize)
    إذا (کانو_2[uc1] != کانو_2[uc2])
      أرجع (0);
  أرجع (1);
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
  إذا (p->error != 0)
    أرجع;

  لكل (c = CHAR_MIN; c <= CHAR_MAX; c++)
    إذا (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      لكل (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        إذا (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
ساكن sopno                       /* start of duplicate */
dupl(بنية parse *p, sopno شروعات_2, /* from here */
     sopno پڄاڻي)                 /* to this less one */
{
  sopno ret = هتي();
  sopno len = پڄاڻي - شروعات_2;

  أكد(پڄاڻي >= شروعات_2);
  إذا (len == 0)
    أرجع (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  أكد(p->ssize >= p->slen + len);
  (فراغ)memmove((حرف *)(p->strip + p->slen), (حرف *)(p->strip + شروعات_2),
                (size_t)len * حجم(sop));
  p->slen += len;
  أرجع (ret);
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
  إذا (p->error != 0)
    أرجع;

  /* deal with oversize operands ("can't happen", more or less) */
  أكد(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  إذا (p->slen >= p->ssize)
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
  إذا (p->error != 0)
    أرجع;

  sn = هتي();
  EMIT(op, opnd); /* do checks, ensure space */
  أكد(هتي() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  أكد(pos > 0);
  لكل (i = 1; i < NPAREN; i++) {
    إذا (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    إذا (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((حرف *)&p->strip[pos + 1], (حرف *)&p->strip[pos],
          (هتي() - pos - 1) * حجم(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
ساكن فراغ dofwd(بنية parse *p, sopno pos, sop قدر_2) {
  /* avoid making error situations worse */
  إذا (p->error != 0)
    أرجع;

  أكد(قدر_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | قدر_2;
}

/*
 - enlarge - enlarge the strip
 */
ساكن فراغ enlarge(بنية parse *p, sopno سائيز_2) {
  sop *sp;

  إذا (p->ssize >= سائيز_2)
    أرجع;

  إذا ((uintptr_t)سائيز_2 > SIZE_MAX / حجم(sop)) {
    SETERROR(REG_ESPACE);
    أرجع;
  }

  sp = (sop *)realloc(p->strip, سائيز_2 * حجم(sop));
  إذا (sp == عدم) {
    SETERROR(REG_ESPACE);
    أرجع;
  }
  p->strip = sp;
  p->ssize = سائيز_2;
}

/*
 - stripsnug - compact the strip
 */
ساكن فراغ stripsnug(بنية parse *p, بنية re_guts *g) {
  g->nstates = p->slen;
  إذا ((uintptr_t)p->slen > SIZE_MAX / حجم(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    أرجع;
  }

  g->strip = (sop *)realloc((حرف *)p->strip, p->slen * حجم(sop));
  إذا (g->strip == عدم) {
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
  sop *جاچ;
  sop *شروعات_2 = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  حرف *cp;
  sopno i;

  /* avoid making error situations worse */
  إذا (p->error != 0)
    أرجع;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  جاچ = g->strip + 1;
  افعل {
    s = *جاچ++;
    بدل (OP(s)) {
    حالة OCHAR:        /* sequence member */
      إذا (newlen == 0) /* new sequence */
        newstart = جاچ - 1;
      newlen++;
      اكسر;
    حالة OPLUS_: /* things that don't break one */
    حالة OLPAREN:
    حالة ORPAREN:
      اكسر;
    حالة OQUEST_: /* things that must be skipped */
    حالة OCH_:
      جاچ--;
      افعل {
        جاچ += OPND(s);
        s = *جاچ;
        /* assert() interferes w debug printouts */
        إذا (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          أرجع;
        }
      } طالما (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    افتراضي:                  /* things that break a sequence */
      إذا (newlen > g->mlen) { /* ends one */
        شروعات_2 = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      اكسر;
    }
  } طالما (OP(s) != OEND);

  إذا (g->mlen == 0) /* there isn't one */
    أرجع;

  /* turn it into a character string */
  g->must = يادگيري_ڏيو((size_t)g->mlen + 1);
  إذا (g->must == عدم) { /* argh; just forget it */
    g->mlen = 0;
    أرجع;
  }
  cp = g->must;
  جاچ = شروعات_2;
  لكل (i = g->mlen; i > 0; i--) {
    طالما (OP(s = *جاچ++) != OCHAR)
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
  sop *جاچ;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  إذا (p->error != 0)
    أرجع (0); /* there may not be an OEND */

  جاچ = g->strip + 1;
  افعل {
    s = *جاچ++;
    بدل (OP(s)) {
    حالة OPLUS_:
      plusnest++;
      اكسر;
    حالة O_PLUS:
      إذا (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      اكسر;
    }
  } طالما (OP(s) != OEND);
  إذا (plusnest != 0)
    g->iflags |= REGEX_BAD;
  أرجع (maxnest);
}
