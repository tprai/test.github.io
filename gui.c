/******************************************************************************
*
* project name:    TI-Chess
* file name:       gui.c
* initial date:    24/02/2000
* authors:         thomas.nussbaumer@gmx.net (coding)
*                  marcos.lopez@gmx.net      (design/graphics/beta testing)
* description:     contains most of the GUI stuff
*
* $Id: gui.c,v 1.22 2004/08/06 13:51:09 DEBROUX Lionel Exp $
*
******************************************************************************/

#include "hardware.h"   // MUST BE ALWAYS HERE ON THE FIRST LINE !!!!
#include <string.h>
#include <graph.h>
#include <gray.h>
#include <setjmp.h>
#include "tichess.h"
#include "input.h"
#include "generic.h"
#include "routines.h"

#if ENABLE_TESTKEY
#include "debug.c"
#endif

STATIC_FUNC void ResetDisplays(void);
STATIC_FUNC void DrawMarkingRect(short idx);
STATIC_FUNC short Cursor2Idx(short x,short y);
STATIC_FUNC void DrawBoard(short hidecursor);
STATIC_FUNC void InitGUI(void);
STATIC_FUNC void HandleComputerMove(void);
STATIC_FUNC void MajorInputLoop(void);

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
SCR_RECT squarerect     = {};           // temporary rectangle
short    inverted       = FALSE;
short    select_x       =  0;           // selected square (x coordinate)
short    select_y       =  0;           // selected square (y coordinate)
short    marked_rect1   = -1;
short    marked_rect2   = -1;
short    idx_from       = -1;           // index into board of from-square
short    idx_to         = -1;           // index into board of to-square
short    select_promote = 0;            // used to store selected promotion fig
short    automatic_mode = FALSE;        // automatic mode setting
short    twoplayer_mode = FALSE;        // two player mode setting
JMP_BUF  jmp_buffer     = {};           // longjump for new and exit functions
char     do_exit        = 0;            // for exit() handling
char     drawn[BOARD_SIZE] = {};        // used to update board display
short    choice = MENU_NONE;


//-----------------------------------------------------------------------------
// declarations of the necessary "external" routines and variables
//-----------------------------------------------------------------------------
extern char*          board;      // the board
extern short          act_color;  // which turn (white or black)
extern short          std_depth;  // standard depth
extern unsigned short move_count; // number of moves so far
extern long           nr_nodes;
extern short          white_king_pos;
extern short          black_king_pos;
extern hash_t         act_hashkey;
extern short          already_mate;
extern store_t*       move_store;
extern short          moves_stored;
extern short          moves_forward;
extern defaults_t     defaults;


/*===========================================================================*/
/* restart the clocks and initialize infoboards                              */
/*===========================================================================*/
STATIC_FUNC void ResetDisplays(void) {
    ToggleClocks(1,inverted);
    if (act_color==WHITE) {
        InitInfo(act_color,-1,std_depth,inverted);
    }
    else {
        InitInfo(act_color,std_depth,-1,inverted);
        ToggleClocks(0,inverted);
    }
}



/*===========================================================================*/
/* draws marking rounded rectangle arround given square (last computer move) */
/*===========================================================================*/
STATIC_FUNC void DrawMarkingRect(short idx) {
    short      x;
    short      y;
    short      x0,x1,y0,y1;

    if (inverted) {
        y = idx / 10 - 2;
        x = 7 - (idx % 10 - 1);
    }
    else {
        y = 7 - (idx / 10 - 2);
        x = idx % 10 - 1;
    }

    x0 = OFFSET_X+SIZE_X*x;
    x1 = OFFSET_X+SIZE_X*(x+1)-1;
    y0 = OFFSET_Y+SIZE_Y*y;
    y1 = OFFSET_Y+SIZE_Y*(y+1)-1;

    DrawColorLine(x0+1,y0,  x1-1,y0,  COLOR_BLACK);
    DrawColorLine(x0+1,y1,  x1-1,y1,  COLOR_BLACK);
    DrawColorLine(x0,  y0+1,x0,  y1-1,COLOR_BLACK);
    DrawColorLine(x1,  y0+1,x1,  y1-1,COLOR_BLACK);
}



