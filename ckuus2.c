/* C K U U S 2 -- "User Interface" STRINGS module for UNIX Kermit */

/*
 * Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 * Columbia University Center for Computing Activities.
 *
 * First released January 1985.
 *
 * Copyright (C) 1985, 1989,
 *   Trustees of Columbia University in the City of New York.
 *
 * Permission is granted to any individual or institution to use, copy,
 *   or redistribute this software so long as it is not sold for profit,
 *   provided this copyright notice is retained.
 */

/*
 * This module separates long strings from
 * the body of the ckuser module.
 */

#include "ckcdeb.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"
#include <ctype.h>
#include <stdio.h>

#ifdef __linux__
#include <string.h>
#endif

/*    
 * Conditional
 * Includes       
 */   
           
/*
 * Whether to include
 * <sys/file.h>...
 */

#ifndef PROVX1
#ifndef XENIX
#ifndef unos
#include <sys/file.h>             /* File information */
#endif
#endif
#endif

#ifdef BSD4
#include <fcntl.h>
#include <sys/file.h>
#endif

/*
 * System III
 * or System V
 */

#ifdef UXIII
#include <termio.h>
#include <sys/ioctl.h>
#include <errno.h>                /* error numbers for system returns */
#include <fcntl.h>                /* directory reading for locking */
#ifdef ATT7300
#ifndef NOCKUDIA
#include <sys/phone.h>            /* UNIX PC, internal modem dialer */
#endif
#endif
#endif

#ifdef HPUX
#include <sys/modem.h>
#endif

/*
 * Not System III
 * or System V
 */

#ifndef UXIII
#include <sgtty.h>                /* Set/Get tty modes */
#ifndef PROVX1
#ifndef V7
#ifndef BSD41
#include <sys/time.h>             /* Clock info (for break generation) */
#endif
#endif
#endif
#endif

#ifdef BSD41
#include <sys/timeb.h>            /* BSD 4.1 ... ceb */
#endif

#ifdef BSD29
#include <sys/timeb.h>            /* BSD 2.9 (Vic Abell, Purdue) */
#endif

#ifdef ultrix
#include <sys/ioctl.h>
#endif

extern CHAR mystch, stchr, eol, seol, padch, mypadc, ctlq;
extern CHAR data[], *rdatap, ttname[];
extern char cmdbuf[], line[], debfil[], pktfil[], sesfil[], trafil[];
extern int nrmt, nprm, dfloc, deblog, seslog, speed, local, parity, duplex;
extern int turn, turnch, pktlog, tralog, mdmtyp, flow, cmask, timef, spsizf;
extern int rtimo, timint, srvtim, npad, mypadn, bctr, delay;
extern int maxtry, spsiz, urpsiz, maxsps, maxrps, ebqflg, ebq;
extern int rptflg, rptq, fncnv, binary, pktlog, warn, quiet, fmask, keep;
extern int tsecs, bctu, len, atcapu, lpcapu, swcapu, wslots, sq, rpsiz;
extern int capas, atcapr;
extern long filcnt, tfc, tlci, tlco, ffc, flci, flco;
extern char *dftty, *versio, *ckxsys;
extern struct keytab prmtab[];
extern struct keytab remcmd[];
#ifndef NODOHLP
int bcharc;
#endif

static char *hlp1[] = {
"\r",
"  Usage: kermit [ -x arg [ -x arg ] ... [ -yyy ] ... ] ]\n",
"    'x' is an option that requires an argument,\n",
"      'y' is an option with no argument:\n",
"        ACTION (* options also require -l and -b) --\n",
"          -s file(s) Send (Use '-s -' to send from stdin)\n",
"          -r         Receive\n",
"          -k         Receive to stdout\n",
"        * -g file(s) Get remote file(s) from server (quote wildcards)\n",
"          -a name    Alternate name (used with -s, -r, and -g)\n",
"          -x         Enter server mode\n",
"        * -f         Finish remote server\n",
"        * -c         Connect before transaction\n",
"        * -n         Connect after transaction\n",
"        SETTING --\n",
"          -l line    Communication line device\n",
"          -b baud    Line speed, e.g. 1200\n",
"          -i         Binary file (text by default)\n",
"          -p x       Parity, 'x' is one of 'e', 'o', 'm', 's', or 'n'\n",
"          -t         Set line turnaround to XON/XOFF (half duplex)\n",
"          -w         Don't write over preexisting files\n",
"          -q         Be quiet during file transfer\n",
#ifdef DEBUG
"          -d         Log debugging info to debug.log\n",
#endif
"          -e length  (Extended) Receive packet length\n",
"  If no ACTION is specified, kernit enters interactive dialog.\n",
""};

