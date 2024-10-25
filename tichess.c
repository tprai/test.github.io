/******************************************************************************
*
* project name:    TI-Chess
* file name:       tichess.c
* initial date:    20/02/2000
* authors:         thomas.nussbaumer@gmx.net (coding)
*                  marcos.lopez@gmx.net      (design/graphics/beta testing)
* description:     the entry point and the special clearscreen routine
*
* $Id: tichess.c,v 1.18 2004/08/06 14:00:17 DEBROUX Lionel Exp $
*
******************************************************************************/

#include "hardware.h"   // IMPORTANT - HAVE TO BE INCLUDED BEFORE TIGCCLIB.H
#include <tigcclib.h>



#include "version.h"
#include "tichess.h"
#include "logout.h"      // contains macros for log writing to link port
#include "interrupt.h"   // own key handler stuff
#include "input.h"       // key definitions
#include "routines.h"



extern defaults_t  defaults;


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// due to a very strange behavior of TIGCC we cannot use separate object until
// yet (the globals are not initialized correctly - NOT even the constants !!)
//
// So we've to go that really silly "include all into one" way ...
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#if ENABLE_LOGGING
#include "logout.c"   // includes logging routines if necessary
#endif


//--------------------------------------------------
// declarations of the necessary "external" routines
//--------------------------------------------------
extern char default_hashmode;


/*===========================================================================*/
/* special effect clear screen                                               */
/*===========================================================================*/
STATIC_FUNC void FXClearScreen(void) {
    short height  = LCD_HEIGHT;
    short height2 = height/2;
    short width   = LCD_WIDTH;
    short width2  = width/2;
    short i;
    volatile short j;

    height--;
    width--;

    //---------------------
    // clear in y direction
    //---------------------

    for (i=0;i<height2;i++) {
        DrawLine(0,i,width,i,A_REPLACE);
        DrawLine(0,height-i,width,height-i,A_REPLACE);
        if (i) {
           DrawLine(0,i-1,width,i-1,A_REVERSE);
           DrawLine(0,height-i+1,width,height-i+1,A_REVERSE);
        }
        for (j=0;j<=2500;j++);
    }

    DrawLine(0,height2-1,width,height2-1,A_REVERSE);

    //-----------------------------
    // clear in x direction
    //-----------------------------
    for (i=0;i<=width2;i++) {
        DrawPix(i,height2,A_REVERSE);
        DrawPix(width-i,height2,A_REVERSE);
        for (j=0;j<=1500;j++);
    }
}



/*===========================================================================*/
/* due to a real strange behaviour the contrast gets lost when we set our    */
/* keyboard handler !!!                                                      */
/* I haven't managed to restore it correctly in the interrupt installer so I */
/* will do it just in the main routine ...                                   */
/*                                                                           */
/* The following routine fetches the address where the contrast will be      */
/* stored, so we can get and restore it later again                          */
/*===========================================================================*/

/* code not used anymore!! - BUT I leave it in if anyone is interested */

//unsigned char* ActiveContrastAddr(void);
//asm("ActiveContrastAddr:\n"
//    "    movem.l  %d0-%d1,-(%a7)\n"
//    "    move.w   #4,%d0\n"
//    "    trap     #9\n"
//    "    movem.l  (%a7)+,%d0-%d1\n"
//    "    rts");


