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

#અન્તર્ભાવ "regex_impl.h"
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ "regex2.h"
#અન્તર્ભાવ "regutils.h"

#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"

/* character-class table */
સ્થિર સંરચના cclass {
  અચલ અક્ષર *name;
  અચલ અક્ષર *chars;
  અચલ અક્ષર *multis;
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
    {ખાલી, 0, ""}};

/* character-name table */
સ્થિર સંરચના cname {
  અચલ અક્ષર *name;
  અક્ષર code;
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
              {ખાલી, 0}};

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
સંરચના parse {
  અચલ અક્ષર *next; /* next character in RE */
  અચલ અક્ષર *end;  /* end of string (-> NUL normally) */
  પૂર્ણાંક error;        /* has an error been seen? */
  sop *strip;       /* malloced strip */
  sopno ssize;      /* malloced strip size (allocated) */
  sopno slen;       /* malloced strip length (used) */
  પૂર્ણાંક ncsalloc;     /* number of csets allocated */
  સંરચના re_guts *g;
#define NPAREN 21       /* we need to remember () 1-20 for back refs */
  sopno pbegin[NPAREN]; /* -> ( ([0] unused) */
  sopno pend[NPAREN];   /* -> ) ([0] unused) */
};

સ્થિર શૂન્ય p_ere(સંરચના parse *, પૂર્ણાંક);
સ્થિર શૂન્ય p_ere_exp(સંરચના parse *);
સ્થિર શૂન્ય p_str(સંરચના parse *);
સ્થિર શૂન્ય p_bre(સંરચના parse *, પૂર્ણાંક, પૂર્ણાંક);
સ્થિર પૂર્ણાંક p_simp_re(સંરચના parse *, પૂર્ણાંક);
સ્થિર પૂર્ણાંક p_ગણતરી(સંરચના parse *);
સ્થિર શૂન્ય p_bracket(સંરચના parse *);
સ્થિર શૂન્ય p_b_term(સંરચના parse *, cset *);
સ્થિર શૂન્ય p_b_cclass(સંરચના parse *, cset *);
સ્થિર શૂન્ય p_b_eclass(સંરચના parse *, cset *);
સ્થિર અક્ષર p_b_પ્રતીક(સંરચના parse *);
સ્થિર અક્ષર p_b_coll_elem(સંરચના parse *, પૂર્ણાંક);
સ્થિર અક્ષર othercase(પૂર્ણાંક);
સ્થિર શૂન્ય bothcases(સંરચના parse *, પૂર્ણાંક);
સ્થિર શૂન્ય uncased(સંરચના parse *, પૂર્ણાંક);
સ્થિર શૂન્ય ordinary(સંરચના parse *, પૂર્ણાંક);
સ્થિર શૂન્ય nonnewline(સંરચના parse *);
સ્થિર શૂન્ય repeat(સંરચના parse *, sopno, પૂર્ણાંક, પૂર્ણાંક);
સ્થિર પૂર્ણાંક seterr(સંરચના parse *, પૂર્ણાંક);
સ્થિર cset *allocset(સંરચના parse *);
સ્થિર શૂન્ય freeset(સંરચના parse *, cset *);
સ્થિર પૂર્ણાંક freezeset(સંરચના parse *, cset *);
સ્થિર પૂર્ણાંક firstch(સંરચના parse *, cset *);
સ્થિર પૂર્ણાંક nch(સંરચના parse *, cset *);
સ્થિર શૂન્ય mcadd(સંરચના parse *, cset *, અચલ અક્ષર *);
સ્થિર શૂન્ય mcinvert(સંરચના parse *, cset *);
સ્થિર શૂન્ય mccase(સંરચના parse *, cset *);
સ્થિર પૂર્ણાંક isinsets(સંરચના re_guts *, પૂર્ણાંક);
સ્થિર પૂર્ણાંક samesets(સંરચના re_guts *, પૂર્ણાંક, પૂર્ણાંક);
સ્થિર શૂન્ય categorize(સંરચના parse *, સંરચના re_guts *);
સ્થિર sopno dupl(સંરચના parse *, sopno, sopno);
સ્થિર શૂન્ય doemit(સંરચના parse *, sop, size_t);
સ્થિર શૂન્ય doinsert(સંરચના parse *, sop, size_t, sopno);
સ્થિર શૂન્ય dofwd(સંરચના parse *, sopno, sop);
સ્થિર શૂન્ય enlarge(સંરચના parse *, sopno);
સ્થિર શૂન્ય stripsnug(સંરચના parse *, સંરચના re_guts *);
સ્થિર શૂન્ય findmust(સંરચના parse *, સંરચના re_guts *);
સ્થિર sopno pluscount(સંરચના parse *, સંરચના re_guts *);

સ્થિર અક્ષર nuls[10]; /* place to point scanner in event of error */

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
#define REQUIRE(co, e) (શૂન્ય)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define ઉમેરો_3(op, pos) doinsert(p, (sop)(op), HERE() - (pos) + 1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE() - (pos))
#define ASTERN(sop, pos) EMIT(sop, HERE() - pos)
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))

#ifdef _POSIX2_RE_DUP_MAX
#define DUPMAX _POSIX2_RE_DUP_MAX
#નહીંતર
#define DUPMAX 255
#endif
#define REGINFINITY (DUPMAX + 1)

#ifndef NDEBUG
સ્થિર પૂર્ણાંક never = 0; /* for use in asserts; shuts lint up */
#નહીંતર
#define never 0 /* some <assert.h>s have bugs too */
#endif

/*
 - llvm_regcomp - interface for parser and compilation
 */