/* U S A G E */

usage()
{ 
	conola(hlp1);
}

/* 
 * Help string
 * definitions
 */

#ifndef NODOHLP
static char *tophlp[] = {"\n\
Type ? for a list of commands, type 'help x' for any command x.\n\
While typing commands, use the following special characters:\n\n\
 DEL, RUBOUT, BACKSPACE, CTRL-H: Delete the most recent character typed.\n\
 CTRL-W:  Delete the most recent word typed.\n",
                         "\
 CTRL-U:  Delete the current line.\n\
 CTRL-R:  Redisplay the current line.\n\
 ?:       (question mark) Display help on the current command or field.\n\
 ESCAPE:  (or TAB) Attempt to complete the current field.\n",
                         "\
 \\        (backslash) include the following character literally.\n\n\
From system level, type 'kermit -h' to get help about command line args.\
\n",
                         ""};
#endif

static char *hmxxbye = "\
Shut down and log out a remote Kermit server";

static char *hmxxclo = "\
Close one of the following logs:\n\
 session, transaction, packet, debugging -- 'help log' for further info.";

static char *hmxxcon = "\
Connect to a remote system via the tty device given in the\n\
most recent 'set line' command";

static char *hmxxget = "\
Format: 'get filespec'.  Tell the remote Kermit server to send the named\n\
files.  If filespec is omitted, then you are prompted for the remote and\n\
local filenames separately.";

static char *hmxxlg[] = {
    "\
Record information in a log file:\n\n\
 debugging             Debugging information, to help track down\n\
  (default debug.log)  bugs in the C-Kermit program.\n\n\
 packets               Kermit packets, to help track down protocol problems.\n\
  (packet.log)\n\n",

    " session               Terminal session, during CONNECT command.\n\
  (session.log)\n\n\
 transactions          Names and statistics about files transferred.\n\
  (transact.log)\n",
    ""};

#ifndef NOCKUSCR
static char *hmxxlogi[] = {
    "\
Syntax: script text\n\n",
    "Login to a remote system using the text provided.  The login script\n",
    "is intended to operate similarly to uucp \"L.sys\" entries.\n",
    "A login script is a sequence of the form:\n\n",
    "       expect send [expect send] . . .\n\n",
    "where 'expect' is a prompt or message to be issued by the remote site, "
    "and\n",
    "'send' is the names, numbers, etc, to return.  The send may also be the\n",
    "keyword EOT, to send control-d, or BREAK, to send a break.  Letters in\n",
    "send may be prefixed by ~ to send special characters.  These are:\n",
    "~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t\n",
    "append a return, and ~o[o[o]] for octal of a character.  As with some \n",
    "uucp systems, sent strings are followed by ~r unless they end with "
    "~c.\n\n",
    "Only the last 7 characters in each expect are matched.  A null expect,\n",
    "e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect\n",
    "that a sequence might not arrive, as with uucp, conditional sequences\n",
    "may be expressed in the form:\n\n",
    "       -send-expect[-send-expect[...]]\n\n",
    "where dashed sequences are followed as long as previous expects fail.\n",
    ""};
#endif

static char *hmxxrc[] = {
    "\
Format: 'receive [filespec]'.  Wait for a file to arrive from the other\n\
Kermit, which must be given a 'send' command.  If the optional filespec is\n",

    "given, the (first) incoming file will be stored under that name, otherwise\n\
it will be stored under the name it arrives with.",
    ""};

static char *hmxxsen = "\
Format: 'send file1 [file2]'.  File1 may contain wildcard characters '*' or\n\
'?'.  If no wildcards, then file2 may be used to specify the name file1 is\n\
sent under; if file2 is omitted, file1 is sent under its own name.";

static char *hmxxser = "\
Enter server mode on the currently selected line.  All further commands\n\
will be taken in packet form from the other Kermit program.";

static char *hmhset[] = {
    "\
The 'set' command is used to establish various communication or file\n",
    "parameters.  The 'show' command can be used to display the values of\n",
    "'set' parameters.  Help is available for each individual parameter;\n",
    "type 'help set ?' to see what's available.\n", ""};

static char *hmxychkt[] = {"\
Type of packet block check to be used for error detection, 1, 2, or 3.\n",
                           "Type 1 is standard, and catches most errors.  "
                           "Types 2 and 3 specify more\n",
                           "rigorous checking at the cost of higher overhead.  "
                           "Not all Kermit programs\n",
                           "support types 2 and 3.\n", ""};