/*===========================================================================*/
/* Home Screen restauration - Greg Dietsche (original HSR 3.0 - 06/19/2002)  */
/*===========================================================================*/
void HomeScreenRestore(void);
asm("HomeScreenRestore:\n"\
"     movem.l  %d0-%d7/%a0-%a6,-(%a7)\n"\
"     movea.l  0xC8,%a4\n"\
"     movea.l  (0x124,%a4),%a0\n"\
"     jsr      (%a0)               | call MenuUpdate\n"\
"     move.w   #0xA,-(%a7)\n"\
"     movea.l  (0x3AC,%a4),%a0\n"\
"     jsr      (%a0)               | call ST_refDsp with empty message\n"\
"     movea.l  (0x38C,%a4),%a0\n"\
"     jsr      (%a0)               | call ST_eraseHelp\n"\
"     move.l   #0x1E,-(%a7)\n"\
"     move.w   #0xFF,-(%a7)\n"\
"     move.l   %a4,%d0\n"\
"     andi.l   #0xE00000,%d0\n"\
"     cmpi.l   #0x800000,%d0\n"\
"     beq.s    ____ti89            | TI-89T\n"\
"     cmpi.l   #0x400000,%d0\n"\
"     beq.s    ____ti92PV200       | TI-92P\n"\
"     cmpi.l   #0x200000,%d0\n"
"     bne.s    ____other_model     | unknown model, skip.\n"
"     movea.l  (0xBC,%a4),%a0\n"\
"     cmpi.b   #0xEF,(0x2,%a0)\n"\
"     beq.s    ____ti92PV200       | V200\n"\
"____ti89:\n"\
"| TI-89/TI-89T\n"\
"     pea      0x56E6.w\n"\
"     bra.s    ____drawline\n"\
"____ti92PV200:\n"\
"| TI-92P/V200\n"\
"     pea      0x5A2E.w\n"\
"____drawline:\n"\
"    movea.l  (0x9F0,%a4),%a0\n"\
"    jsr      (%a0)                | call memset (draws bottom line)\n"\
"____other_model:\n"\
"    movea.l  (%a4),%a0            | FirstWindow\n"\
"    movea.l  (%a0),%a0            | dereference the pointer\n"\
"____dirtywindows:\n"\
"    ori.w    #0x2000,(%a0)        | set Dirty Flag\n"\
"    tst.l    (0x22,%a0)           | is there another window?\n"\
"    movea.l  (0x22,%a0),%a0       | movea does not affect ccr\n"\
"    bne.s    ____dirtywindows\n"\
"    lea      (0xC,%a7),%a7\n"\
"    movem.l  (%a7)+,%d0-%d7/%a0-%a6\n"\
"    rts");




/*===========================================================================*/
/* MAIN - where all the fun starts ...                                       */
/*===========================================================================*/
void _main(void) {
    char           tmpstr[50];
    unsigned long  size;
    //unsigned char  initial_contrast;

/*
#if !defined(USE_TI89)
    if (TI89) {
        ST_showHelp("Cannot run TI-92p version");
        return;
    }
#else
    if (!TI89) {
        ST_showHelp("Cannot run TI-89 version");
        return;
    }
#endif
*/

    //initial_contrast = *ActiveContrastAddr(); // get the initial contrast

    LoadDefaults();   // load defaults from ticcfg

    default_hashmode = defaults.hashtables;

    FXClearScreen();

#if SHOW_MEM_USAGE
    if (default_hashmode == OPTION_ON) size = sizeof(hentry_t)*HASHSIZE*2;
    else                               size = 0;

    size += sizeof(char)*BOARD_SIZE + \
            sizeof(short)*MAX_DEPTH+sizeof(move_t)*(STACK_SIZE+1) + \
            sizeof(short)*MAX_DEPTH+sizeof(killer_t)*MAX_DEPTH + \
            sizeof(bicolor_t)*BOARD_SIZE + \
            sizeof(bicolor_t)*(H_COLUMN+2)*2 + \
            sizeof(short)*MAX_DEPTH*4 + \
            sizeof(fromto_t*)*MAX_DEPTH + \
            sizeof(fromto_t)*MAX_DEPTH*MAX_DEPTH + \
            sizeof(hash_t*)*12 + \
            sizeof(hash_t)*64*12 + \
            sizeof(store_t)*MAX_MOVES_STORED + \
            60000 + \
            SIZE_OF_RESERVED_MEMORY;
#endif
    FontSetSys(F_6x8);

    if (!InitFastDraw()) {
        HomeScreenRestore();
        ST_helpMsg("FATAL: init draw failed");
        return;
    }

    if (!InitMemory()) {
        FreeMemory();

        size = sizeof(char)*BOARD_SIZE + \
           sizeof(short)*MAX_DEPTH+sizeof(move_t)*(STACK_SIZE+1) + \
           sizeof(short)*MAX_DEPTH+sizeof(killer_t)*MAX_DEPTH + \
           sizeof(bicolor_t)*BOARD_SIZE + \
           sizeof(bicolor_t)*(H_COLUMN+2)*2 + \
           sizeof(short)*MAX_DEPTH*4 + \
           sizeof(fromto_t*)*MAX_DEPTH + \
           sizeof(fromto_t)*MAX_DEPTH*MAX_DEPTH + \
           sizeof(hash_t*)*12 + \
           sizeof(hash_t)*64*12 + \
           sizeof(store_t)*MAX_MOVES_STORED + \
           60000 + \
           SIZE_OF_RESERVED_MEMORY;

        HomeScreenRestore();
        sprintf(tmpstr,"requires approx. %ld bytes free RAM",size);
        ST_helpMsg(tmpstr);
        return;
    }

#if SHOW_MEM_USAGE
    sprintf(s3,"[using %u Bytes]",size);
    FontSetSys(F_6x8);
    DrawStr(0,0,s3,A_REPLACE);
    DrawStr(0,10,"press a key",A_REPLACE);
    ngetchx();
#endif

    //----------------------------------------------------------
    // install own key handler (MUST BE DONE BEFORE GrayOn)
    //----------------------------------------------------------
    InstallInterruptHandlers();

    //---------------------------
    // turn on grayscale graphics
    //---------------------------
    FreeReservedMemory();
    if (!GrayOn()) {
        FreeMemory();
        SaveDefaults();  // just in the case hashing gets turned off
        RestoreInterruptHandlers();
        HomeScreenRestore();
        return;
    }


    //---------------------------------
    // !!!!! restore the contrast !!!!!
    //---------------------------------
    //*ActiveContrastAddr() = initial_contrast;

    OSFreeTimer(USER_TIMER); // make sure timer is not in use !!

    Logo();

    //-----------------------------------------------
    // start timer for clocks and enter the main loop
    //-----------------------------------------------
    OSRegisterTimer(USER_TIMER,20);

    MainGUI();

    //-----------------------------------------------
    // stop timer again, turn grayscales off, restore
    // old interrupt handlers again and free memory
    //-----------------------------------------------
    OSFreeTimer(USER_TIMER);
    GrayOff();
    RestoreInterruptHandlers();
    FreeMemory();

    //------------------------------------------
    // give the system keyboard handler time to
    // recover and then read out all keypresses
    //------------------------------------------
    WaitForMillis(200);
    GKeyFlush();              // remove any pending keys
    HomeScreenRestore();
}



