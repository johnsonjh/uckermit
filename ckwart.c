char *wartv = "Wart Preprocessor, 1A(211), 23 Apr 2021";

/* W A R T */

/*
 * Copyright (C) 1981-2011,
 *   Trustees of Columbia University in the City of New York.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   - Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *      
 *   - Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in   
 *       the documentation and/or other materials provided with the
 *       distribution.                                                     
 *     
 *   - Neither the name of Columbia University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission. 
 *       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

/*
 * Pre-process a lex-like file into a C program.
 *
 *
 * Author: Jeff Damens,
 *   Columbia University Center for Computing Activites, 11/84.
 *
 * Copyright (C) 1985,
 *  Trustees of Columbia University in the City of New York.
 *
 * Permission is granted to any individual or institution to use, copy,
 *   or redistribute this software so long as it is not sold for profit,
 *   provided this copyright notice is retained.
 *
 * Input format is:
 *  lines to be copied | %state <state names...>
 *  %%
 * <state> | <state,state,...> CHAR  { actions }
 * ...
 *  %%
 */

#include "ckcdeb.h"          /* Includes */
#include <stdio.h>
#ifdef __linux__
#include <stdlib.h>
#include <string.h>
#endif
#include <ctype.h>

#define TBL_TYPE "CHAR"

#define C_L  014             /* Formfeed */
#define SEP    1             /* Token types */
#define LBRACK 2
#define RBRACK 3
#define WORD   4
#define COMMA  5

/*
 * Storage
 * sizes
 */

#define MAXSTATES 50         /* max number of states */
#define MAXWORD 50           /* max # of chars/word */
#define SBYTES \
  ((MAXSTATES + 7) / 8)      /* # of bytes for state bitmask */

 /*
  * Name of wart function
  * in generated program
  */

#ifndef FNAME
#define FNAME "wart"
#endif

 /*
  * Structure for
  * state information
  */

struct trans {
  CHAR states[SBYTES];       /* included states */
  int anyst;                 /* true if this good from any state */
  CHAR inchr;                /* input character */
  int actno;                 /* associated action */
  struct trans *nxt;
};                           /* next transition */

typedef struct trans *Trans;

#ifndef __linux__
char *malloc();              /* Returns pointer (not int) */
#endif

 /*
  * Variables and
  * tables
  */

int lines, nstates, nacts;
char tokval[MAXWORD];
int tbl[MAXSTATES * 128];
char *tbl_type = TBL_TYPE;
char *txt1 = "\n#define BEGIN state =\n\nint state = 0;\n\n";
char *fname = FNAME;         /* function name goes here */
char *txt2 = "()\n\
{\n\
    int c, actno;\n\
    extern ";

 /*
  * Data type of state table is
  * inserted here (short or int)
  */

char *txt2a = " tbl[];\n\
    while (1) {\n\
        c = input();\n\
        if ((actno = tbl[c + state*128]) != -1)\n\
            switch(actno) {\n";

 /*
  * This program's output goes
  * here, followed by final text...
  */

char *txt3 = "\n            }\n    }\n}\n\n";

 /*
  * Turn on the bit associated
  * with the given state
  */

int
wsetstate(state, t)
int state;
Trans t;
{
  int idx, msk;
  idx = state / 8;           /* byte associated with state */
  msk = 0x80 >> (state % 8); /* bit mask for state */
  t->states[idx] |= msk;
  return 0;
}

 /*
  * See if the state is
  * involved in the transition
  */

int
teststate(state, t)
int state;
Trans t;
{
  int idx, msk;
  idx = state / 8;
  msk = 0x80 >> (state % 8);
  return t->states[idx] & msk;
}

 /*
  * Read input
  * from here...
  */

Trans
rdinput(infp, outfp)
FILE *infp, *outfp;
{
  Trans x, rdrules();
  lines = 1;   /* line counter */
  nstates = 0; /* no states */
  nacts = 0;   /* no actions yet */

  fprintf(outfp, "%c* WARNING -- This C source program generated by ", '/');
  fprintf(outfp, "Wart preprocessor. */\n");
  fprintf(outfp, "%c* Do not edit this file; edit the Wart-format ", '/');
  fprintf(outfp, "source file instead, */\n");
  fprintf(outfp, "%c* and then run it through Wart to produce a new ", '/');
  fprintf(outfp, "C source file.     */\n\n");
  fprintf(outfp, "%c* Wart Version Info: */\n", '/');
  fprintf(outfp, "char *wartv = \"%s\";\n\n", wartv);

  initial(infp, outfp);     /* read state names, initial defs */
  prolog(outfp);            /* write out our initial code */
  x = rdrules(infp, outfp); /* read rules */
  epilogue(outfp);          /* write out epilogue code */
  return x;
}

 /*
  * Initial - read initial definitions and state names.
  * Returns on EOF or %%.
  */