static char *hmxyf[] = {
    "\
set file: names, type, warning, display.\n\n",
    "'names' are normally 'converted', which means file names are converted\n",
    "to 'common form' during transmission; 'literal' means use filenames\n",
    "literally (useful between like systems).\n\n",
    "'type' is normally 'text', in which conversion is done between Unix\n",
    "newlines and CRLF line delimiters; 'binary' means to do no conversion.\n",
    "Use 'binary' for executable programs or binary data.\n\n",
    "'warning' is 'on' or 'off', normally off.  When off, incoming files "
    "will\n",
    "overwrite existing files of the same name.  When on, new names will be\n",
    "given to incoming files whose names are the same as existing files.\n",
    "\n\
'display' is normally 'on', causing file transfer progress to be displayed\n",
    "on your screen when in local mode.  'set display off' is useful for\n",
    "allowing file transfers to proceed in the background.\n\n",
    ""};

static char *hmhrmt[] = {
    "\
The 'remote' command is used to send file management instructions to a\n",
    "remote Kermit server.  There should already be a Kermit running in "
    "server\n",
    "mode on the other end of the currently selected line.  Type 'remote ?' "
    "to\n",
    "see a list of available remote commands.  Type 'help remote x' to get\n",
    "further information about a particular remote command 'x'.\n",
    ""};

/* D O H L P -- Give a help message */

dohlp(xx) int xx;
{
  int x, y;

  if (xx < 0)
    return xx;
  switch (xx) {

#ifndef NODOHLP
  case XXBYE:
    return hmsg(hmxxbye);

  case XXCLO:
    return hmsg(hmxxclo);

  case XXCON:
    return hmsg(hmxxcon);

  case XXCWD:
    return hmsg("Change Working Directory, equivalent to Unix 'cd' command");

  case XXDEL:
    return hmsg("Delete a local file or files");

  case XXDIAL:
#ifndef NOCKUDIA
    return hmsg("Dial a number using modem autodialer");
#endif

  case XXDIR:
    return hmsg("Display a directory of local files");

  case XXECH:
    return hmsg("Display the rest of the command on the terminal,\n\
useful in command files.");

  case XXEXI:
  case XXQUI:
    return hmsg("Exit from the Kermit program, closing any open logs.");

  case XXFIN:
    return hmsg("\
Tell the remote Kermit server to shut down without logging out.");

  case XXGET:
    return hmsg(hmxxget);

  case XXHAN:
#ifndef NOCKUDIA
    return hmsg("Hang up the phone.");
#endif

  case XXHLP:
    return hmsga(tophlp);

  case XXLOG:
    return hmsga(hmxxlg);

  case XXLOGI:
    return hmsga(hmxxlogi);

  case XXREC:
    return hmsga(hmxxrc);

  case XXREM:
    if ((y = cmkey(remcmd, nrmt, "Remote command", "")) == -2)
      return y;
    if (y == -1)
      return y;
    if ((x = cmcfm()) < 0)
      return x;
    return dohrmt(y);

  case XXSEN:
    return hmsg(hmxxsen);

  case XXSER:
    return hmsg(hmxxser);

  case XXSET:
    if ((y = cmkey(prmtab, nprm, "Parameter", "")) == -2)
      return y;
    if (y == -2)
      return y;
    if ((x = cmcfm()) < 0)
      return x;
    return dohset(y);

#ifndef NOPUSH
  case XXSHE:
    return hmsg("\
Issue a command to the Unix shell (space required after '!')");
#endif

  case XXSHO:
    return hmsg("\
Display current values of 'set' parameters; 'show version' will display\n\
program version information for each of the C-Kermit modules.");

  case XXSPA:
    return hmsg("Display disk usage in current device, directory");

  case XXSTA:
    return hmsg("Display statistics about most recent file transfer");

  case XXTAK:
    return hmsg("\
Take Kermit commands from the named file.  Kermit command files may\n\
themselves contain 'take' commands, up to a reasonable depth of nesting.");

  case XXTRA:
    return hmsg("\
Raw upload. Send a file, a line at a time (text) or a character at a time.\n\
For text, wait for turnaround character (default 10 = LF) after each line.\n\
Specify 0 for no waiting.");
#endif
  default:
    if ((x = cmcfm()) < 0)
      return x;
    printf("Not available yet - %s\n", cmdbuf);
    break;
  }
  return 0;
}

/* H M S G -- Get confirmation, then print the given message */

hmsg(s) char *s;
{
  int x;
  if ((x = cmcfm()) < 0)
    return x;
  puts(s);
  return 0;
}

hmsga(s) char *s[];
{ /* Same function, but for arrays */
  int x, i;
  if ((x = cmcfm()) < 0)
    return x;
  for (i = 0; *s[i]; i++)
    fputs(s[i], stdout);
  putc('\n', stdout);
  return 0;
}

