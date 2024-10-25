/******************************************************************************
*
* project name:    TI-Chess
* file name:       menu.c
* initial date:    19/03/2000
* authors:         thomas.nussbaumer@gmx.net (coding)
*                  marcos.lopez@gmx.net      (design/graphics/beta testing)
* description:     contains the menu system
*
* $Id: menu.c,v 1.23 2004/08/06 13:56:10 DEBROUX Lionel Exp $
*
******************************************************************************/


#include "hardware.h"   // MUST BE ALWAYS HERE ON THE FIRST LINE !!!!
#include <version.h>
#include <graph.h>
#include <gray.h>
#include <string.h>
#include "version.h"
#include "input.h"
#include "generic.h"
#include "routines.h"


#include "opening.c"

STATIC_FUNC void  InvertOptionEntry(short index);
STATIC_FUNC void  OptionMenu(void);
STATIC_FUNC void  InvertLoadSaveEntry(short index);
STATIC_FUNC short LoadSaveMenu(void);
STATIC_FUNC void  InvertMainEntry(short index);
STATIC_FUNC void  DrawMainMenu(void);
STATIC_FUNC void  HelpMenu(void);
STATIC_FUNC void  ShowMovesMenu(void);
STATIC_FUNC void  AboutMenu(void);
STATIC_FUNC short TrainMenu(void);


extern infoboard_t    infos[2];
extern short          active_board;
extern unsigned short move_count;
extern short          moves_stored;
extern store_t*       move_store;

#define M_HEIGHT2      44


//-----------------------------------------------------------------------------
// calculator depended constants
//-----------------------------------------------------------------------------
#define M_TSTART_Y   C89_92(  7,  17)
#define M_CENTERX    C89_92( 80, 120)
#define M_CENTERY    C89_92( 47,  57)
#define M_WIDTH      C89_92(140, 140)
#define M_WIDTH2     C89_92( 70,  70)
#define M_CSTART_X   C89_92( 13,  53)
#define M_CEND_X     C89_92(147, 187)
#define M_PARAMS1X   C89_92( 15,  55)
#define M_PARAMS2X   C89_92( 80, 120)
#define HP_START1X   C89_92( 15,  55)
#define HP_START2X   C89_92( 70, 110)
#define HP_DELTAY    C89_92(  7,   7)


defaults_t defaults = {0,0,0,0,0,0,0,0,0,0,0,1};  // make -Wall happy ... =;-)

extern short  std_depth;          // standard depth


extern unsigned short puzzle_active;
extern unsigned short puzzle_max;

extern char puzzle_text[8][SIZE_PUZZLETEXT+1];


/*===========================================================================*/
/* draws menu area with border                                               */
/*===========================================================================*/
void InitCenterMenu(void) {
    DrawPopup(M_CENTERX-M_WIDTH2-1,M_CENTERY-M_HEIGHT2-1,M_CENTERX+M_WIDTH2+1,M_CENTERY+M_HEIGHT2+8);
}


short  main_idx      = 0;
short  loadsave_idx  = 0;

#if defined (USE_TI89)
const short  main_coords[] = {MAIN_MENU_COORDS_TI89}; //{0,15, 19,55,  59,77,  81,105, 109,124, 126,145, 147,159};
#else
const short  main_coords[] = {MAIN_MENU_COORDS_TI92P}; //{0,15, 31,67, 83,101, 117,141, 157,172, 188,207, 223,236};
#endif



/*===========================================================================*/
/* inverts an entry of the loadsave pull down menu                           */
/*===========================================================================*/
STATIC_FUNC void InvertOptionEntry(short index) {
    short x = OFFSET_X-2+3+main_coords[3*2];
    short y = OFFSET_Y+11+index*7;
    InvertGrayRect(x,y,x+OPTION_MENU_DIALOGWIDTH-6,y+6);
}


// TODO: optimize this array !
const char* options[8] = {
    OPTION_MENU_STRENGTH,
    OPTION_MENU_AUTOLOAD,
    OPTION_MENU_AUTOSAVE,
    OPTION_MENU_HASING,
    OPTION_MENU_REQUESTS,
    OPTION_MENU_SAVEMVS,
    OPTION_MENU_PIECESET,
    OPTION_MENU_USEBOOK
};