/*===========================================================================*/
/* maps cursor position to field value                                       */
/*===========================================================================*/
STATIC_FUNC short Cursor2Idx(short x,short y) {
    if (inverted) return 21 + y*10+(7-x);
    else          return 21 + (7-y)*10+x;
}



/*===========================================================================*/
/* draws complete board                                                      */
/*===========================================================================*/
STATIC_FUNC void DrawBoard(short hidecursor) {
    short i,j,idx;
    char val;

    // check all squares for redraw ...
    for (i=0;i<8;i++) {
        for (j=0;j<8;j++) {
            idx = Cursor2Idx(i,j);
            val = board[idx];
            if (drawn[idx] == val) {
                if (i!=select_x || j!=select_y) continue;
            }

            DrawSquare(i,j,val);

            drawn[idx] = val;

            if (idx == marked_rect1) {
               DrawMarkingRect(idx);
               drawn[idx]   = -OUTSIDE;
               marked_rect1 = -1;
            }
            else if (idx == marked_rect2) {
               DrawMarkingRect(idx);
               drawn[idx]   = -OUTSIDE;
               marked_rect2 = -1;
            }

            if (idx == idx_from) DrawMarkingRect(idx);

        }
    }

    //---------------------------------------------------
    // draw the cursor if wanted
    //---------------------------------------------------
    if (!hidecursor) {
        squarerect.xy.x0 = OFFSET_X+SIZE_X*select_x;
        squarerect.xy.x1 = OFFSET_X+SIZE_X*(select_x+1)-1;
        squarerect.xy.y0 = OFFSET_Y+SIZE_Y*select_y;
        squarerect.xy.y1 = OFFSET_Y+SIZE_Y*(select_y+1)-1;
        for (i=0;i<2;i++) {
            SetPlane(i);
            ScrRectFill(&squarerect,&fullscreen,A_XOR);
        }
    }
}



/*===========================================================================*/
/* initializes GUI                                                           */
/*===========================================================================*/
STATIC_FUNC inline void InitGUI(void) {
    short i;

    select_x     =  0;
    select_y     =  7;
    idx_from     = -1;
    idx_to       = -1;
    marked_rect1 = -1;
    marked_rect2 = -1;

    FontSetSys(F_4x6);

    ClearInfoBoardData();
    OutputBackground();

    for (i=0;i<BOARD_SIZE;i++) drawn[i] = -OUTSIDE;

    InitHashCode();
}


/*===========================================================================*/
/* forces a request dialog to pop up                                         */
/*===========================================================================*/
short ForceRequestDialog(const char* request) {
    unsigned char def = defaults.requests;
    short retval;

    defaults.requests = OPTION_ON;   // force REQUEST display
    retval = RequestDialog(request);
    defaults.requests = def;         // restore old requests settings
    return retval;
}


/*===========================================================================*/
/* handles a request dialog (returns TRUE if 'y' is pressed)                 */
/*===========================================================================*/
short RequestDialog(const char* request) {
     if (defaults.requests == OPTION_OFF) {
         return TRUE;
     }
     else {
         short read;
         short sy;

         LCD_BUFFER LCD_tmp1;
         LCD_BUFFER LCD_tmp2;

         memcpy(LCD_tmp1,GetPlane(LIGHT_PLANE),LCD_SIZE);
         memcpy(LCD_tmp2,GetPlane(DARK_PLANE), LCD_SIZE);

         sy = STATIC_LCD_HEIGHT/2 - 2;

         DrawPopup(4,sy-10,STATIC_LCD_WIDTH-5,sy+10);
         DrawString(0,sy-3,request,F_6x8,A_REPLACE | A_CENTERED | A_SHADOWED);

         do {
             read = GetUserInput(0);
         }
         while (read != KEY_ESCAPE && read != KEY_ENTER && read != KEY_EXIT);

         memcpy(GetPlane(LIGHT_PLANE),LCD_tmp1,LCD_SIZE);
         memcpy(GetPlane(DARK_PLANE), LCD_tmp2,LCD_SIZE);

         if (read == KEY_ENTER) return TRUE;
         else                   return FALSE;
    }
}



/*===========================================================================*/
/* simple exit() implementation which does a long jump back into MainGUI()   */
/*===========================================================================*/
void LeavePrg(const char* msg) {
    if (msg) RequestDialog(msg);
    do_exit = 1;
    longjmp(jmp_buffer,1);
}



