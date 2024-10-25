*******************************************************************************
*                                                                             *
* TTTTTTTTTTT  III         CCCCC   HH    HH   EEEEEEEE     SSSS       SSSS    *
* TTTTTTTTTTT  III        CCCCCCC  HH    HH   EEEEEEEE   SSSSSSSS   SSSSSSSS  *
*     TTT      III       CCC   CC  HH    HH   EE        SSS        SSS        *
*     TTT      III  XXX  CC        HHHHHHHH   EEEEEEEE   SSSSSS     SSSSSS    *
*     TTT      III  XXX  CC        HHHHHHHH   EEEEEEEE     SSSSSS     SSSSSS  *
*     TTT      III       CCC   CC  HH    HH   EE               SSS        SSS *
*     TTT      III        CCCCCCC  HH    HH   EEEEEEEE   SSSSSSSS   SSSSSSSS  *
*     TTT      III         CCCCC   HH    HH   EEEEEEEE     SSSS       SSSS    *
*                                                                             *
*                                                                             *
*                                                                             *
*                        TI-Chess for TI89/TI89T/TI92+/V200                   *
*                                                                             *
*                                     by                                      *
*                                                                             *
*                                   T I C T                                   *
*                                                                             *
*                              The TI-Chess Team                              *
*                                                                             *
*                                                                             *
*                                                                             *
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ *
* +                                                                         + *
* +              VISIT The TiCT-HQ at:  http://tict.ticalc.org              + *
* +                                                                         + *
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ *
*                                                                             *
*******************************************************************************
$Id: readme.txt,v 1.17 2004/08/06 13:58:43 DEBROUX Lionel Exp $

-------------------------------------------------------------------------------
NOTES ON THE SOURCECODE RELEASE
-------------------------------------------------------------------------------

To compile the source code you need the TIGCC development environment
(GNU C cross compiler) which can be downloaded from http://www.ticalc.org
or from the TICT-HQ (http://tict.ticalc.org). The latest version is always 
available at http://tigcc.ticalc.org.


******************************************************************************
-NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE-
******************************************************************************

The current version of TI-Chess is ONLY compatible with TI-GCC 0.95 Beta 15+.

******************************************************************************
-NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE----NOTE-
******************************************************************************



The source code of TI-CHESS is splitted into the following files:
-----------------------------------------------------------------
bg_ti89.c    ... image data and output routine of background (TI89 version)
bg_ti92.c    ... image data and output routine of background (TI92 version)
board.c      ... pieceset data and output routine for single square
build89.bat  ... builds the TI89 NOSTUB version and the launcher program
                 and copies them to the corresponding directories
build92p.bat ... builds the TI92 NOSTUB version and the launcher program
                 and copies them to the corresponding directories
clocks.c     ... sprite data and handling routines
defines.h    ... defines and structures used for the complete project
debug.c      ... debug page functionality
engine.c     ... code of the chess engine
generic.c    ... utility functions
generic.h    ... header file for utility functions
gui.c        ... most of the graphical stuff
hardware.h   ... hardware dependent settings and "global" compiler directives
hash.c       ... hashtable stuff
infoboards.c ... output routines for the info boards
input.c      ... keyboard handling routine
input.h      ... key definitions
interrupt.c  ... own key handler stuff
interrupt.h  ... own key handler stuff
loadsave.c   ... routines for loading and saving
logo.c       ... image data and output routine of splashscreen
logout.c     ... routines which may be used with the VTI Logger Tool
                 (VTI-Logger may be found on the TI-Chess Homepage)
logout.h     ... header file for the routines in logout.c
menu.c       ... most of the menu stuff
messages.h   ... multilanguage support
msg_english.h... english messages for multilanguage support
msg_french.h ... french messages for multilanguage support
msg_german.h ... german messages for multilanguage support
msg_spanish.h .. spanish messages for multilanguage support
opening.c    ... opening book support
routines.h   ... prototypes of functions which are used by other files
tichess.c    ... contains just the entry point
tichess.h    ... the project header file
ticstart.c   ... used to build special launcher ticstart which is capable
                 of starting tichess.ppg from folder TICT
train.c      ... handling of puzzle files
version.h    ... just contains the version number as precompiler define
version.h    ... just contains the version number as precompiler define
waitms.c     ... WaitForMillis() utility function


To compile this version of tichess use batchfile build89.bat or build92p.bat.
These batch files compile the program using the -pack option of tigcc
(generate exepacked program) and copies the generated files to the
corresponding directory.

That's it.


-------------------------------------------------------------------------------
OTHER USEFUL FLAGS FOR TIGCC (just as hint)
-------------------------------------------------------------------------------

-S ... generate just assembler output
-E ... generate just preprocessor output


-------------------------------------------------------------------------------
Contact
-------------------------------------------------------------------------------

You can reach me by email at: thomas.nussbaumer@gmx.net

Use our messageboard (http://pub26.ezboard.com/btichessteamhq) to post
suggestions, bug reports and similar.