/*===========================================================================*/
/* handles the option menu                                                   */
/*===========================================================================*/
STATIC_FUNC void OptionMenu(void) {
    short              startx1     = OFFSET_X-2+main_coords[3*2];
    short              startx2     = startx1 + OPTION_MENU_TEXTSTART2;
    short              y           = OFFSET_Y+8;
    static const char* state[2]    = {OPTION_MENU_ON, OPTION_MENU_OFF};
    static char        level[3]    = {'L','1',0};
    const char*        s;
    const char*        s2;
    short              i;
    short              read;
    static const char* pieceset[2] = {"S1","S2"};

    DrawPopup(startx1,y,startx1+OPTION_MENU_DIALOGWIDTH,y+47+7+7);

    y+=4;
    startx1+=3;

    for (i=0;i<8;i++) {
        DrawString(startx1,y,options[i],F_4x6,A_REPLACE);

        switch(i) {
           case 0:  level[1] = '1'+defaults.strength;
                    s = level;
                    break;
           case 1:  s = state[defaults.autoload];
                    break;
           case 2:  s = state[defaults.autosave];
                    break;
           case 3:  s = state[defaults.hashtables];
                    break;
           case 4:  s = state[defaults.requests];
                    break;
           case 5:  s = state[defaults.savemoves];
                    break;
           case 6:  s = pieceset[defaults.pieceset];
                    break;
           default: s = state[defaults.usebooks];
                    break;
        }
        DrawString(startx2,y,s,F_4x6,A_REPLACE);
        y+=7;
    }

    InvertOptionEntry(0);
    i = 0;

    do {
        if ((read = GetUserInput(0))==KEY_ESCAPE) break;

        switch(read) {
            case KEY_MUP:
                InvertOptionEntry(i);
                if (i==0) {
                    read = KEY_ESCAPE;
                    break;
                }
                i--;
                InvertOptionEntry(i);
                break;
            case KEY_MDOWN:
                if (i==7) break;
                InvertOptionEntry(i);
                i++;
                InvertOptionEntry(i);
                break;
            case KEY_MLEFT:  // fall through
            case KEY_MRIGHT:
                if (i==0) {
                    if (read == KEY_MLEFT) {
                        if (defaults.strength > 0) defaults.strength--;
                        else                       defaults.strength = 4;
                    }
                    else {
                        if (defaults.strength < 4) defaults.strength++;
                        else                       defaults.strength = 0;
                    }
                    y = OFFSET_Y+12+i*7;
                    DrawString(startx2,y,level,F_4x6,A_NORMAL);
                    level[1] = '1'+(char)defaults.strength;
                    DrawString(startx2,y,level,F_4x6,A_REVERSE);
                }
                else {
                    if (i==1) {
                        s2 = state[defaults.autoload];
                        if (defaults.autoload) defaults.autoload = 0;
                        else                   defaults.autoload = 1;
                        s = state[defaults.autoload];
                    }
                    else if (i==2) {
                        s2 = state[defaults.autosave];
                        if (defaults.autosave) defaults.autosave = 0;
                        else                   defaults.autosave = 1;
                        s = state[defaults.autosave];
                    }
                    else if (i==3) {
                        s2 = state[defaults.hashtables];
                        if (defaults.hashtables) defaults.hashtables = 0;
                        else                     defaults.hashtables = 1;
                        s = state[defaults.hashtables];
                    }
                    else if (i==4) {
                        s2 = state[defaults.requests];
                        if (defaults.requests) defaults.requests = 0;
                        else                   defaults.requests = 1;
                        s = state[defaults.requests];
                    }
                    else if (i==5) {
                        s2 = state[defaults.savemoves];
                        if (defaults.savemoves) defaults.savemoves = 0;
                        else                    defaults.savemoves = 1;
                        s = state[defaults.savemoves];
                    }
                    else if (i==6) {
                        s2 = pieceset[defaults.pieceset];
                        if (defaults.pieceset) defaults.pieceset = 0;
                        else                   defaults.pieceset = 1;
                        s = pieceset[defaults.pieceset];
                    }
                    else {
                        //i == 7
                        s2 = state[defaults.usebooks];
                        if (defaults.usebooks) defaults.usebooks = 0;
                        else                   defaults.usebooks = 1;
                        s = state[defaults.usebooks];
                    }
                    y = OFFSET_Y+12+i*7;
                    DrawString(startx2,y,s2,F_4x6,A_NORMAL);
                    DrawString(startx2,y,s,F_4x6,A_REVERSE);
                }
                break;
        }
    }
    while (read != KEY_ESCAPE && read != KEY_EXIT);

    std_depth = defaults.strength+1;

    CorrectInfoBoard();
}