/*===========================================================================*/
/* corrects informations of the info board                                   */
/*===========================================================================*/
void CorrectInfoBoard(void) {
    if (twoplayer_mode) {
        InitInfo(act_color,-1,-1,inverted);
    }
    else {
        if (automatic_mode) {
            InitInfo(act_color,std_depth,std_depth,inverted);
        }
        else {
            if (act_color==WHITE) InitInfo(act_color,-1,std_depth,inverted);
            else                  InitInfo(act_color,std_depth,-1,inverted);
        }
    }
}



/*===========================================================================*/
/* handles a computer move                                                   */
/*===========================================================================*/
STATIC_FUNC void HandleComputerMove(void) {
    move_t *executed;

    if (defaults.usebooks == OPTION_ON) {
        char* bookmove = GetOpening();
        if (bookmove) {
            short ret = UserMove(bookmove,1,std_depth);
            MisuseNodeDisplay(GetOpeningDesc());
            if (!ret) { // SHOULD NEVER HAPPEN!!
                ForceRequestDialog(MSG_INVALIDBOOKMOVE);
                automatic_mode = FALSE; // turn automatic off (otherwise we
                                        // will hang)
            }
            else {
                if (!already_mate) ToggleClocks(0,inverted);
            }
            return;
        }
    }

    SetActiveInfo(act_color,std_depth,TRUE);
    executed = ComputerMove();
    UpdateNodeDisplay(nr_nodes);

    if (executed) {
        marked_rect1 = executed->to;
        marked_rect2 = executed->from;
        if (!already_mate) {
            if (act_color == WHITE) {
                if (AttacksField(white_king_pos,BLACK)) OutputSpecial(MSG_CHECK,"");
            }
            else {
                if (AttacksField(black_king_pos,WHITE)) OutputSpecial(MSG_CHECK,"");
            }
        }

        if (automatic_mode) SetActiveInfo(act_color,std_depth,TRUE);
        else                SetActiveInfo(act_color,-1,TRUE);
        if (!already_mate) ToggleClocks(0,inverted);
    }
    else {
        automatic_mode = FALSE;
        ClearActiveMoveAndNodeInfoBoardData();
        CorrectInfoBoard();
    }

}