int
initial(infp, outfp)
FILE *infp, *outfp;
{
  int c;
  char wordbuf[MAXWORD];
  while ((c = getc(infp)) != EOF) {
    if (c == '%') {
      rdword(infp, wordbuf);
      if (strcmp(wordbuf, "states") == 0)
        rdstates(infp, outfp);
      else if (strcmp(wordbuf, "%") == 0)
        return 0;
      else
        fprintf(outfp, "%%%s", wordbuf);
    } else
      putc(c, outfp);
    if (c == '\n')
      lines++;
  }
  return 0;
}

 /*
  * boolean function to tell if the
  * given character can be part of a word.
  */

int
isin(s, c)
char *s;
int c;
{
  for (; *s != '\0'; s++)
    if (*s == c)
      return 1;
  return 0;
}

int
isword(c)
int c;
{
  static char special[] = ".%_-$@";         /* these are allowable */
  return isalnum(c) || isin(special, c);
}

 /*
  * Read the next word
  * into the given buffer.
  */

int
rdword(fp, buf)
FILE *fp;
char *buf;
{
  int len = 0, c;
  while (isword(c = getc(fp)) && ++len < MAXWORD)
    *buf++ = c;
  *buf++ = '\0';                            /* tie off word */
  ungetc(c, fp);                            /* put break char back */
  return 0;
}

/*
 * Read state names,
 * up to a newline.
 */

int
rdstates(fp, ofp)
FILE *fp, *ofp;
{
  int c;
  char wordbuf[MAXWORD];
  while ((c = getc(fp)) != EOF && c != '\n') {
    if (isspace(c) || c == C_L)
      continue;                             /* skip whitespace */
    ungetc(c, fp);                          /* put char back */
    rdword(fp, wordbuf);                    /* read the whole word */
    enter(wordbuf, ++nstates);              /* put into symbol tbl */
    fprintf(
      ofp,
        "#define %s %d\n",
          wordbuf,
            nstates);
  }
  lines++;
  return 0;
}

 /*
  * Allocate a new,
  * empty transition node
  */

Trans
newtrans()
{
  Trans new;
  int i;

  new = (Trans)malloc(sizeof(struct trans));
  for (i = 0; i < SBYTES; i++)
    new->states[i] = 0;
  new->anyst = 0;
  new->nxt = NULL;
  return new;
}

 /*
  * Read all
  * the rules.
  */

Trans
rdrules(fp, out)
FILE *fp, *out;
{
  Trans head, cur, prev;
  int curtok;
  head = cur = NULL;
  while ((curtok = gettoken(fp)) != SEP)

    switch (curtok) {
    case LBRACK:
      if (cur == NULL)
        cur = newtrans();
      else
        fatal("duplicate state list");
      statelist(fp, cur); /* set states */
      continue;           /* prepare to read char */

    case WORD:
      if (strlen(tokval) != 1)
        fatal("multiple chars in state");
      if (cur == NULL) {
        cur = newtrans();
        cur->anyst = 1;
      }
      cur->actno = ++nacts;
      cur->inchr = tokval[0];
      if (head == NULL)
        head = cur;
      else
        prev->nxt = cur;
      prev = cur;
      cur = NULL;
      copyact(fp, out, nacts);
      break;

    default:
      fatal("bad input format");
    }
  return head;
}

 /*
  * Read a list of (comma-separated) states,
  * set them in the given transition.
  */

int
statelist(fp, t)
FILE *fp;
Trans t;
{
  int curtok, sval;
  curtok = COMMA;
  while (curtok != RBRACK) {
    if (curtok != COMMA)
      fatal("missing comma");
    if ((curtok = gettoken(fp)) != WORD)
      fatal("missing state name");
    if ((sval = lkup(tokval)) == -1) {
      fprintf(stderr, "state %s undefined\n", tokval);
      fatal("undefined state");
    }
    wsetstate(sval, t);
    curtok = gettoken(fp);
  }
  return 0;
}

 /*
  * Copy an action from the
  * input to the output file
  */