/*===========================================================================*/
/* inverts an entry of the loadsave pull down menu                           */
/*===========================================================================*/
STATIC_FUNC void InvertLoadSaveEntry(short index) {
    short x = OFFSET_X-2+3+main_coords[2];
    short y = OFFSET_Y+11+index*7;
    InvertGrayRect(x,y,x+LS_MENU_DIALOGWIDTH-6,y+6);
}



/*===========================================================================*/
/* handles the load save menu                                                */
/*===========================================================================*/
STATIC_FUNC short LoadSaveMenu(void) {
    static char s1[]   = LS_MENU_LOAD;
    static char s2[]   = LS_MENU_SAVE;
    short       startx = OFFSET_X-2+main_coords[2];
    short       y      = OFFSET_Y+8;
    short       i;
    short       read;

    DrawPopup(startx,y,startx+LS_MENU_DIALOGWIDTH,y+54+7);

    y+=4;
    startx+=3;
    for (i=0;i<4;i++) {
        s1[LS_MENU_LOAD_IDX] = '0'+i;
        DrawString(startx,y,s1,F_4x6,A_REPLACE);
        y+=7;
    }

    for (i=0;i<3;i++) {
        s2[LS_MENU_SAVE_IDX] = '1'+i;
        DrawString(startx,y,s2,F_4x6,A_REPLACE);
        y+=7;
    }

    DrawString(startx,y,LS_MENU_EXPORT,F_4x6,A_REPLACE);

    InvertLoadSaveEntry(0);
    i = 0;

    do {
        read = GetUserInput(0);

        switch(read) {
            case KEY_MUP:
                InvertLoadSaveEntry(i);
                if (i==0) return FALSE;
                i--;
                InvertLoadSaveEntry(i);
                break;
            case KEY_MDOWN:
                if (i==7) break;
                InvertLoadSaveEntry(i);
                i++;
                InvertLoadSaveEntry(i);
                break;
            case KEY_ENTER:
                loadsave_idx = i;
                return TRUE;
        }
    }
    while(read != KEY_ESCAPE && read != KEY_EXIT);
    return FALSE;
}



/*===========================================================================*/
/* inverts an entry of the main menu                                         */
/*===========================================================================*/
STATIC_FUNC void InvertMainEntry(short index) {
    InvertGrayRect(OFFSET_X-2+main_coords[index*2],
                   OFFSET_Y+1,
                   OFFSET_X-2+main_coords[index*2+1],
                   OFFSET_Y+1+6);
}



/*===========================================================================*/
/* show main menu                                                            */
/*===========================================================================*/
STATIC_FUNC void DrawMainMenu(void) {
    short x0 = 0;
    short x1 = STATIC_LCD_WIDTH-1;

    DrawColorLine(x0,0,x1,0,COLOR_BLACK);
    DrawColorLine(x0,1,x1,1,COLOR_DARKGRAY);
    DrawColorLine(x0,2,x1,2,COLOR_LIGHTGRAY);
    DrawColorRect(x0,3,x1,3+6,COLOR_WHITE,RECT_FILLED);
#if defined(USE_TI89)
    FastDraw(x0,4,MAIN_MENU_TI89);
#else
    FastDraw(x0,4,MAIN_MENU_TI92P);
#endif
    DrawColorLine(x0,10,x1,10,COLOR_LIGHTGRAY);
    DrawColorLine(x0,11,x1,11,COLOR_DARKGRAY);
    DrawColorLine(x0,12,x1,12,COLOR_BLACK);
    InvertMainEntry(main_idx);
}