/* B C A R C B -- Help format help for baud rates */

#ifndef NODOHLP
bcarcb(calsp) long calsp;
{
#ifndef MAXBRATE
#ifdef B4000000
#define MAXBRATE 4000000
#endif
#endif

#ifndef MAXBRATE
#ifdef B3500000
#define MAXBRATE 3500000
#endif
#endif

#ifndef MAXBRATE
#ifdef B2500000
#define MAXBRATE 2500000
#endif
#endif

#ifndef MAXBRATE
#ifdef B2000000
#define MAXBRATE 2000000
#endif
#endif

#ifndef MAXBRATE
#ifdef B1152000
#define MAXBRATE 1152000
#endif
#endif

#ifndef MAXBRATE
#ifdef B1000000
#define MAXBRATE 1000000
#endif
#endif

#ifndef MAXBRATE
#ifdef B921600
#define MAXBRATE 921600
#endif
#endif

#ifndef MAXBRATE
#ifdef B576000
#define MAXBRATE 576000
#endif
#endif

#ifndef MAXBRATE
#ifdef B500000
#define MAXBRATE 500000
#endif
#endif

#ifndef MAXBRATE
#ifdef B460000
#define MAXBRATE 460000
#endif
#endif

#ifndef MAXBRATE
#ifdef B230000
#define MAXBRATE 230000
#endif
#endif

#ifndef MAXBRATE
#ifdef B115200
#define MAXBRATE 115200
#endif
#endif

#ifndef MAXBRATE
#ifdef B76800
#define MAXBRATE 76800
#endif
#endif

#ifndef MAXBRATE
#ifdef B57600
#define MAXBRATE 57600
#endif
#endif

#ifndef MAXBRATE
#ifdef B38400
#define MAXBRATE 38400
#endif
#endif

#ifndef MAXBRATE
#ifdef B19200
#define MAXBRATE 19200
#endif
#endif

#ifndef MAXBRATE
#ifdef B9600
#define MAXBRATE 9600
#endif
#endif

#ifndef MAXBRATE
#ifdef B4800
#define MAXBRATE 4800
#endif
#endif

#ifndef MAXBRATE
#ifdef B2400
#define MAXBRATE 2400
#endif
#endif

#ifndef MAXBRATE
#ifdef B1800
#define MAXBRATE 1800
#endif
#endif

#ifndef MAXBRATE
#ifdef B1200
#define MAXBRATE 1200
#endif
#endif

#ifndef MAXBRATE
#ifdef B600
#define MAXBRATE 600
#endif
#endif

#ifndef MAXBRATE
#ifdef B300
#define MAXBRATE 300
#endif
#endif

#ifndef MAXBRATE
#ifdef B200
#define MAXBRATE 200
#endif
#endif

#ifndef MAXBRATE
#ifdef B150
#define MAXBRATE 150
#endif
#endif

#ifndef MAXBRATE
#ifdef B134
#define MAXBRATE 134
#endif
#endif

#ifndef MAXBRATE
#ifdef B110
#define MAXBRATE 110
#endif
#endif

#ifndef MAXBRATE
#ifdef B75
#define MAXBRATE 75
#endif
#endif

#ifndef MAXBRATE
#ifdef B50
#define MAXBRATE 50
#endif
#endif

#ifndef MAXBRATE
#ifdef B0
#define MAXBRATE 0
#endif
#endif

#ifndef MAXBRATE
#define MAXBRATE 19200
#endif

  int brpln = 0;

  if (calsp < 0) brpln = 4;
  if (calsp > 10) brpln = 5;
  if (calsp > 100) brpln = 6;
  if (calsp > 1000) brpln = 7;
  if (calsp > 10000) brpln = 8;
  if (calsp > 100000) brpln = 9;
  if (calsp > 1000000) brpln = 10;
  if (calsp > 10000000) brpln = 11;
  
  printf (" %ld",calsp);
  if (calsp >= MAXBRATE)
	  printf(".\n");
  else
	  printf(",");
  bcharc=bcharc+brpln;
  if (bcharc >= 66) {
    printf("\n");
    bcharc = 0;
  }
}
#endif

/* D O H S E T -- Give help for SET command */