int
copyact(inp, outp, actno)
FILE *inp, *outp;
int actno;
{
  int c, bcnt;
  fprintf(outp, "case %d:\n", actno);
  while (c = getc(inp), (isspace(c) || c == C_L))
    if (c == '\n')
      lines++;
  if (c == '{') {
    bcnt = 1;
    fputs("    {", outp);
    while (bcnt > 0 && (c = getc(inp)) != EOF) {
      if (c == '{')
        bcnt++;
      else if (c == '}')
        bcnt--;
      else if (c == '\n')
        lines++;
      putc(c, outp);
    }
    if (bcnt > 0)
      fatal("action doesn't end");
  } else {
    while (c != '\n' && c != EOF) {
      putc(c, outp);
      c = getc(inp);
    }
    lines++;
  }
  fprintf(outp, "\n    break;\n");
  return 0;
}

 /*
  * Find the action associated with a
  * given character and state.
  * returns -1 if one can't be found.
  */

int
faction(hd, state, chr)
Trans hd;
int state, chr;
{
  while (hd != NULL) {
    if (hd->anyst || teststate(state, hd))
      if (hd->inchr == '.' || hd->inchr == chr)
        return hd->actno;
    hd = hd->nxt;
  }
  return -1;
}

 /*
  * Empty the
  * table...
  */

int
emptytbl()
{
  int i;

  for (i = 0; i < nstates * 128; i++)
    tbl[i] = -1;
  return 0;
}

 /*
  * Add the specified action to the
  * output for the given state and chr.
  */

int
addaction(act, state, chr)
int act, state, chr;
{
        tbl[state * 128 + chr] = act;
        return 0;
}

int
writetbl(fp)
FILE *fp;
{
        warray(fp, "tbl", tbl, 128 * (nstates + 1), TBL_TYPE);
        return 0;
}

 /*
  * Write an array to the output file,
  * given its name and size.
  */

int
warray(fp, nam, cont, siz, typ)
FILE *fp;
char *nam;
int cont[], siz;
char *typ;
{
  int i;
  fprintf(fp, "%s %s[] = {\n", typ, nam);
  for (i = 0; i < siz;) {
    fprintf(fp, "%2d, ", cont[i]);
    if ((++i % 16) == 0)
      putc('\n', fp);
  }
  fprintf(fp, "};\n");
  return 0;
}

 /*
  * Main
  * function
  */

int
main(argc, argv)
int argc;
char *argv[];
{
  Trans head;
  int state, c;
  FILE *infile, *outfile;

  if (argc > 1) {
    if ((infile = fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "Can't open %s\n", argv[1]);
      fatal("unreadable input file");
    }
  } else
    infile = stdin;

  if (argc > 2) {
    if ((outfile = fopen(argv[2], "w")) == NULL) {
      fprintf(stderr, "Can't write to %s\n", argv[2]);
      fatal("bad output file");
    }
  } else
    outfile = stdout;

  clrhash();                       /* empty hash table */
  head = rdinput(infile, outfile); /* read input file */
  emptytbl();                      /* empty our tables */
  for (state = 0; state <= nstates; state++)
    for (c = 1; c < 128; c++)
      addaction(faction(head, state, c), state,
        c);                        /* find actions, add to tbl */
  writetbl(outfile);
  copyrest(infile, outfile);
  printf("%d states, %d actions\n", nstates, nacts);
  return GOOD_EXIT;
}

 /*
  * Fatal error
  * handler
  */

int
fatal(msg)
char *msg;
{
  fprintf(
    stderr,
      "error in line %d: %s\n",
        lines,
          msg);
  exit(BAD_EXIT);
  return(BAD_EXIT);               /* unreachable */
}

int
prolog(outfp)
FILE *outfp;
{
  int c;
  while ((c = *txt1++) != '\0')
    putc(c, outfp);
  while ((c = *fname++) != '\0')
    putc(c, outfp);
  while ((c = *txt2++) != '\0')
    putc(c, outfp);
  while ((c = *tbl_type++) != '\0')
    putc(c, outfp);
  while ((c = *txt2a++) != '\0')
    putc(c, outfp);
  return 0;
}