/*===========================================================================*/
/* nomen omen est                                                            */
/*===========================================================================*/
STATIC_FUNC void MajorInputLoop(void) {
    short  read = 0;
    short  tmpfig;
    short  i;
    char   tmpnotation[10];

    do {
        DrawBoard(automatic_mode);

#if 0
        // JUST A SIMPLE TEST FOR A POSSIBLE EDITOR FEATURE

        DrawFigure(C89_92(104,154),C89_92(19,27),B_KING);
        DrawFigure(C89_92(104,154),C89_92(32,40),B_QUEEN);
        DrawFigure(C89_92(124,174),C89_92(19,27),B_ROOK);
        DrawFigure(C89_92(124,174),C89_92(32,40),B_KNIGHT);
        DrawFigure(C89_92(144,194),C89_92(19,27),B_BISHOP);
        DrawFigure(C89_92(144,194),C89_92(32,40),B_PAWN);

        DrawFigure(C89_92(104,154),C89_92(64,72),W_KING);
        DrawFigure(C89_92(104,154),C89_92(77,85),W_QUEEN);
        DrawFigure(C89_92(124,174),C89_92(64,72),W_ROOK);
        DrawFigure(C89_92(124,174),C89_92(77,85),W_KNIGHT);
        DrawFigure(C89_92(144,194),C89_92(64,72),W_BISHOP);
        DrawFigure(C89_92(144,194),C89_92(77,85),W_PAWN);
#endif

        if (choice == MENU_NONE) {
            if (!automatic_mode) {
                read = GetUserInput(1);
                automatic_mode = FALSE;
            }
            else {
                read = KeyPressed();
                if (read == KEY_EXIT) return;
                if (read) {
                    GetUserInput(0); // remove pressed key ...
                    if (read == KEY_ESCAPE && RequestDialog(MSG_ABORT)) {
                        automatic_mode = FALSE;
                        ClearActiveMoveAndNodeInfoBoardData();
                        CorrectInfoBoard();
                        continue;
                    }
                }

                // automatic mode goes here ...
                HandleComputerMove();
                continue;
            }
        }
        else {
            switch(choice) {
                case MENU_NEW:
                    longjmp(jmp_buffer,1);
                    break;
                case MENU_TRAIN:
                    LoadActivePuzzle();
                    ResetDisplays();
                    choice = MENU_NONE;
                    continue;
                case MENU_CMD_LOAD:
                    HandleLoad(FALSE);
                    ResetDisplays();
                    choice = MENU_NONE;
                    continue;
                case MENU_CMD_SAVE:
                    HandleSave(FALSE);
                    choice = MENU_NONE;
                    continue;
                case MENU_CMD_EXPORT:
                    HandleExport();
                    choice = MENU_NONE;
                    continue;
                case MENU_EXIT:
                    return;
                default:
                    choice = MENU_NONE;
                    continue;
            }
        }

        switch(read) {

            #if ENABLE_TESTKEY
            //--------------------------------------------------------
            // NOTE: this key ('=') should be only enabled for testing
            //--------------------------------------------------------
            case KEY_TESTING:
                DebugMenu();
                break;
            #endif

            case KEY_OPENINGMOVES:
                ShowContinuationList();
                break;

            case KEY_EXIT:
                LeavePrg(0);
                break;

            case KEY_ESCAPE:
                if (idx_from != -1) {
                    drawn[idx_from] = -OUTSIDE;
                    idx_from        = -1;
                }
                else {
                    choice = MainMenu(MENU_NONE);
                    for (i=0;i<BOARD_SIZE;i++) drawn[i] = -OUTSIDE;
                    DrawBoard(FALSE);
                    DrawInfoBoard(0);
                    DrawInfoBoard(1);
                }
                break;

            case KEY_MLEFT:
                drawn[Cursor2Idx(select_x,select_y)] = -OUTSIDE;
                select_x--;
                if (select_x < 0) select_x = 7;
                break;

            case KEY_MRIGHT:
                drawn[Cursor2Idx(select_x,select_y)] = -OUTSIDE;
                select_x++;
                if (select_x > 7) select_x = 0;
                break;

            case KEY_MDOWN:
                drawn[Cursor2Idx(select_x,select_y)] = -OUTSIDE;
                select_y++;
                if (select_y > 7) select_y = 0;
                break;

            case KEY_MUP:
                drawn[Cursor2Idx(select_x,select_y)] = -OUTSIDE;
                select_y--;
                if (select_y < 0) select_y = 7;
                break;

            case KEY_ENTER:
                if (already_mate) break;

                //----------------------------------------
                // we haven't marked a piece until now ...
                //----------------------------------------
                if (idx_from == -1) {
                    i      = Cursor2Idx(select_x,select_y);
                    tmpfig = board[i];
                    //------------------------------------
                    // check if we can mark this piece ...
                    //------------------------------------
                    if (tmpfig == EMPTY ||
                       (act_color==WHITE && tmpfig<0) ||
                       (act_color==BLACK && tmpfig>0)) break;

                    //--------------------------------------
                    // mark it and schedule it for redrawing
                    //--------------------------------------
                    idx_from        =   i;
                    drawn[idx_from] = -OUTSIDE;
                }
                else {
                    i = Cursor2Idx(select_x,select_y);
                    //---------------------------------------
                    // user want us to deselect the piece ...
                    //---------------------------------------
                    if (i == idx_from) {
                        idx_from = -1;
                        drawn[i] = -OUTSIDE;
                        break;
                    }

                    //---------------------------------------
                    // user want to finish its move
                    // check if move is allowed
                    //---------------------------------------
                    tmpfig = board[i];

                    // check if user wants to select another figure
                    if ((act_color==WHITE && tmpfig>0) ||
                        (act_color==BLACK && tmpfig<0))
                    {
                        drawn[idx_from] = -OUTSIDE;
                        idx_from        = i;
                        drawn[idx_from] = -OUTSIDE;
                        break;
                    }

                    //----------------------------------
                    // move seems to be valid. Now check
                    // if its a pawn promotion move...
                    // 4.01 introduces a partial fix of a
                    // bug found by Lionel Debroux and
                    // fixed by Kevin Kofler.
                    //----------------------------------
                    idx_to = i;
                    tmpfig = board[idx_from];
                    if ((act_color==WHITE && tmpfig==W_PAWN && idx_from >= A7 && idx_to >= A8) ||
                        (act_color==BLACK && tmpfig==B_PAWN && idx_from <= H2 && idx_to <= H1))
                    {
                        select_promote = PromotionDialog();
                        // if exit key pressed -> leave processing immediately
                        if (KeyPressed() == KEY_EXIT) break;
                    }
                    else {
                       select_promote=0;
                    }

                    //-------------------------------------------------
                    // generate notation for move and try to execute it
                    //-------------------------------------------------
                    Field2Notation(idx_from,tmpnotation);
                    Field2Notation(idx_to,&tmpnotation[2]);
                    tmpnotation[4]  = select_promote;
                    tmpnotation[5] = 0;

                    if (!UserMove(tmpnotation,TRUE,HUMAN_LEVEL)) {
                        RequestDialog(MSG_INVALIDMOVE);
                    }
                    else {
                        drawn[idx_from] = -OUTSIDE;
                        idx_from        = -1;
                        if (!already_mate) {
                            ToggleClocks(0,inverted);
                            if (!twoplayer_mode) {
                                DrawBoard(TRUE);
                                HandleComputerMove();
                            }
                            else {
                                SetActiveInfo(act_color,-1,TRUE);
                            }
                        }
                    }
                }
                break;

            case KEY_HELP:
                MainMenu(MENU_HELP);
                break;

            case KEY_SHOWMOVES:
                MainMenu(MENU_SHOWMOVES);
                break;

            case KEY_AUTOMATIC:
                if (!automatic_mode) {
                   if (RequestDialog(MSG_STARTAUTOMATIC)) {
                      automatic_mode = TRUE;
                      twoplayer_mode = FALSE;
                      if (idx_from != -1) {
                          drawn[idx_from] = -OUTSIDE;
                          idx_from        = -1;
                          DrawBoard(FALSE);
                      }
                   }
                }
                else {
                   automatic_mode = FALSE;
                }
                CorrectInfoBoard();
                break;

            case KEY_LOADSAVE:
                choice = MainMenu(MENU_LOADSAVE);
                break;

            case KEY_BACK:
                // unselect a previously selected piece
                if (idx_from != -1) {
                    drawn[idx_from] = -OUTSIDE;
                    idx_from        = -1;
                }

                ClearInfoBoardData();
                TakeBackMove(FALSE);
                already_mate = FALSE;
                break;

            case KEY_FORWARD:
                // unselect a previously selected piece
                if (idx_from != -1) {
                    drawn[idx_from] = -OUTSIDE;
                    idx_from        = -1;
                }
                UndoTakeBackMove(FALSE);
                break;

            case KEY_CHANGESIDE:
                if (!twoplayer_mode) { // cannot change sides in two player mode
                    if (RequestDialog(MSG_CHANGESIDES)) {
                        // unselect a previously selected piece
                        if (idx_from != -1) {
                            drawn[idx_from] = -OUTSIDE;
                            idx_from        = -1;
                        }
                        DrawBoard(FALSE);
                        // to get a refresh of the info board
                        if (act_color == WHITE) SetActiveInfo(BLACK,-1,TRUE);
                        else                    SetActiveInfo(WHITE,-1,TRUE);
                        HandleComputerMove();
                    }
                }
                break;


            case KEY_LEVELUP:
                std_depth++;
                if (std_depth > MAX_USERLEVEL) std_depth = MAX_USERLEVEL;
                defaults.strength = std_depth-1;
                CorrectInfoBoard();
                break;


            case KEY_LEVELDOWN:
                std_depth--;
                if (std_depth < MIN_USERLEVEL) std_depth = MIN_USERLEVEL;
                defaults.strength = std_depth-1;
                CorrectInfoBoard();
                break;


            case KEY_TWOPLAYER:
                if (twoplayer_mode) {
                    if (RequestDialog(MSG_ONEPLAYER)) {
                        twoplayer_mode = FALSE;
                    }
                }
                else {
                    if (RequestDialog(MSG_TWOPLAYER)) {
                        twoplayer_mode = TRUE;
                    }
                }
                CorrectInfoBoard();
                break;

            case KEY_ROTATE:
                if (inverted) inverted = FALSE;
                else          inverted = TRUE;

                for (i=0;i<BOARD_SIZE;i++) drawn[i] = -OUTSIDE;
#if !defined(USE_TI89)
                // on the TI92 we can draw coordinates
                DrawCoordinates(inverted);
#endif
                DrawBoard(FALSE);
                SwitchClockPositions();
                SwitchInfoBoards();
                break;
        }
    }
    while (1);
}