/*===========================================================================*/
/* show a menu                                                               */
/*===========================================================================*/
short MainMenu(short showmenu) {
    short      read        = 0;
    short      end_of_loop = FALSE;
    short      redraw_menu = TRUE;
    short      retval      = MENU_NONE;
    LCD_BUFFER LCD_tmp1;
    LCD_BUFFER LCD_tmp2;

    memcpy(LCD_tmp1,GetPlane(LIGHT_PLANE),LCD_SIZE);
    memcpy(LCD_tmp2,GetPlane(DARK_PLANE), LCD_SIZE);

    if (showmenu != MENU_NONE) {
        switch(showmenu) {
            case MENU_LOADSAVE:
                if (LoadSaveMenu()) {
                    if (loadsave_idx < 4)      retval = MENU_CMD_LOAD;
                    else if (loadsave_idx < 7) retval = MENU_CMD_SAVE;
                    else                       retval = MENU_CMD_EXPORT;
                }
                else {
                    retval = MENU_NONE;
                }
                break;
            case MENU_HELP:
                HelpMenu();
                retval = MENU_NONE;
                break;
            case MENU_SHOWMOVES:
                ShowMovesMenu();
                retval = MENU_NONE;
                break;
        }
    }
    else {
        do {
            if (redraw_menu) {
                memcpy(GetPlane(LIGHT_PLANE),LCD_tmp2,LCD_SIZE);
                memset(GetPlane(DARK_PLANE),0,LCD_SIZE);
                DrawMainMenu();
                redraw_menu = FALSE;
            }
            read = GetUserInput(0);
            switch(read) {
                case KEY_MLEFT:
                    InvertMainEntry(main_idx);
                    main_idx--;
                    if (main_idx<0) main_idx=6;
                    InvertMainEntry(main_idx);
                    break;
                case KEY_MRIGHT:
                    InvertMainEntry(main_idx);
                    main_idx++;
                    if (main_idx>6) main_idx=0;
                    InvertMainEntry(main_idx);
                    break;
                case KEY_EXIT:
                    retval = MENU_EXIT;
                    end_of_loop = TRUE;
                    break;
                case KEY_ENTER:
                case KEY_MDOWN:
                    switch(main_idx) {
                        case MENU_SETTINGS:
                            OptionMenu();
                            redraw_menu = TRUE;
                            break;
                        case MENU_LOADSAVE:
                            InvertMainEntry(main_idx);
                            if (LoadSaveMenu()) {
                                if (loadsave_idx < 4)      retval = MENU_CMD_LOAD;
                                else if (loadsave_idx < 7) retval = MENU_CMD_SAVE;
                                else                       retval = MENU_CMD_EXPORT;
                                end_of_loop = TRUE;
                            }
                            else {
                                redraw_menu = TRUE;
                            }
                            InvertMainEntry(main_idx);
                            break;
                        case MENU_TRAIN:
                            if (TrainMenu()) {
                                retval = MENU_TRAIN;
                                end_of_loop = TRUE;
                            }
                            else {
                                redraw_menu = TRUE;
                            }
                            break;
                        case MENU_ABOUT:
                            AboutMenu();
                            redraw_menu = TRUE;
                            break;
                        case MENU_HELP:
                            HelpMenu();
                            redraw_menu = TRUE;
                            break;
                        default:
                            retval = main_idx;
                            end_of_loop = TRUE;
                            break;
                    }
                    break;
            }

        }
        while(read != KEY_ESCAPE && !end_of_loop && read != KEY_EXIT);
    }

    memcpy(GetPlane(LIGHT_PLANE),LCD_tmp1,LCD_SIZE);
    memcpy(GetPlane(DARK_PLANE), LCD_tmp2,LCD_SIZE);


    if (read == KEY_ESCAPE) return MENU_NONE;

    return retval;
}



#if !defined(USE_TI89)

//--------------------------------------------
// help page for TI92 version
//--------------------------------------------
static const char* helppage[] = { HELP_PAGE_TI92P };

#else

//--------------------------------------------
// help page for TI89 version
//--------------------------------------------
static const char* helppage[] = { HELP_PAGE_TI89 };

#endif



/*===========================================================================*/
/* handles help menu                                                         */
/*===========================================================================*/
STATIC_FUNC void HelpMenu(void) {
    short i=0;

    InitCenterMenu();

    do {
        FastDraw(HP_START1X,i/2*HP_DELTAY+M_TSTART_Y-1,(char*)helppage[i]);
        FastDraw(HP_START2X,i/2*HP_DELTAY+M_TSTART_Y-1,(char*)helppage[i+1]);
        i+=2;
    }
    while (helppage[i] != 0);
    GetUserInput(0);
}



#define LINES_PER_PAGE 13