dohset(xx) int xx;
{
  if (xx == -3)
    return hmsga(hmhset);
  if (xx < 0)
    return xx;
  switch (xx) {
#ifndef NODOHLP
  case XYATTR:
    puts("Turn Attribute packet exchange off or on");
    return 0;

  case XYIFD:
    puts("Discard or Keep incompletely received files, default is discard");
    return 0;

  case XYCHKT:
    return hmsga(hmxychkt);

  case XYDELA:
    puts("\
Number of seconds to wait before sending first packet after 'send' command.");
    return 0;

  case XYTERM:
    puts("\
'set terminal bytesize 7 or 8' to use 7- or 8-bit terminal characters.");
    return 0;

  case XYDUPL:
    puts("\
During 'connect': 'full' means remote host echoes, 'half' means this program");
    puts("does its own echoing.");
    return 0;

  case XYESC:
    printf("%s", "\
Decimal ASCII value for escape character during 'connect', normally 28\n\
(Control-\\)\n");
    return 0;

  case XYFILE:
    return hmsga(hmxyf);

  case XYFLOW:
    puts("\
Type of flow control to be used.  Choices are 'xon/xoff' and 'none'.");
    puts("normally xon/xoff.");
    return 0;

  case XYHAND:
    puts("\
Decimal ASCII value for character to use for half duplex line turnaround");
    puts("handshake.  Normally, handshaking is not done.");
    return 0;

  case XYLINE:
    printf("\
Device name of communication line to use.  Normally %s.\n",
           dftty);
    if (!dfloc) {
      printf("\
If you set the line to other than %s, then Kermit\n",
             dftty);
      printf("\
will be in 'local' mode; 'set line' will reset Kermit to remote mode.\n");
#ifndef NOCKUDIA
      puts("\
If the line has a modem, and if the modem-dialer is set to direct, this");
      puts("\
command causes waiting for a carrier detect (e.g. on a hayes type modem).");
      puts("\
This can be used to wait for incoming calls.");
      puts("\
To use the modem to dial out, first set modem-dialer (e.g., to hayes), then");
      puts("set line, next issue the dial command, and finally connect.");
#endif
    }
    return 0;

  case XYMODM:
#ifndef NOCKUDIA
    puts("\
Type of modem for dialing remote connections.  Needed to indicate modem can");
    puts("\
be commanded to dial without 'carrier detect' from modem.  Many recently");
    puts("\
manufactured modems use 'hayes' protocol.  Type 'set modem ?' to see what");
    puts("\
types of modems are supported by this program.");
#endif
    return 0;

  case XYPARI:
    puts("Parity to use during terminal connection and file transfer:");
    puts("even, odd, mark, space, or none.  Normally none.");
    return 0;

  case XYPROM:
    puts("Prompt string for this program, normally 'C-Kermit>'.");
    return 0;

  case XYRETR:
    puts("\
How many times to retransmit a particular packet before giving up");
    return 0;

  case XYSPEE:
    puts("\
Communication line speed for external tty line specified in most recent");
    puts("\
'set line' command.  Any of the common baud rates:");
#ifdef B0
	bcarcb(0);
#endif
#ifdef B50
	bcarcb(50);
#endif
#ifdef B75
	bcarcb(75);
#endif
#ifdef B110
	bcarcb(110);
#endif
#ifdef B134
	bcarcb(134);
#endif
#ifdef B150
	bcarcb(150);
#endif
#ifdef B200
	bcarcb(200);
#endif
#ifdef B300
	bcarcb(300);
#endif
#ifdef B600
	bcarcb(600);
#endif
#ifdef B1200
	bcarcb(1200);
#endif
#ifdef B1800
	bcarcb(1800);
#endif
#ifdef B2400
	bcarcb(2400);
#endif
#ifdef B4800
	bcarcb(4800);
#endif
#ifdef B9600
	bcarcb(9600);
#endif
#ifdef B19200
	bcarcb(19200);
#endif
#ifdef B38400
	bcarcb(38400);
#endif
#ifdef B57600
	bcarcb(57600);
#endif
#ifdef B115200
	bcarcb(115200);
#endif
#ifdef B230400
	bcarcb(230400);
#endif
#ifdef B460800
	bcarcb(460800);
#endif
#ifdef B500000
	bcarcb(500000);
#endif
#ifdef B921600
	bcarcb(921600);
#endif
#ifdef B1000000
	bcarcb(1000000);
#endif
#ifdef B1152000
	bcarcb(1152000);
#endif
#ifdef B1500000
	bcarcb(1500000);
#endif
#ifdef B2000000
	bcarcb(2000000);
#endif
#ifdef B2500000
	bcarcb(2500000);
#endif
#ifdef B3000000
	bcarcb(3000000);
#endif
#ifdef B3500000
	bcarcb(3500000);
#endif
#ifdef B4000000
	bcarcb(4000000);
#endif
    return 0;

  case XYRECV:
    puts("\
Specify parameters for inbound packets:");
    puts("\
End-Of-Packet (ASCII value), Packet-Length (1000 or less),");
    puts("\
Padding (amount, 94 or less), Pad-Character (ASCII value),");
    puts("\
Start-Of-Packet (ASCII value), and Timeout (94 seconds or less),");
    puts("\
all specified as decimal numbers.");
    return 0;

  case XYSEND:
    puts("\
Specify parameters for outbound packets:");
    puts("\
End-Of-Packet (ASCII value), Packet-Length (2000 or less),");
    puts("\
Padding (amount, 94 or less), Pad-Character (ASCII value),");
    puts("\
Start-Of-Packet (ASCII value), and Timeout (94 seconds or less),");
    puts("\
all specified as decimal numbers.");
    return 0;

  case XYSERV:
    puts("server timeout:");
    puts("\
Server command wait timeout interval, how often the C-Kermit server issues");
    puts("\
a NAK while waiting for a command packet.  Specify 0 for no NAKs at all.");
    return 0;
#endif
  default:
    printf("Not available yet - %s\n", cmdbuf);
    return 0;
  }
}