/*===========================================================================*/
/* main routine of the GUI                                                   */
/*===========================================================================*/
void MainGUI(void) {
    do_exit = 0;
    choice  = MENU_NONE;

    //---------------------------------------------------
    // here we could add program-first-time-used-features
    //---------------------------------------------------
    if (defaults.firsttime == OPTION_ON) {
        defaults.firsttime = OPTION_OFF;
        SaveDefaults();
    }

    InitBooks();
    InitPuzzles();

    setjmp(jmp_buffer);

    if (do_exit == 0) {
        InitGUI();
        automatic_mode = FALSE;
        twoplayer_mode = FALSE;
        InitEngine(defaults.strength+1);

        if (choice != MENU_NEW && defaults.autoload == OPTION_ON) HandleLoad(TRUE);

        if (choice == MENU_NEW) choice = MENU_NONE;

#if !defined(USE_TI89)
        // on the TI92 we can draw coordinates
        DrawCoordinates(inverted);
#endif

        ResetDisplays();
        MajorInputLoop();
    }

    if (defaults.autosave == OPTION_ON) HandleSave(TRUE);
    SaveDefaults();

    CleanupBooks();
    CleanupPuzzles();
}



//#############################################################################
//###################### NO MORE FAKES BEYOND THIS LINE #######################
//#############################################################################
//
//=============================================================================
// Revision History
//=============================================================================
//
// $Log: gui.c,v $
// Revision 1.22  2004/08/06 13:51:09  DEBROUX Lionel
// generic commit
//
// Revision 1.21  2002/10/28 09:13:23  tnussb
// various cursor/selection related topics fixed
//
// Revision 1.20  2002/10/21 12:19:48  tnussb
// see changes for v3.99b in history.txt
//
// Revision 1.19  2002/10/17 09:56:41  tnussb
// generic commit for v3.97
//
// Revision 1.18  2002/10/16 20:40:54  tnussb
// just minor code beautifying
//
// Revision 1.17  2002/10/16 18:28:51  tnussb
// changes related to the complete new puzzle file support (see history.txt)
//
// Revision 1.16  2002/10/14 12:10:15  tnussb
// treatment of KEY_OPENINGMOVES added
//
// Revision 1.15  2002/10/14 08:07:27  tnussb
// "not toggle clocks" bug fixed when using opening books
//
// Revision 1.14  2002/10/11 11:14:44  tnussb
// replaced all "defined(USE_TI92P)" with "!defined(USE_TI89)"
//
// Revision 1.13  2002/10/10 13:19:30  tnussb
// changes related to new chess puzzle format
//
// Revision 1.12  2002/10/08 17:44:29  tnussb
// changes related to v3.90/v3.91
//
// Revision 1.11  2002/09/13 15:25:03  tnussb
// changes for v3.81 BETA (external opening book support)
//
// Revision 1.10  2002/09/13 10:20:00  tnussb
// changes related to opening book support (3.80 Beta)
//
// Revision 1.9  2002/03/01 17:29:03  tnussb
// changes due to multilanguage support
//
// Revision 1.8  2002/02/11 16:38:11  tnussb
// many changes due to "separate file compiling" restructuring
//
// Revision 1.7  2002/02/07 11:39:45  tnussb
// changes for v3.50beta and v3.50 (see history.txt)
//
// Revision 1.6  2001/06/20 20:24:15  Thomas Nussbaumer
// size optimizations
//
// Revision 1.5  2001/02/17 15:00:11  Thomas Nussbaumer
// changes due to new TIGCC version
//
// Revision 1.4  2000/12/19 13:55:29  Thomas Nussbaumer
// warnings stated by compiling with option -Wall fixed
//
// Revision 1.3  2000/12/02 15:12:53  Thomas Nussbaumer
// changes due to global plane1/2 renaming
//
// Revision 1.2  2000/08/12 15:31:12  Thomas Nussbaumer
// substitution keywords added
//
//