/*===========================================================================*/
/* lists all stored moves so far                                             */
/*===========================================================================*/
STATIC_FUNC void ShowMovesMenu(void) {
    short i  = 0;
    short mv = move_count-moves_stored;
    char  s[50];
    short counter=0;
    short inputkey;

    InitCenterMenu();

    if (moves_stored == 0) {
        DrawString(HP_START1X,M_TSTART_Y-1,MOVELIST_NOMOVES,F_4x6,A_REPLACE | A_SHADOWED);
        GetUserInput(0);
        return;
    }

    if (mv % 2 == 1) {
        sprintf(s,"[%03d]",mv+1);
        FastDraw(HP_START1X,counter*HP_DELTAY+M_TSTART_Y-1,s);

        sprintf(s,"%s",Move2Str((move_t*)&move_store[0],FALSE));
        FastDraw(HP_START2X,counter*HP_DELTAY+M_TSTART_Y-1,s);
        i = 1;
        counter++;
    }

    for (;i<moves_stored;i+=2) {
        if (!counter) InitCenterMenu();
        sprintf(s,"[%03d] %s",mv+i+1,Move2Str((move_t*)&move_store[i],FALSE));
        FastDraw(HP_START1X,counter*HP_DELTAY+M_TSTART_Y-1,s);
        counter++;

        if (i+1 < moves_stored) {
             sprintf(s,"%s",Move2Str((move_t*)&move_store[i+1],FALSE));
             FastDraw(HP_START2X,(counter-1)*HP_DELTAY+M_TSTART_Y-1,s);
        }

        if ((counter) % LINES_PER_PAGE == LINES_PER_PAGE-1) {
            if (i<(moves_stored-2)) {
                DrawString(HP_START1X,(LINES_PER_PAGE-1)*HP_DELTAY+M_TSTART_Y-1,
                           MOVELIST_CONTINUE,F_4x6,A_REPLACE | A_SHADOWED);
                inputkey = GetUserInput(0);
                if (inputkey == KEY_ESCAPE || inputkey == KEY_EXIT) return;
            }
            counter = 0;
        }
    }

    DrawString(HP_START1X,(LINES_PER_PAGE-1)*HP_DELTAY+M_TSTART_Y-1,
               MOVELIST_ENDOFLIST,F_4x6,A_REPLACE | A_SHADOWED);
    GetUserInput(0);
}



/*===========================================================================*/
/* handles about menu                                                        */
/*===========================================================================*/
STATIC_FUNC void AboutMenu(void) {
    InitCenterMenu();
    FastDrawCentered(M_TSTART_Y+12,ABOUT_STR2);
    FastDrawCentered(M_TSTART_Y+35,ABOUT_STR4);
    FastDrawCentered(M_TSTART_Y+45,ABOUT_STR5);
    FastDrawCentered(M_TSTART_Y+63,ABOUT_STR7);
    FastDrawCentered(M_TSTART_Y+76,"TI-Chess v"TIC_VERSION_COMPLETE" "__DATE__" "__TIME__);
    FastDrawCentered(M_TSTART_Y+82,"("__TIGCC_VERSION_STRING__"/"__TIGCCLIB_VERSION_STRING__")");

    DrawString(0,M_TSTART_Y+2, ABOUT_STR1,F_4x6,A_REPLACE | A_CENTERED | A_SHADOWED);
    DrawString(0,M_TSTART_Y+25,ABOUT_STR3,F_6x8,A_REPLACE | A_CENTERED | A_SHADOWED);
    DrawString(0,M_TSTART_Y+53,ABOUT_STR6,F_6x8,A_REPLACE | A_CENTERED | A_SHADOWED);

    GetUserInput(0);
}