/* D O H R M T -- Give help about REMOTE command */

dohrmt(xx) int xx;
{
  int x;
  if (xx == -3)
    return hmsga(hmhrmt);
  if (xx < 0)
    return xx;
  switch (xx) {
#ifndef NODOHLP
  case XZCWD:
    return hmsg("\
Ask remote Kermit server to change its working directory.");

  case XZDEL:
    return hmsg("\
Ask remote Kermit server to delete the named file(s).");

  case XZDIR:
    return hmsg("\
Ask remote Kermit server to provide directory listing of the named file(s).");

  case XZHLP:
    return hmsg("\
Ask remote Kermit server to tell you what services it provides.");

  case XZHOS:
    return hmsg("\
Send a command to the remote system in its own command language\n\
through the remote Kermit server.");

  case XZSPA:
    return hmsg("\
Ask the remote Kermit server to tell you about its disk space.");

  case XZTYP:
    return hmsg("\
Ask the remote Kermit server to type the named file(s) on your screen.");

  case XZWHO:
    return hmsg("\
Ask the remote Kermit server to list who's logged in, or to give information\n\
about the specified user.");
#endif
  default:
    if ((x = cmcfm()) < 0)
      return x;
    printf("Not working yet - %s\n", cmdbuf);
    return -2;
  }
}

/*
 * The following functions moved here
 * from ckuusr.c because that module
 * got too big for PDP-11s. 
 */

/* D O L O G -- Do the log command */

dolog(x) int x;
{
  int y;
  char *s;

  switch (x) {

  case LOGD:
#ifdef DEBUG
    y = cmofi("Name of debugging log file", "debug.log", &s);
#else
    y = -2;
    s = "";
    printf("%s", "- Sorry, debug log not available\n");
#endif
    break;

  case LOGP:
    y = cmofi("Name of packet log file", "packet.log", &s);
    break;

  case LOGS:
    y = cmofi("Name of session log file", "session.log", &s);
    break;

  case LOGT:
#ifdef TLOG
    y = cmofi("Name of transaction log file", "transact.log", &s);
#else
    y = -2;
    s = "";
    printf("%s", "- Sorry, transaction log not available\n");
#endif
    break;

  default:
    printf("\n?Unexpected log designator - %d\n", x);
    return -2;
  }
  if (y < 0)
    return y;

  strcpy(line, s);
  s = line;
  if ((y = cmcfm()) < 0)
    return y;

  switch (x) {

  case LOGD:
    return deblog = debopn(s);

  case LOGP:
    zclose(ZPFILE);
    y = zopeno(ZPFILE, s);
    if (y > 0)
      strcpy(pktfil, s);
    else
      *pktfil = '\0';
    return pktlog = y;

  case LOGS:
    zclose(ZSFILE);
    y = zopeno(ZSFILE, s);
    if (y > 0)
      strcpy(sesfil, s);
    else
      *sesfil = '\0';
    return seslog = y;

  case LOGT:
    zclose(ZTFILE);
    tralog = zopeno(ZTFILE, s);
    if (tralog > 0) {
      strcpy(trafil, s);
      tlog(F110, "Transaction Log:", versio, 0l);
      tlog(F100, ckxsys, "", 0);
      ztime(&s);
      tlog(F100, s, "", 0l);
    } else
      *trafil = '\0';
    return tralog;

  default:
    return -2;
  }
}

/* D E B O P N -- Open a debugging file */

debopn(s) char *s;
{
#ifdef DEBUG
  char *tp;

  zclose(ZDFILE);
  deblog = zopeno(ZDFILE, s);
  if (deblog > 0) {
    strcpy(debfil, s);
    debug(F110, "Debug Log ", versio, 0);
    debug(F100, ckxsys, "", 0);
    ztime(&tp);
    debug(F100, tp, "", 0);
  } else
    *debfil = '\0';
  return deblog;
#else
  return 0;
#endif
}