પૂર્ણાંક /* 0 success, otherwise REG_something */
llvm_regcomp(llvm_regex_t *preg, અચલ અક્ષર *pattern, પૂર્ણાંક cflags) {
  સંરચના parse pa;
  સંરચના re_guts *g;
  સંરચના parse *p = &pa;
  પૂર્ણાંક i;
  size_t len;
#ifdef REDEBUG
#define GOODFLAGS(f) (f)
#નહીંતર
#define GOODFLAGS(f) ((f) & ~REG_DUMP)
#endif

  cflags = GOODFLAGS(cflags);
  જો ((cflags & REG_EXTENDED) && (cflags & REG_NOSPEC))
    પાછા (REG_INVARG);

  જો (cflags & REG_PEND) {
    જો (preg->re_endp < pattern)
      પાછા (REG_INVARG);
    len = preg->re_endp - pattern;
  } નહીંતર {
    len = strlen(pattern);
  }

  /* do the mallocs early so failure handling is easy */
  g = (સંરચના re_guts *)malloc(કદ(સંરચના re_guts) +
                               (NC - 1) * કદ(cat_t));
  જો (g == ખાલી)
    પાછા (REG_ESPACE);
  p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; /* ugh */
  p->strip = (sop *)calloc(p->ssize, કદ(sop));
  p->slen = 0;
  જો (p->strip == ખાલી) {
    મુક્ત((અક્ષર *)g);
    પાછા (REG_ESPACE);
  }

  /* set things up */
  p->g = g;
  p->next = pattern;
  p->end = p->next + len;
  p->error = 0;
  p->ncsalloc = 0;
  માટે (i = 0; i < NPAREN; i++) {
    p->pbegin[i] = 0;
    p->pend[i] = 0;
  }
  g->csetsize = NC;
  g->sets = ખાલી;
  g->setbits = ખાલી;
  g->ncsets = 0;
  g->cflags = cflags;
  g->iflags = 0;
  g->nbol = 0;
  g->neol = 0;
  g->must = ખાલી;
  g->mlen = 0;
  g->nsub = 0;
  g->ncategories = 1; /* category 0 is "everything else" */
  g->categories = &g->catspace[-(CHAR_MIN)];
  (શૂન્ય)memset((અક્ષર *)g->catspace, 0, NC * કદ(cat_t));
  g->backrefs = 0;

  /* do it */
  EMIT(OEND, 0);
  g->firststate = THERE();
  જો (cflags & REG_EXTENDED)
    p_ere(p, OUT);
  નહીંતર જો (cflags & REG_NOSPEC)
    p_str(p);
  નહીંતર
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
  જો (g->iflags & REGEX_BAD)
    SETERROR(REG_ASSERT);
#endif

  /* win or lose, we're done */
  જો (p->error != 0) /* lose */
    llvm_regfree(preg);
  પાછા (p->error);
}

/*
 - p_ere - ERE parser top level, concatenation and alternation
 */