/*===========================================================================*/
/* handles train menu                                                       */
/*===========================================================================*/
STATIC_FUNC short TrainMenu(void) {
    short read,i;
    short show_solution = 0;
    short redraw = TRUE;
    char  tmpbuf[100];

    do {
        if (redraw) {
            InitCenterMenu();
            LoadPuzzleDesc();

            if (puzzle_max == 0) {
                FastDrawCentered(M_TSTART_Y,TRAIN_MENU_NOPUZZLES);
                GetUserInput(0);
                return FALSE;
            }

            // draw counter (inverted)
            sprintf(tmpbuf,"%d/%d",puzzle_active,puzzle_max);
            DrawString(0,M_TSTART_Y,tmpbuf,F_6x8,A_REPLACE | A_SHADOWED | A_CENTERED);
            InvertGrayRect(M_CSTART_X,M_TSTART_Y-2,M_CEND_X,10+M_TSTART_Y-2);

            // generic statement
            for (i=0;i<3;i++) {
                FastDrawCentered(12+i*8+M_TSTART_Y,puzzle_text[i]);
            }

            // draw the "TURN:WHITE/BLACK" line (inverted)
            DrawString(0,39+M_TSTART_Y,puzzle_text[3],F_6x8,A_REPLACE | A_CENTERED);
            InvertGrayRect(M_CSTART_X,M_TSTART_Y-2+39,M_CEND_X,10+39+M_TSTART_Y-2);

            // solution text (max. 4 lines)
            if (show_solution) {
                for (i=0;i<4;i++) {
                    FastDrawCentered(52+i*7+M_TSTART_Y,puzzle_text[4+i]);
                }
            }

            // usage info (inverted)
#if defined(USE_TI89)
            DrawString(0,83+M_TSTART_Y,"\x15\x16=-+1   \x17\x18=-+25   F1=!!!!!",F_4x6,A_REPLACE | A_CENTERED);
#else
            DrawString(0,83+M_TSTART_Y,"\x15\x16=-+1   \x17\x18=-+25   F2=!!!!!",F_4x6,A_REPLACE | A_CENTERED);
#endif
            InvertGrayRect(M_CSTART_X,M_TSTART_Y-2+84,M_CEND_X,6+84+M_TSTART_Y-2);

            redraw = FALSE;
        }
        read = GetUserInput(0);

        switch(read) {
            case KEY_MRIGHT:
                puzzle_active++;
                if (puzzle_active > puzzle_max) {
                    puzzle_active = 1;
                }
                show_solution = 0;
                redraw = TRUE;
                break;
            case KEY_MDOWN:
                puzzle_active+=25;
                if (puzzle_active > puzzle_max) {
                    puzzle_active = 1;
                }
                show_solution = 0;
                redraw = TRUE;
                break;
            case KEY_MLEFT:
                puzzle_active--;
                if (puzzle_active < 1) {
                    puzzle_active = puzzle_max;
                }
                show_solution = 0;
                redraw = TRUE;
                break;
            case KEY_MUP:
                if (puzzle_active < 26) puzzle_active = puzzle_max;
                else                    puzzle_active-=25;
                show_solution = 0;
                redraw = TRUE;
                break;
            case KEY_HELP:
                if (show_solution) {
                    show_solution = 0;
                }
                else {
                    show_solution = 1;
                }
                redraw = TRUE;
                break;
        }
    }
    while (read != KEY_ENTER && read != KEY_ESCAPE && read != KEY_EXIT);

    if (read == KEY_ENTER) return TRUE;
    else                   return FALSE;
}



/*===========================================================================*/
/* handles a the promotion dialog                                            */
/*===========================================================================*/
short PromotionDialog(void) {
    short read;
    short x;
    short y;
    short i=0;

    LCD_BUFFER LCD_tmp1;
    LCD_BUFFER LCD_tmp2;

    memcpy(LCD_tmp1,GetPlane(LIGHT_PLANE),LCD_SIZE);
    memcpy(LCD_tmp2,GetPlane(DARK_PLANE), LCD_SIZE);

    x = infos[active_board].x-2;
    y = infos[active_board].y-2;


    DrawPopup(x,y,x+57,y+34);
    x+=9;
    y+=6;
    DrawString(x+PROMOTE_CORRECTX,y,MSG_PROMOTETO,F_4x6,A_REPLACE);
    y+=11;
    x-=4;

#if !defined(USE_TI89)
    // quick hack to re-align sprite position for TI92p version
    x--;
    y--;
#endif
    if (active_board) {
        // black
        DrawFigure(x,y,B_QUEEN);
        DrawFigure(x+12,y,B_ROOK);
        DrawFigure(x+24,y,B_KNIGHT);
        DrawFigure(x+36,y,B_BISHOP);
    }
    else {
        // white
        DrawFigure(x,y,W_QUEEN);
        DrawFigure(x+12,y,W_ROOK);
        DrawFigure(x+24,y,W_KNIGHT);
        DrawFigure(x+36,y,W_BISHOP);
    }
#if !defined(USE_TI89)
    // quick hack to re-align sprite position for TI92p version
    x++;
    y++;
#endif

    InvertGrayRect(x,y,x+11,y+11);

    do {
        read = GetUserInput(0);
        switch(read) {
            case KEY_MLEFT:
                InvertGrayRect(x+i*12,y,x+i*12+11,y+11);
                i--;
                if (i<0) i=3;
                InvertGrayRect(x+i*12,y,x+i*12+11,y+11);
                break;
            case KEY_MRIGHT:
                InvertGrayRect(x+i*12,y,x+i*12+11,y+11);
                i++;
                if (i>3) i=0;
                InvertGrayRect(x+i*12,y,x+i*12+11,y+11);
                break;
        }
    }
    while (read != KEY_ENTER && read != KEY_EXIT);

    memcpy(GetPlane(LIGHT_PLANE),LCD_tmp1,LCD_SIZE);
    memcpy(GetPlane(DARK_PLANE), LCD_tmp2,LCD_SIZE);

    switch(i) {
        case 1:  return 'R';
        case 2:  return 'N';
        case 3:  return 'B';
        default: return 'Q';
    }
}