//#############################################################################
//###################### NO MORE FAKES BEYOND THIS LINE #######################
//#############################################################################
//
//=============================================================================
// Revision History
//=============================================================================
//
// $Log: tichess.c,v $
// Revision 1.18  2004/08/06 14:00:17  DEBROUX Lionel
// (hopefully) fixed HSR support for the Titanium
//
// Revision 1.17  2002/10/17 11:47:38  tnussb
// unnecessary (??) contrast handling disabled
//
// Revision 1.16  2002/10/17 11:30:57  tnussb
// changes due to new font graphics data retrieving by Lionel Debroux
// (the sprite data is fetched from the AMS)
//
// Revision 1.15  2002/10/16 18:28:52  tnussb
// changes related to the complete new puzzle file support (see history.txt)
//
// Revision 1.14  2002/10/14 12:53:01  tnussb
// output of "memory required" changed to approx. number of total bytes needed
// (including unpacked ppg file)
//
// Revision 1.13  2002/10/11 11:07:52  tnussb
// usage of a backbuffer replaced by HomeScreenRestore function
// (thanx to Greg Dietsche for the original implementation)
//
// Revision 1.12  2002/03/01 17:29:04  tnussb
// changes due to multilanguage support
//
// Revision 1.11  2002/02/11 16:38:12  tnussb
// many changes due to "separate file compiling" restructuring
//
// Revision 1.10  2002/02/07 21:32:58  tnussb
// critical bugs if memory is running low fixed
//
// Revision 1.9  2002/02/07 11:39:45  tnussb
// changes for v3.50beta and v3.50 (see history.txt)
//
// Revision 1.8  2001/06/20 20:24:15  Thomas Nussbaumer
// size optimizations
//
// Revision 1.7  2001/02/17 15:56:10  Thomas Nussbaumer
// cannot start ti89 version on ti92p and ti92p version on ti89 to
// prevent complete "hang"
//
// Revision 1.6  2001/02/17 15:00:11  Thomas Nussbaumer
// changes due to new TIGCC version
//
// Revision 1.5  2000/12/19 13:55:29  Thomas Nussbaumer
// warnings stated by compiling with option -Wall fixed
//
// Revision 1.4  2000/12/09 15:39:35  Thomas Nussbaumer
// not using OPTIMIZE_ROM_CALLS anymore (causes crashes)
//
// Revision 1.3  2000/12/02 15:22:00  Thomas Nussbaumer
// adaptions due to new TIGCC standard library 2.22 features
//
// Revision 1.2  2000/08/12 15:31:13  Thomas Nussbaumer
// substitution keywords added
//
//