સ્થિર શૂન્ય p_ere(સંરચના parse *p,
                  પૂર્ણાંક stop) /* character this ERE should end at */
{
  અક્ષર c;
  sopno prevback = 0;
  sopno prevfwd = 0;
  sopno conc;
  પૂર્ણાંક first = 1; /* is this the first alternative? */

  માટે (;;) {
    /* do a bunch of concatenated expressions */
    conc = HERE();
    જ્યારે (MORE() && (c = PEEK()) != '|' && c != stop)
      p_ere_exp(p);
    REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

    જો (!EAT('|'))
      તોડો; /* NOTE BREAK OUT */

    જો (first) {
      ઉમેરો_3(OCH_, conc); /* offset is wrong */
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

  જો (!first) { /* tail-end fixups */
    AHEAD(prevfwd);
    ASTERN(O_CH, prevback);
  }

  પ્રતિજ્ઞા(!MORE() || SEE(stop));
}

/*
 - p_ere_exp - parse one subERE, an atom possibly followed by a repetition op
 */
સ્થિર શૂન્ય p_ere_exp(સંરચના parse *p) {
  અક્ષર c;
  sopno pos;
  પૂર્ણાંક ગણતરી;
  પૂર્ણાંક count2;
  પૂર્ણાંક backrefnum;
  sopno subno;
  પૂર્ણાંક wascaret = 0;

  પ્રતિજ્ઞા(MORE()); /* caller should have ensured this */
  c = GETNEXT();

  pos = HERE();
  પસંદગી (c) {
  સ્થિતિ '(':
    REQUIRE(MORE(), REG_EPAREN);
    p->g->nsub++;
    subno = p->g->nsub;
    જો (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    જો (!SEE(')'))
      p_ere(p, ')');
    જો (subno < NPAREN) {
      p->pend[subno] = HERE();
      પ્રતિજ્ઞા(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    MUSTEAT(')', REG_EPAREN);
    તોડો;
#ifndef POSIX_MISTAKE
  સ્થિતિ ')': /* happens only if no current unmatched ( */
    /*
     * You may ask, why the ifndef?  Because I didn't notice
     * this until slightly too late for 1003.2, and none of the
     * other 1003.2 regular-expression reviewers noticed it at
     * all.  So an unmatched ) is legal POSIX, at least until
     * we can get it fixed.
     */
    SETERROR(REG_EPAREN);
    તોડો;
#endif
  સ્થિતિ '^':
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
    wascaret = 1;
    તોડો;
  સ્થિતિ '$':
    EMIT(OEOL, 0);
    p->g->iflags |= USEEOL;
    p->g->neol++;
    તોડો;
  સ્થિતિ '|':
    SETERROR(REG_EMPTY);
    તોડો;
  સ્થિતિ '*':
  સ્થિતિ '+':
  સ્થિતિ '?':
    SETERROR(REG_BADRPT);
    તોડો;
  સ્થિતિ '.':
    જો (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    નહીંતર
      EMIT(OANY, 0);
    તોડો;
  સ્થિતિ '[':
    p_bracket(p);
    તોડો;
  સ્થિતિ '\\':
    REQUIRE(MORE(), REG_EESCAPE);
    c = GETNEXT();
    જો (c >= '1' && c <= '9') {
      /* \[0-9] is taken to be a back-reference to a previously specified
       * matching group. backrefnum will hold the number. The matching
       * group must exist (i.e. if \4 is found there must have been at
       * least 4 matching groups specified in the pattern previously).
       */
      backrefnum = c - '0';
    } નહીંતર જો (c == 'g') {
      /* Support back-references with index greater 9.
       * These look like that: \g{n}
       * Extract the number inside the brackets.
       */
      MUSTEAT('{', REG_BADRPT);

      backrefnum = 0;
      જ્યારે (MORE() && isdigit(PEEK())) {
        c = GETNEXT();
        backrefnum = backrefnum * 10 + c - '0';
      }
      MUSTEAT('}', REG_BADRPT);
    } નહીંતર જો (c == 'n') {
      ordinary(p, '\n');
      તોડો;
    } નહીંતર જો (c == 't') {
      ordinary(p, '\t');
      તોડો;
    } નહીંતર જો (c == 'x') {
      /* Support \xAA hexadecimal escape sequences. \x must be followed by
       * exactly two hex digits, otherwise it is interpreted literally.
       */
      અક્ષર hexstr[3] = {0};
      અક્ષર *hexp;
      પૂર્ણાંક val;
      જો (MORE2()) {
        hexstr[0] = PEEK();
        hexstr[1] = PEEK2();
        val = strtol(hexstr, &hexp, 16);
        જો (*hexp == '\0') {
          NEXT2();
          uncased(p, (અક્ષર)val);
          તોડો;
        }
      }
      ordinary(p, 'x');
      તોડો;
    } નહીંતર {
      /* Other chars are simply themselves when escaped with a backslash.
       */
      ordinary(p, c);
      તોડો;
    }

    જો (backrefnum >= NPAREN) {
      SETERROR(REG_ESUBREG);
      તોડો;
    }

    જો (p->pend[backrefnum] == 0) {
      SETERROR(REG_ESUBREG);
      તોડો;
    }

    /* Make sure everything checks out and emit the sequence
     * that marks a back-reference to the parse structure.
     */
    પ્રતિજ્ઞા(backrefnum <= p->g->nsub);
    EMIT(OBACK_, backrefnum);
    પ્રતિજ્ઞા(p->pbegin[backrefnum] != 0);
    પ્રતિજ્ઞા(OP(p->strip[p->pbegin[backrefnum]]) == OLPAREN);
    પ્રતિજ્ઞા(OP(p->strip[p->pend[backrefnum]]) == ORPAREN);
    (શૂન્ય)dupl(p, p->pbegin[backrefnum] + 1, p->pend[backrefnum]);
    EMIT(O_BACK, backrefnum);
    p->g->backrefs = 1;
    તોડો;
  સ્થિતિ '{': /* okay as ordinary except if digit follows */
    REQUIRE(!MORE() || !isdigit((uch)PEEK()), REG_BADRPT);
    LLVM_FALLTHROUGH;
  મૂળભૂત:
    ordinary(p, c);
    તોડો;
  }

  જો (!MORE())
    પાછા;
  c = PEEK();
  /* we call { a repetition if followed by a digit */
  જો (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    પાછા; /* no repetition, we're done */
  NEXT();

  REQUIRE(!wascaret, REG_BADRPT);
  પસંદગી (c) {
  સ્થિતિ '*': /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ઉમેરો_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ઉમેરો_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
    તોડો;
  સ્થિતિ '+':
    ઉમેરો_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    તોડો;
  સ્થિતિ '?':
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ઉમેરો_3(OCH_, pos); /* offset slightly wrong */
    ASTERN(OOR1, pos); /* this one's right */
    AHEAD(pos);        /* fix the OCH_ */
    EMIT(OOR2, 0);     /* offset very wrong... */
    AHEAD(THERE());    /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    તોડો;
  સ્થિતિ '{':
    ગણતરી = p_ગણતરી(p);
    જો (EAT(',')) {
      જો (isdigit((uch)PEEK())) {
        count2 = p_ગણતરી(p);
        REQUIRE(ગણતરી <= count2, REG_BADBR);
      } નહીંતર /* single number with comma */
        count2 = REGINFINITY;
    } નહીંતર /* just a single number */
      count2 = ગણતરી;
    repeat(p, pos, ગણતરી, count2);
    જો (!EAT('}')) { /* error heuristics */
      જ્યારે (MORE() && PEEK() != '}')
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
    તોડો;
  }

  જો (!MORE())
    પાછા;
  c = PEEK();
  જો (!(c == '*' || c == '+' || c == '?' ||
        (c == '{' && MORE2() && isdigit((uch)PEEK2()))))
    પાછા;
  SETERROR(REG_BADRPT);
}

/*
 - p_str - string (no metacharacters) "parser"
 */
સ્થિર શૂન્ય p_str(સંરચના parse *p) {
  REQUIRE(MORE(), REG_EMPTY);
  જ્યારે (MORE())
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
સ્થિર શૂન્ય p_bre(સંરચના parse *p, પૂર્ણાંક end1, /* first terminating character */
                  પૂર્ણાંક end2)                  /* second terminating character */
{
  sopno start = HERE();
  પૂર્ણાંક first = 1; /* first subexpression? */
  પૂર્ણાંક wasdollar = 0;

  જો (EAT('^')) {
    EMIT(OBOL, 0);
    p->g->iflags |= USEBOL;
    p->g->nbol++;
  }
  જ્યારે (MORE() && !SEETWO(end1, end2)) {
    wasdollar = p_simp_re(p, first);
    first = 0;
  }
  જો (wasdollar) { /* oops, that was a trailing anchor */
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
સ્થિર પૂર્ણાંક /* was the simple RE an unbackslashed $? */
p_simp_re(સંરચના parse *p,
          પૂર્ણાંક starordinary) /* is a leading * an ordinary character? */
{
  પૂર્ણાંક c;
  પૂર્ણાંક ગણતરી;
  પૂર્ણાંક count2;
  sopno pos;
  પૂર્ણાંક i;
  sopno subno;
#define BACKSL (1 << CHAR_BIT)

  pos = HERE(); /* repetition op, if any, covers from here */

  પ્રતિજ્ઞા(MORE()); /* caller should have ensured this */
  c = GETNEXT();
  જો (c == '\\') {
    REQUIRE(MORE(), REG_EESCAPE);
    c = BACKSL | GETNEXT();
  }
  પસંદગી (c) {
  સ્થિતિ '.':
    જો (p->g->cflags & REG_NEWLINE)
      nonnewline(p);
    નહીંતર
      EMIT(OANY, 0);
    તોડો;
  સ્થિતિ '[':
    p_bracket(p);
    તોડો;
  સ્થિતિ BACKSL | '{':
    SETERROR(REG_BADRPT);
    તોડો;
  સ્થિતિ BACKSL | '(':
    p->g->nsub++;
    subno = p->g->nsub;
    જો (subno < NPAREN)
      p->pbegin[subno] = HERE();
    EMIT(OLPAREN, subno);
    /* the MORE here is an error heuristic */
    જો (MORE() && !SEETWO('\\', ')'))
      p_bre(p, '\\', ')');
    જો (subno < NPAREN) {
      p->pend[subno] = HERE();
      પ્રતિજ્ઞા(p->pend[subno] != 0);
    }
    EMIT(ORPAREN, subno);
    REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
    તોડો;
  સ્થિતિ BACKSL | ')': /* should not get here -- must be user */
  સ્થિતિ BACKSL | '}':
    SETERROR(REG_EPAREN);
    તોડો;
  સ્થિતિ BACKSL | '1':
  સ્થિતિ BACKSL | '2':
  સ્થિતિ BACKSL | '3':
  સ્થિતિ BACKSL | '4':
  સ્થિતિ BACKSL | '5':
  સ્થિતિ BACKSL | '6':
  સ્થિતિ BACKSL | '7':
  સ્થિતિ BACKSL | '8':
  સ્થિતિ BACKSL | '9':
    i = (c & ~BACKSL) - '0';
    પ્રતિજ્ઞા(i < NPAREN);
    જો (p->pend[i] != 0) {
      પ્રતિજ્ઞા(i <= p->g->nsub);
      EMIT(OBACK_, i);
      પ્રતિજ્ઞા(p->pbegin[i] != 0);
      પ્રતિજ્ઞા(OP(p->strip[p->pbegin[i]]) == OLPAREN);
      પ્રતિજ્ઞા(OP(p->strip[p->pend[i]]) == ORPAREN);
      (શૂન્ય)dupl(p, p->pbegin[i] + 1, p->pend[i]);
      EMIT(O_BACK, i);
    } નહીંતર {
      SETERROR(REG_ESUBREG);
    }
    p->g->backrefs = 1;
    તોડો;
  સ્થિતિ '*':
    REQUIRE(starordinary, REG_BADRPT);
    LLVM_FALLTHROUGH;
  મૂળભૂત:
    ordinary(p, (અક્ષર)c);
    તોડો;
  }

  જો (EAT('*')) { /* implemented as +? */
    /* this case does not require the (y|) trick, noKLUDGE */
    ઉમેરો_3(OPLUS_, pos);
    ASTERN(O_PLUS, pos);
    ઉમેરો_3(OQUEST_, pos);
    ASTERN(O_QUEST, pos);
  } નહીંતર જો (EATTWO('\\', '{')) {
    ગણતરી = p_ગણતરી(p);
    જો (EAT(',')) {
      જો (MORE() && isdigit((uch)PEEK())) {
        count2 = p_ગણતરી(p);
        REQUIRE(ગણતરી <= count2, REG_BADBR);
      } નહીંતર /* single number with comma */
        count2 = REGINFINITY;
    } નહીંતર /* just a single number */
      count2 = ગણતરી;
    repeat(p, pos, ગણતરી, count2);
    જો (!EATTWO('\\', '}')) { /* error heuristics */
      જ્યારે (MORE() && !SEETWO('\\', '}'))
        NEXT();
      REQUIRE(MORE(), REG_EBRACE);
      SETERROR(REG_BADBR);
    }
  } નહીંતર જો (c == '$') /* $ (but not \$) ends it */
    પાછા (1);

  પાછા (0);
}

/*
 - p_count - parse a repetition count
 */
સ્થિર પૂર્ણાંક /* the value */
p_ગણતરી(સંરચના parse *p) {
  પૂર્ણાંક ગણતરી = 0;
  પૂર્ણાંક ndigits = 0;

  જ્યારે (MORE() && isdigit((uch)PEEK()) && ગણતરી <= DUPMAX) {
    ગણતરી = ગણતરી * 10 + (GETNEXT() - '0');
    ndigits++;
  }

  REQUIRE(ndigits > 0 && ગણતરી <= DUPMAX, REG_BADBR);
  પાછા (ગણતરી);
}

/*
 - p_bracket - parse a bracketed character list
 *
 * Note a significant property of this code:  if the allocset() did SETERROR,
 * no set operations are done.
 */
સ્થિર શૂન્ય p_bracket(સંરચના parse *p) {
  cset *cs;
  પૂર્ણાંક invert = 0;

  /* Dept of Truly Sickening Special-Case Kludges */
  જો (p->end - p->next > 5) {
    જો (strncmp(p->next, "[:<:]]", 6) == 0) {
      EMIT(OBOW, 0);
      NEXTn(6);
      પાછા;
    }
    જો (strncmp(p->next, "[:>:]]", 6) == 0) {
      EMIT(OEOW, 0);
      NEXTn(6);
      પાછા;
    }
  }

  જો ((cs = allocset(p)) == ખાલી) {
    /* allocset did set error status in p */
    પાછા;
  }

  જો (EAT('^'))
    invert++; /* make note to invert set at end */
  જો (EAT(']'))
    CHadd(cs, ']');
  નહીંતર જો (EAT('-'))
    CHadd(cs, '-');
  જ્યારે (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
    p_b_term(p, cs);
  જો (EAT('-'))
    CHadd(cs, '-');
  MUSTEAT(']', REG_EBRACK);

  જો (p->error != 0) { /* don't mess things up further */
    freeset(p, cs);
    પાછા;
  }

  જો (p->g->cflags & REG_ICASE) {
    પૂર્ણાંક i;
    પૂર્ણાંક ci;

    માટે (i = p->g->csetsize - 1; i >= 0; i--)
      જો (CHIN(cs, i) && isalpha(i)) {
        ci = othercase(i);
        જો (ci != i)
          CHadd(cs, ci);
      }
    જો (cs->multis != ખાલી)
      mccase(p, cs);
  }
  જો (invert) {
    પૂર્ણાંક i;

    માટે (i = p->g->csetsize - 1; i >= 0; i--)
      જો (CHIN(cs, i))
        CHsub(cs, i);
      નહીંતર
        CHadd(cs, i);
    જો (p->g->cflags & REG_NEWLINE)
      CHsub(cs, '\n');
    જો (cs->multis != ખાલી)
      mcinvert(p, cs);
  }

  પ્રતિજ્ઞા(cs->multis == ખાલી); /* xxx */

  જો (nch(p, cs) == 1) { /* optimize singleton sets */
    ordinary(p, firstch(p, cs));
    freeset(p, cs);
  } નહીંતર {
    EMIT(OANYOF, freezeset(p, cs));
  }
}

/*
 - p_b_term - parse one term of a bracketed character list
 */
સ્થિર શૂન્ય p_b_term(સંરચના parse *p, cset *cs) {
  અક્ષર c;
  અક્ષર start, finish;
  પૂર્ણાંક i;

  /* classify what we've got */
  પસંદગી ((MORE()) ? PEEK() : '\0') {
  સ્થિતિ '[':
    c = (MORE2()) ? PEEK2() : '\0';
    તોડો;
  સ્થિતિ '-':
    SETERROR(REG_ERANGE);
    પાછા; /* NOTE RETURN */
    તોડો;
  મૂળભૂત:
    c = '\0';
    તોડો;
  }

  પસંદગી (c) {
  સ્થિતિ ':': /* character class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECTYPE);
    p_b_cclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
    તોડો;
  સ્થિતિ '=': /* equivalence class */
    NEXT2();
    REQUIRE(MORE(), REG_EBRACK);
    c = PEEK();
    REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
    p_b_eclass(p, cs);
    REQUIRE(MORE(), REG_EBRACK);
    REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
    તોડો;
  મૂળભૂત: /* symbol, ordinary character, or range */
           /* xxx revision needed for multichar stuff */
    start = p_b_પ્રતીક(p);
    જો (SEE('-') && MORE2() && PEEK2() != ']') {
      /* range */
      NEXT();
      જો (EAT('-'))
        finish = '-';
      નહીંતર
        finish = p_b_પ્રતીક(p);
    } નહીંતર {
      finish = start;
    }
    /* xxx what about signed chars here... */
    REQUIRE(start <= finish, REG_ERANGE);
    માટે (i = start; i <= finish; i++)
      CHadd(cs, i);
    તોડો;
  }
}

/*
 - p_b_cclass - parse a character-class name and deal with it
 */
સ્થિર શૂન્ય p_b_cclass(સંરચના parse *p, cset *cs) {
  અચલ અક્ષર *sp = p->next;
  સંરચના cclass *cp;
  size_t len;
  અચલ અક્ષર *u;
  અક્ષર c;

  જ્યારે (MORE() && isalpha((uch)PEEK()))
    NEXT();
  len = p->next - sp;
  માટે (cp = cclasses; cp->name != ખાલી; cp++)
    જો (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
      તોડો;
  જો (cp->name == ખાલી) {
    /* oops, didn't find it */
    SETERROR(REG_ECTYPE);
    પાછા;
  }

  u = cp->chars;
  જ્યારે ((c = *u++) != '\0')
    CHadd(cs, c);
  માટે (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
    MCadd(p, cs, u);
}

/*
 - p_b_eclass - parse an equivalence-class name and deal with it
 *
 * This implementation is incomplete. xxx
 */
સ્થિર શૂન્ય p_b_eclass(સંરચના parse *p, cset *cs) {
  અક્ષર c;

  c = p_b_coll_elem(p, '=');
  CHadd(cs, c);
}

/*
 - p_b_symbol - parse a character or [..]ed multicharacter collating symbol
 */
સ્થિર અક્ષર /* value of symbol */
p_b_પ્રતીક(સંરચના parse *p) {
  અક્ષર કિંમત_2;

  REQUIRE(MORE(), REG_EBRACK);
  જો (!EATTWO('[', '.'))
    પાછા (GETNEXT());

  /* collating symbol */
  કિંમત_2 = p_b_coll_elem(p, '.');
  REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
  પાછા (કિંમત_2);
}

/*
 - p_b_coll_elem - parse a collating-element name and look it up
 */
સ્થિર અક્ષર                              /* value of collating element */
p_b_coll_elem(સંરચના parse *p, પૂર્ણાંક endc) /* name ended by endc,']' */
{
  અચલ અક્ષર *sp = p->next;
  સંરચના cname *cp;
  size_t len;

  જ્યારે (MORE() && !SEETWO(endc, ']'))
    NEXT();
  જો (!MORE()) {
    SETERROR(REG_EBRACK);
    પાછા (0);
  }
  len = p->next - sp;
  માટે (cp = cnames; cp->name != ખાલી; cp++)
    જો (strncmp(cp->name, sp, len) == 0 && strlen(cp->name) == len)
      પાછા (cp->code); /* known name */
  જો (len == 1)
    પાછા (*sp);         /* single character */
  SETERROR(REG_ECOLLATE); /* neither */
  પાછા (0);
}

/*
 - othercase - return the case counterpart of an alphabetic
 */
સ્થિર અક્ષર /* if no counterpart, return ch */
othercase(પૂર્ણાંક ch) {
  ch = (uch)ch;
  પ્રતિજ્ઞા(isalpha(ch));
  જો (isupper(ch))
    પાછા ((uch)tolower(ch));
  નહીંતર જો (islower(ch))
    પાછા ((uch)toupper(ch));
  નહીંતર /* peculiar, but could happen */
    પાછા (ch);
}

/*
 - bothcases - emit a dualcase version of a two-case character
 *
 * Boy, is this implementation ever a kludge...
 */
સ્થિર શૂન્ય bothcases(સંરચના parse *p, પૂર્ણાંક ch) {
  અચલ અક્ષર *oldnext = p->next;
  અચલ અક્ષર *oldend = p->end;
  અક્ષર bracket[3];

  ch = (uch)ch;
  પ્રતિજ્ઞા(othercase(ch) != ch); /* p_bracket() would recurse */
  p->next = bracket;
  p->end = bracket + 2;
  bracket[0] = ch;
  bracket[1] = ']';
  bracket[2] = '\0';
  p_bracket(p);
  પ્રતિજ્ઞા(p->next == bracket + 2);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - uncased - emit an ordinary character that is never treated as cased
 */
સ્થિર શૂન્ય uncased(સંરચના parse *p, પૂર્ણાંક ch) {
  cat_t *cap = p->g->categories;

  EMIT(OCHAR, (uch)ch);
  જો (cap[ch] == 0)
    cap[ch] = p->g->ncategories++;
}

/*
 - ordinary - emit an ordinary character
 */
સ્થિર શૂન્ય ordinary(સંરચના parse *p, પૂર્ણાંક ch) {
  જો ((p->g->cflags & REG_ICASE) && isalpha((uch)ch) && othercase(ch) != ch)
    bothcases(p, ch);
  નહીંતર
    uncased(p, ch);
}

/*
 - nonnewline - emit REG_NEWLINE version of OANY
 *
 * Boy, is this implementation ever a kludge...
 */
સ્થિર શૂન્ય nonnewline(સંરચના parse *p) {
  અચલ અક્ષર *oldnext = p->next;
  અચલ અક્ષર *oldend = p->end;
  સ્થિર અચલ અક્ષર bracket[4] = {'^', '\n', ']', '\0'};

  p->next = bracket;
  p->end = bracket + 3;
  p_bracket(p);
  પ્રતિજ્ઞા(p->next == bracket + 3);
  p->next = oldnext;
  p->end = oldend;
}

/*
 - repeat - generate code for a bounded repetition, recursively if needed
 */
સ્થિર શૂન્ય repeat(સંરચના parse *p,
                   sopno start, /* operand from here to end of strip */
                   પૂર્ણાંક from,    /* repeated from this number */
                   પૂર્ણાંક to)      /* to this number of times (maybe INFINITY) */
{
  sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f) * 8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == REGINFINITY) ? INF : N)
  sopno copy;

  જો (p->error != 0) /* head off possible runaway recursion */
    પાછા;

  પ્રતિજ્ઞા(from <= to);

  પસંદગી (REP(MAP(from), MAP(to))) {
  સ્થિતિ REP(0, 0):         /* must be user doing this */
    DROP(finish - start); /* drop the operand */
    તોડો;
  સ્થિતિ REP(0, 1):   /* as x{1,1}? */
  સ્થિતિ REP(0, N):   /* as x{1,n}? */
  સ્થિતિ REP(0, INF): /* as x{1,}? */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ઉમેરો_3(OCH_, start); /* offset is wrong... */
    repeat(p, start + 1, 1, to);
    ASTERN(OOR1, start);
    AHEAD(start); /* ... fix it */
    EMIT(OOR2, 0);
    AHEAD(THERE());
    ASTERN(O_CH, THERETHERE());
    તોડો;
  સ્થિતિ REP(1, 1): /* trivial case */
    /* done */
    તોડો;
  સ્થિતિ REP(1, N): /* as x?x{1,n-1} */
    /* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
    ઉમેરો_3(OCH_, start);
    ASTERN(OOR1, start);
    AHEAD(start);
    EMIT(OOR2, 0);  /* offset very wrong... */
    AHEAD(THERE()); /* ...so fix it */
    ASTERN(O_CH, THERETHERE());
    copy = dupl(p, start + 1, finish + 1);
    પ્રતિજ્ઞા(copy == finish + 4);
    repeat(p, copy, 1, to - 1);
    તોડો;
  સ્થિતિ REP(1, INF): /* as x+ */
    ઉમેરો_3(OPLUS_, start);
    ASTERN(O_PLUS, start);
    તોડો;
  સ્થિતિ REP(N, N): /* as xx{m-1,n-1} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to - 1);
    તોડો;
  સ્થિતિ REP(N, INF): /* as xx{n-1,INF} */
    copy = dupl(p, start, finish);
    repeat(p, copy, from - 1, to);
    તોડો;
  મૂળભૂત:                /* "can't happen" */
    SETERROR(REG_ASSERT); /* just in case */
    તોડો;
  }
}

/*
 - seterr - set an error condition
 */
સ્થિર પૂર્ણાંક /* useless but makes type checking happy */
seterr(સંરચના parse *p, પૂર્ણાંક e) {
  જો (p->error == 0) /* keep earliest error condition */
    p->error = e;
  p->next = nuls; /* try to bring things to a halt */
  p->end = nuls;
  પાછા (0); /* make the return value well-defined */
}

/*
 - allocset - allocate a set of characters for []
 */
સ્થિર cset *allocset(સંરચના parse *p) {
  પૂર્ણાંક no = p->g->ncsets++;
  size_t nc;
  size_t nbytes;
  cset *cs;
  size_t css = (size_t)p->g->csetsize;
  પૂર્ણાંક i;

  જો (no >= p->ncsalloc) { /* need another column of space */
    શૂન્ય *ptr;

    p->ncsalloc += CHAR_BIT;
    nc = p->ncsalloc;
    જો (nc > SIZE_MAX / કદ(cset))
      જાઓ nomem;
    પ્રતિજ્ઞા(nc % CHAR_BIT == 0);
    nbytes = nc / CHAR_BIT * css;

    ptr = (cset *)realloc((અક્ષર *)p->g->sets, nc * કદ(cset));
    જો (ptr == ખાલી)
      જાઓ nomem;
    p->g->sets = ptr;

    ptr = (uch *)realloc((અક્ષર *)p->g->setbits, nbytes);
    જો (ptr == ખાલી)
      જાઓ nomem;
    p->g->setbits = ptr;

    માટે (i = 0; i < no; i++)
      p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);

    (શૂન્ય)memset((અક્ષર *)p->g->setbits + (nbytes - css), 0, css);
  }
  /* XXX should not happen */
  જો (p->g->sets == ખાલી || p->g->setbits == ખાલી)
    જાઓ nomem;

  cs = &p->g->sets[no];
  cs->ptr = p->g->setbits + css * ((no) / CHAR_BIT);
  cs->mask = 1 << ((no) % CHAR_BIT);
  cs->hash = 0;
  cs->smultis = 0;
  cs->multis = ખાલી;

  પાછા (cs);
nomem:
  મુક્ત(p->g->sets);
  p->g->sets = ખાલી;
  મુક્ત(p->g->setbits);
  p->g->setbits = ખાલી;

  SETERROR(REG_ESPACE);
  /* caller's responsibility not to do set ops */
  પાછા (ખાલી);
}

/*
 - freeset - free a now-unused set
 */
સ્થિર શૂન્ય freeset(સંરચના parse *p, cset *cs) {
  size_t i;
  cset *ટોચ_2 = &p->g->sets[p->g->ncsets];
  size_t css = (size_t)p->g->csetsize;

  માટે (i = 0; i < css; i++)
    CHsub(cs, i);
  જો (cs == ટોચ_2 - 1) /* recover only the easy case */
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
સ્થિર પૂર્ણાંક /* set number */
freezeset(સંરચના parse *p, cset *cs) {
  uch h = cs->hash;
  size_t i;
  cset *ટોચ_2 = &p->g->sets[p->g->ncsets];
  cset *cs2;
  size_t css = (size_t)p->g->csetsize;

  /* look for an earlier one which is the same */
  માટે (cs2 = &p->g->sets[0]; cs2 < ટોચ_2; cs2++)
    જો (cs2->hash == h && cs2 != cs) {
      /* maybe */
      માટે (i = 0; i < css; i++)
        જો (!!CHIN(cs2, i) != !!CHIN(cs, i))
          તોડો; /* no */
      જો (i == css)
        તોડો; /* yes */
    }

  જો (cs2 < ટોચ_2) { /* found one */
    freeset(p, cs);
    cs = cs2;
  }

  પાછા ((પૂર્ણાંક)(cs - p->g->sets));
}

/*
 - firstch - return first character in a set (which must have at least one)
 */
સ્થિર પૂર્ણાંક /* character; there is no "none" value */
firstch(સંરચના parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;

  માટે (i = 0; i < css; i++)
    જો (CHIN(cs, i))
      પાછા ((અક્ષર)i);
  પ્રતિજ્ઞા(never);
  પાછા (0); /* arbitrary */
}

/*
 - nch - number of characters in a set
 */
સ્થિર પૂર્ણાંક nch(સંરચના parse *p, cset *cs) {
  size_t i;
  size_t css = (size_t)p->g->csetsize;
  પૂર્ણાંક n = 0;

  માટે (i = 0; i < css; i++)
    જો (CHIN(cs, i))
      n++;
  પાછા (n);
}

/*
 - mcadd - add a collating element to a cset
 */
સ્થિર શૂન્ય mcadd(સંરચના parse *p, cset *cs, અચલ અક્ષર *cp) {
  size_t oldend = cs->smultis;
  શૂન્ય *np;

  cs->smultis += strlen(cp) + 1;
  np = realloc(cs->multis, cs->smultis);
  જો (np == ખાલી) {
    જો (cs->multis)
      મુક્ત(cs->multis);
    cs->multis = ખાલી;
    SETERROR(REG_ESPACE);
    પાછા;
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
સ્થિર શૂન્ય mcinvert(સંરચના parse *p, cset *cs) {
  પ્રતિજ્ઞા(cs->multis == ખાલી); /* xxx */
}

/*
 - mccase - add case counterparts of the list of collating elements in a cset
 *
 * This would have to know the set of possibilities.  Implementation
 * is deferred.
 */
/* ARGSUSED */
સ્થિર શૂન્ય mccase(સંરચના parse *p, cset *cs) {
  પ્રતિજ્ઞા(cs->multis == ખાલી); /* xxx */
}

/*
 - isinsets - is this character in any sets?
 */
સ્થિર પૂર્ણાંક /* predicate */
isinsets(સંરચના re_guts *g, પૂર્ણાંક c) {
  uch *col;
  પૂર્ણાંક i;
  પૂર્ણાંક ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc = (uch)c;

  માટે (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    જો (col[uc] != 0)
      પાછા (1);
  પાછા (0);
}

/*
 - samesets - are these two characters in exactly the same sets?
 */
સ્થિર પૂર્ણાંક /* predicate */
samesets(સંરચના re_guts *g, પૂર્ણાંક c1, પૂર્ણાંક c2) {
  uch *col;
  પૂર્ણાંક i;
  પૂર્ણાંક ncols = (g->ncsets + (CHAR_BIT - 1)) / CHAR_BIT;
  unsigned uc1 = (uch)c1;
  unsigned uc2 = (uch)c2;

  માટે (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
    જો (col[uc1] != col[uc2])
      પાછા (0);
  પાછા (1);
}

/*
 - categorize - sort out character categories
 */
સ્થિર શૂન્ય categorize(સંરચના parse *p, સંરચના re_guts *g) {
  cat_t *cats = g->categories;
  પૂર્ણાંક c;
  પૂર્ણાંક c2;
  cat_t cat;

  /* avoid making error situations worse */
  જો (p->error != 0)
    પાછા;

  માટે (c = CHAR_MIN; c <= CHAR_MAX; c++)
    જો (cats[c] == 0 && isinsets(g, c)) {
      cat = g->ncategories++;
      cats[c] = cat;
      માટે (c2 = c + 1; c2 <= CHAR_MAX; c2++)
        જો (cats[c2] == 0 && samesets(g, c, c2))
          cats[c2] = cat;
    }
}

/*
 - dupl - emit a duplicate of a bunch of sops
 */
સ્થિર sopno                       /* start of duplicate */
dupl(સંરચના parse *p, sopno start, /* from here */
     sopno finish)                 /* to this less one */
{
  sopno ret = HERE();
  sopno len = finish - start;

  પ્રતિજ્ઞા(finish >= start);
  જો (len == 0)
    પાછા (ret);
  enlarge(p, p->ssize + len); /* this many unexpected additions */
  પ્રતિજ્ઞા(p->ssize >= p->slen + len);
  (શૂન્ય)memmove((અક્ષર *)(p->strip + p->slen), (અક્ષર *)(p->strip + start),
                (size_t)len * કદ(sop));
  p->slen += len;
  પાછા (ret);
}

/*
 - doemit - emit a strip operator
 *
 * It might seem better to implement this as a macro with a function as
 * hard-case backup, but it's just too big and messy unless there are
 * some changes to the data structures.  Maybe later.
 */
સ્થિર શૂન્ય doemit(સંરચના parse *p, sop op, size_t opnd) {
  /* avoid making error situations worse */
  જો (p->error != 0)
    પાછા;

  /* deal with oversize operands ("can't happen", more or less) */
  પ્રતિજ્ઞા(opnd < 1 << OPSHIFT);

  /* deal with undersized strip */
  જો (p->slen >= p->ssize)
    enlarge(p, (p->ssize + 1) / 2 * 3); /* +50% */
  પ્રતિજ્ઞા(p->slen < p->ssize);

  /* finally, it's all reduced to the easy case */
  p->strip[p->slen++] = SOP(op, opnd);
}

/*
 - doinsert - insert a sop into the strip
 */
સ્થિર શૂન્ય doinsert(સંરચના parse *p, sop op, size_t opnd, sopno pos) {
  sopno sn;
  sop s;
  પૂર્ણાંક i;

  /* avoid making error situations worse */
  જો (p->error != 0)
    પાછા;

  sn = HERE();
  EMIT(op, opnd); /* do checks, ensure space */
  પ્રતિજ્ઞા(HERE() == sn + 1);
  s = p->strip[sn];

  /* adjust paren pointers */
  પ્રતિજ્ઞા(pos > 0);
  માટે (i = 1; i < NPAREN; i++) {
    જો (p->pbegin[i] >= pos) {
      p->pbegin[i]++;
    }
    જો (p->pend[i] >= pos) {
      p->pend[i]++;
    }
  }

  memmove((અક્ષર *)&p->strip[pos + 1], (અક્ષર *)&p->strip[pos],
          (HERE() - pos - 1) * કદ(sop));
  p->strip[pos] = s;
}

/*
 - dofwd - complete a forward reference
 */
સ્થિર શૂન્ય dofwd(સંરચના parse *p, sopno pos, sop કિંમત_2) {
  /* avoid making error situations worse */
  જો (p->error != 0)
    પાછા;

  પ્રતિજ્ઞા(કિંમત_2 < 1 << OPSHIFT);
  p->strip[pos] = OP(p->strip[pos]) | કિંમત_2;
}

/*
 - enlarge - enlarge the strip
 */
સ્થિર શૂન્ય enlarge(સંરચના parse *p, sopno size) {
  sop *sp;

  જો (p->ssize >= size)
    પાછા;

  જો ((uintptr_t)size > SIZE_MAX / કદ(sop)) {
    SETERROR(REG_ESPACE);
    પાછા;
  }

  sp = (sop *)realloc(p->strip, size * કદ(sop));
  જો (sp == ખાલી) {
    SETERROR(REG_ESPACE);
    પાછા;
  }
  p->strip = sp;
  p->ssize = size;
}

/*
 - stripsnug - compact the strip
 */
સ્થિર શૂન્ય stripsnug(સંરચના parse *p, સંરચના re_guts *g) {
  g->nstates = p->slen;
  જો ((uintptr_t)p->slen > SIZE_MAX / કદ(sop)) {
    g->strip = p->strip;
    SETERROR(REG_ESPACE);
    પાછા;
  }

  g->strip = (sop *)realloc((અક્ષર *)p->strip, p->slen * કદ(sop));
  જો (g->strip == ખાલી) {
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
સ્થિર શૂન્ય findmust(સંરચના parse *p, સંરચના re_guts *g) {
  sop *scan;
  sop *start = 0;    /* start initialized in the default case, after that */
  sop *newstart = 0; /* newstart was initialized in the OCHAR case */
  sopno newlen;
  sop s;
  અક્ષર *cp;
  sopno i;

  /* avoid making error situations worse */
  જો (p->error != 0)
    પાછા;

  /* find the longest OCHAR sequence in strip */
  newlen = 0;
  scan = g->strip + 1;
  કરો {
    s = *scan++;
    પસંદગી (OP(s)) {
    સ્થિતિ OCHAR:        /* sequence member */
      જો (newlen == 0) /* new sequence */
        newstart = scan - 1;
      newlen++;
      તોડો;
    સ્થિતિ OPLUS_: /* things that don't break one */
    સ્થિતિ OLPAREN:
    સ્થિતિ ORPAREN:
      તોડો;
    સ્થિતિ OQUEST_: /* things that must be skipped */
    સ્થિતિ OCH_:
      scan--;
      કરો {
        scan += OPND(s);
        s = *scan;
        /* assert() interferes w debug printouts */
        જો (OP(s) != O_QUEST && OP(s) != O_CH && OP(s) != OOR2) {
          g->iflags |= REGEX_BAD;
          પાછા;
        }
      } જ્યારે (OP(s) != O_QUEST && OP(s) != O_CH);
      LLVM_FALLTHROUGH;
    મૂળભૂત:                  /* things that break a sequence */
      જો (newlen > g->mlen) { /* ends one */
        start = newstart;
        g->mlen = newlen;
      }
      newlen = 0;
      તોડો;
    }
  } જ્યારે (OP(s) != OEND);

  જો (g->mlen == 0) /* there isn't one */
    પાછા;

  /* turn it into a character string */
  g->must = malloc((size_t)g->mlen + 1);
  જો (g->must == ખાલી) { /* argh; just forget it */
    g->mlen = 0;
    પાછા;
  }
  cp = g->must;
  scan = start;
  માટે (i = g->mlen; i > 0; i--) {
    જ્યારે (OP(s = *scan++) != OCHAR)
      ચાલુ;
    પ્રતિજ્ઞા(cp < g->must + g->mlen);
    *cp++ = (અક્ષર)OPND(s);
  }
  પ્રતિજ્ઞા(cp == g->must + g->mlen);
  *cp++ = '\0'; /* just on general principles */
}

/*
 - pluscount - count + nesting
 */
સ્થિર sopno /* nesting depth */
pluscount(સંરચના parse *p, સંરચના re_guts *g) {
  sop *scan;
  sop s;
  sopno plusnest = 0;
  sopno maxnest = 0;

  જો (p->error != 0)
    પાછા (0); /* there may not be an OEND */

  scan = g->strip + 1;
  કરો {
    s = *scan++;
    પસંદગી (OP(s)) {
    સ્થિતિ OPLUS_:
      plusnest++;
      તોડો;
    સ્થિતિ O_PLUS:
      જો (plusnest > maxnest)
        maxnest = plusnest;
      plusnest--;
      તોડો;
    }
  } જ્યારે (OP(s) != OEND);
  જો (plusnest != 0)
    g->iflags |= REGEX_BAD;
  પાછા (maxnest);
}