//#############################################################################
//###################### NO MORE FAKES BEYOND THIS LINE #######################
//#############################################################################
//
//=============================================================================
// Revision History
//=============================================================================
//
// $Log: menu.c,v $
// Revision 1.23  2004/08/06 13:56:10  DEBROUX Lionel
// generic commit
//
// Revision 1.22  2002/10/23 15:03:00  tnussb
// very strange behaviour in option menu fixed: previously on AMS 1.xx versions
// (even on VTI) you couldn't switch the pieceset easily. I have checked the
// code, but the code was ok and it also worked correctly on AMS 2.xx. So I
// believe in a AMS bug, but I still don't understand where and why exactly.
//
// Revision 1.21  2002/10/22 08:41:15  tnussb
// About Dialog: display now TI-Chess version
//
// Revision 1.20  2002/10/21 12:19:48  tnussb
// see changes for v3.99b in history.txt
//
// Revision 1.19  2002/10/18 08:53:19  tnussb
// (1) code size reductions
// (2) bug in paging through puzzles fixed (now no longer negative numbers
// can be occur)
//
// Revision 1.18  2002/10/16 20:42:43  tnussb
// solution can now be toggled in train menu. pressing a second time F1
// will hide the solution again.
//
// Revision 1.17  2002/10/16 18:28:51  tnussb
// changes related to the complete new puzzle file support (see history.txt)
//
// Revision 1.16  2002/10/14 12:50:24  tnussb
// replace various calls to DrawString() with FastDraw()
//
// Revision 1.15  2002/10/11 11:14:44  tnussb
// replaced all "defined(USE_TI92P)" with "!defined(USE_TI89)"
//
// Revision 1.14  2002/10/10 13:19:30  tnussb
// changes related to new chess puzzle format
//
// Revision 1.13  2002/10/08 17:44:29  tnussb
// changes related to v3.90/v3.91
//
// Revision 1.12  2002/09/13 16:17:27  tnussb
// exporting now InitCenterMenu()
//
// Revision 1.11  2002/03/01 17:29:04  tnussb
// changes due to multilanguage support
//
// Revision 1.10  2002/02/11 16:38:12  tnussb
// many changes due to "separate file compiling" restructuring
//
// Revision 1.9  2002/02/07 11:39:45  tnussb
// changes for v3.50beta and v3.50 (see history.txt)
//
// Revision 1.8  2001/06/20 20:24:15  Thomas Nussbaumer
// size optimizations
//
// Revision 1.7  2001/02/17 15:55:27  Thomas Nussbaumer
// about dialog modified (build time and date added)
//
// Revision 1.6  2001/02/17 15:00:11  Thomas Nussbaumer
// changes due to new TIGCC version
//
// Revision 1.5  2000/12/19 13:55:29  Thomas Nussbaumer
// warnings stated by compiling with option -Wall fixed
//
// Revision 1.4  2000/12/09 15:38:18  Thomas Nussbaumer
// grayscale adjustment keys F2/F3 added to help text
//
// Revision 1.3  2000/12/02 15:12:53  Thomas Nussbaumer
// changes due to global plane1/2 renaming
//
// Revision 1.2  2000/08/12 15:31:13  Thomas Nussbaumer
// substitution keywords added
//
//