int
epilogue(outfp)
FILE *outfp;
{
  int c;
  while ((c = *txt3++) != '\0')
    putc(c, outfp);
  return 0;
}

int
copyrest(in, out)
FILE *in, *out;
{
  int c;
  while ((c = getc(in)) != EOF)
    putc(c, out);
  return 0;
}

/*
 * gettoken - returns token type of next token, sets tokval
 * to the string value of the token if appropriate.
 */

int
gettoken(fp)
FILE *fp;
{
  int c;
  while (1) { /* loop if reading comments... */
    do {
      c = getc(fp);
      if (c == '\n')
        lines++;
    } while ((isspace(c) || c == C_L)); /* skip whitespace */
    switch (c) {
    case EOF:
      return SEP;
    case '%':
      if ((c = getc(fp)) == '%')
        return SEP;
      tokval[0] = '%';
      tokval[1] = c;
      rdword(fp, tokval + 2);
      return WORD;
    case '<':
      return LBRACK;
    case '>':
      return RBRACK;
    case ',':
      return COMMA;
    case '/':
      if ((c = getc(fp)) == '*') {
        rdcmnt(fp); /* skip over the comment */
        continue;
      } /* and keep looping */
      else {
        ungetc(c, fp); /* put this back into input */
        c = '/';
      } /* put character back, fall thru */

    default:
      if (isword(c)) {
        ungetc(c, fp);
        rdword(fp, tokval);
        return WORD;
      } else
        fatal("Invalid character in input");
    }
  }
}

 /*
  * Skip over
  * a comment
  */

int
rdcmnt(fp)
FILE *fp;
{
  int c, star, prcnt;
  prcnt = star = 0; /* no star seen yet */
  while (!((c = getc(fp)) == '/' && star)) {
    if (c == EOF || (prcnt && c == '%'))
      fatal("Unterminated comment");
    prcnt = (c == '%');
    star = (c == '*');
    if (c == '\n')
      lines++;
  }
  return 0;
}

 /*
  * Symbol table management for wart
  *
  * Entry points:
  *   clrhash - empty hash table.
  *   enter - enter a name into the symbol table
  *   lkup - find a name's value in the symbol table.
  */

#define HASHSIZE 101  /* # of entries in hash table */
struct sym {
  char *name;         /* symbol name */
  int val;            /* value */
  struct sym *hnxt;
}                     /* next on collision chain */
    * htab[HASHSIZE]; /* the hash table */

 /*
  * Empty the hash table
  * before using it...
  */

int
clrhash() {
  int i;

  for (i = 0; i < HASHSIZE; i++)
    htab[i] = NULL;
  return 0;
}

 /*
  * Compute the value of
  * the hash for a symbol
  */

int
hash(name)
char *name;
{
  int sum;
  for (sum = 0; *name != '\0'; name++)
    sum += (sum + *name);
  sum %= HASHSIZE;     /* take sum mod hashsize */
  if (sum < 0)
    sum += HASHSIZE;   /* disallow negative hash value */
  return sum;
}

/*
 * Make a private
 * copy of a string...
 */

char *
copy(s)
char *s;
{
  char *new;
  new = (char *)malloc(strlen(s) + 1);
  strcpy(new, s);
  return new;
}

/*
 * Enter state name
 * into the hash table
 */

int
enter(name, svalue)
char *name;
int svalue;
{
  int h;
  struct sym *cur;
  if (lkup(name) != -1) {
    fprintf(stderr, "state %s appears twice...\n", name);
    exit(BAD_EXIT);
  }
  h = hash(name);
  cur = (struct sym *)malloc(sizeof(struct sym));
  cur->name = copy(name);
  cur->val = svalue;
  cur->hnxt = htab[h];
  htab[h] = cur;
  return 0;
}

 /*
  * Find name in the symbol table,
  * return its value.
  * Returns -1 if not found.
  */

int
lkup(name)
char *name;
{
  struct sym *cur;
  for (cur = htab[hash(name)]; cur != NULL; cur = cur->hnxt)
    if (strcmp(cur->name, name) == 0)
      return cur->val;
  return -1;
}