/* S H O P A R -- Show Parameters */

shopar()
{

  int i;
#ifndef NOCKUDIA
  extern struct keytab mdmtab[];
  extern int nmdm;
#endif

  printf("\n%s,%s, ", versio, ckxsys);
  puts("Communications Parameters:");
  printf(" Line: %s, speed: %d, mode: ", ttname, speed);
  if (local)
    printf("local");
  else
    printf("remote");

#ifndef NOCKUDIA
  for (i = 0; i < nmdm; i++) {
    if (mdmtab[i].val == mdmtyp) {
      printf(", modem-dialer: %s", mdmtab[i].kwd);
      break;
    }
  }
#endif
  printf("\n Bits: %d", (parity) ? 7 : 8);
  printf(", parity: ");
  switch (parity) {
  case 'e':
    printf("even");
    break;
  case 'o':
    printf("odd");
    break;
  case 'm':
    printf("mark");
    break;
  case 's':
    printf("space");
    break;
  case 0:
    printf("none");
    break;
  default:
    printf("invalid - %d", parity);
    break;
  }
  printf(", duplex: ");
  if (duplex)
    printf("half, ");
  else
    printf("full, ");
  printf("flow: ");
  if (flow == 1)
    printf("xon/xoff");
  else if (flow == 0)
    printf("none");
  else
    printf("%d", flow);
  printf(", handshake: ");
  if (turn)
    printf("%d\n", turnch);
  else
    printf("none\n");
  printf("Terminal emulation: %d bits\n", (cmask == 0177) ? 7 : 8);

  printf("\nProtocol Parameters:   Send    Receive");
  if (timef || spsizf)
    printf("    (* = override)");
  printf("\n Timeout:      %11d%9d", rtimo, timint);
  if (timef)
    printf("*");
  else
    printf(" ");
  printf("       Server timeout:%4d\n", srvtim);
  printf("\n Padding:      %11d%9d", npad, mypadn);
  printf("        Block Check: %6d\n", bctr);
  printf(" Pad Character:%11d%9d", padch, mypadc);
  printf("        Delay:       %6d\n", delay);
  printf(" Packet Start: %11d%9d", mystch, stchr);
  printf("        Max Retries: %6d\n", maxtry);
  printf(" Packet End:   %11d%9d", seol, eol);
  if (ebqflg)
    printf("        8th-Bit Prefix: '%c'", ebq);
  printf("\n Packet Length:%11d", spsiz);
  printf(spsizf ? "*" : " ");
  printf("%8d", urpsiz);
  printf((urpsiz > 94) ? " (94)" : "     ");
  if (rptflg)
    printf("   Repeat Prefix:  '%c'", rptq);
  printf("\n Length Limit: %11d%9d\n", maxsps, maxrps);

  printf("\nFile parameters:              Attributes:       ");
  if (atcapr)
    printf("on");
  else
    printf("off");
  printf("\n File Names:   ");
  if (fncnv)
    printf("%-12s", "converted");
  else
    printf("%-12s", "literal");
#ifdef DEBUG
  printf("   Debugging Log:    ");
  if (deblog)
    printf("%s", debfil);
  else
    printf("none");
#endif
  printf("\n File Type:    ");
  if (binary)
    printf("%-12s", "binary");
  else
    printf("%-12s", "text");
  printf("   Packet Log:       ");
  if (pktlog)
    printf("%s", pktfil);
  else
    printf("none");
  printf("\n File Warning: ");
  if (warn)
    printf("%-12s", "on");
  else
    printf("%-12s", "off");
  printf("   Session Log:      ");
  if (seslog)
    printf("%s", sesfil);
  else
    printf("none");
  printf("\n File Display: ");
  if (quiet)
    printf("%-12s", "off");
  else
    printf("%-12s", "on");
#ifdef TLOG
  printf("   Transaction Log:  ");
  if (tralog)
    printf("%s", trafil);
  else
    printf("none");
#endif
  printf("\n\nFile Byte Size: %d", (fmask == 0177) ? 7 : 8);
  printf(", Incomplete File Disposition: ");
  if (keep)
    printf("keep");
  else
    printf("discard");
#ifdef KERMRC
  printf("\n Init file: %s", KERMRC);
#endif
  puts("\n");
}

/* D O S T A T -- Display file transfer statistics */

#ifndef NOSTATS
dostat()
{
  printf("\nMost recent transaction --\n");
  printf(" Files: %ld\n", filcnt);
  printf(" Total file characters  : %ld\n", tfc);
  printf(" Communication line in  : %ld\n", tlci);
  printf(" Communication line out : %ld\n", tlco);
  printf(" Elapsed time           : %d sec\n", tsecs);
  if (filcnt > 0) {
    if (tsecs > 0) {
      long lx;
      lx = (tfc * 10l) / tsecs;
      printf(" Effective data rate    : %ld\n", lx);
      if (speed > 0) {
        lx = (lx * 100l) / speed;
        printf(" Efficiency             : %ld %%\n", lx);
      }
    }
    printf(" Packet length          : %d (Send), %d (Receive)\n", spsiz,
           urpsiz);
    printf(" Block check type used  : %d\n", bctu);
    printf(" Compression            : ");
    if (rptflg)
      printf("Yes [%c]\n", rptq);
    else
      printf("No\n");
    printf(" 8th bit prefixing      : ");
    if (ebqflg)
      printf("Yes [%c]\n", ebq);
    else
      printf("No\n\n");
  } else
    printf("\n");
  return 0;
}

/* F S T A T S -- Record file statistics in transaction log */

fstats()
{
  tfc += ffc;
  tlog(F100, " end of file", "", 0l);
  tlog(F101, "  file characters        ", "", ffc);
  tlog(F101, "  communication line in  ", "", flci);
  tlog(F101, "  communication line out ", "", flco);
}

/* T S T A T S -- Record statistics in transaction log */

tstats()
{
  char *tp;
  int x;

  ztime(&tp);                               /* Get time stamp */
  tlog(F110, "End of transaction", tp, 0l); /* Record it */

  if (filcnt < 1)
    return; /* If no files, done. */

  /*
   * If multiple files, record
   * character totals for all files
   */

  if (filcnt > 1) {
    tlog(F101, " files", "", filcnt);
    tlog(F101, " total file characters   ", "", tfc);
    tlog(F101, " communication line in   ", "", tlci);
    tlog(F101, " communication line out  ", "", tlco);
  }

  /*
   * Record timing info
   * for one or more files
   */

  tlog(F101, " elapsed time (seconds)  ", "", (long)tsecs);
  if (tsecs > 0) {
    long lx;
    lx = (tfc / tsecs) * 10;
    tlog(F101, " effective baud rate     ", "", lx);
    if (speed > 0) {
      lx = (lx * 100L) / speed;
      tlog(F101, " efficiency (percent)    ", "", lx);
    }
  }
  tlog(F100, "", "", 0L); /* Leave a blank line */
}
#endif

/* S D E B U -- Record spar results in debugging log */

#ifdef DEBUG
sdebu(len) int len;
{
  debug(F111, "spar: data", rdatap, len);
  debug(F101, " spsiz ", "", spsiz);
  debug(F101, " timint", "", timint);
  debug(F101, " npad  ", "", npad);
  debug(F101, " padch ", "", padch);
  debug(F101, " seol  ", "", seol);
  debug(F101, " ctlq  ", "", ctlq);
  debug(F101, " ebq   ", "", ebq);
  debug(F101, " ebqflg", "", ebqflg);
  debug(F101, " bctr  ", "", bctr);
  debug(F101, " rptq  ", "", rptq);
  debug(F101, " rptflg", "", rptflg);
  debug(F101, " atcapu", "", atcapu);
  debug(F101, " lpcapu", "", lpcapu);
  debug(F101, " swcapu", "", swcapu);
  debug(F101, " wslots", "", wslots);
}

/* R D E B U -- Debugging display of rpar() values */

rdebu(len) int len;
{
  debug(F111, "rpar: data", data + 1, len); /*** was rdatap ***/
  debug(F101, " rpsiz ", "", xunchar(data[1]));
  debug(F101, " rtimo ", "", rtimo);
  debug(F101, " mypadn", "", mypadn);
  debug(F101, " mypadc", "", mypadc);
  debug(F101, " eol   ", "", eol);
  debug(F101, " ctlq  ", "", ctlq);
  debug(F101, " sq    ", "", sq);
  debug(F101, " ebq   ", "", ebq);
  debug(F101, " ebqflg", "", ebqflg);
  debug(F101, " bctr  ", "", bctr);
  debug(F101, " rptq  ", "", data[9]);
  debug(F101, " rptflg", "", rptflg);
  debug(F101, " capas ", "", capas);
  debug(F101, " bits  ", "", data[capas]);
  debug(F101, " atcapu", "", atcapu);
  debug(F101, " lpcapu", "", lpcapu);
  debug(F101, " swcapu", "", swcapu);
  debug(F101, " wslots", "", wslots);
  debug(F101, " rpsiz(extended)", "", rpsiz);
}
#endif
