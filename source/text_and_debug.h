#ifndef _TEXT_AND_DEBUG_H_
#define _TEXT_AND_DEBUG_H_

#define TextConsoleWidth ( SCREEN_WIDTH / 8 )
#define TextConsoleHeight ( SCREEN_HEIGHT / 8 )

enum {
    Color_Black = 0,
    Color_Darkblue,
    Color_Darkgreen,
    Color_Teal,
    Color_Darkred,
    Color_Purple,
    Color_Brown,
    Color_Lightgray,
    Color_Darkgray,
    Color_Blue,
    Color_Green,
    Color_Cyan,
    Color_Red,
    Color_Magenta,
    Color_Yellow,
    Color_White,
    Color_Transparent
};

void textAndDebugInit( void );

void mgbaPrintf( const char* format, ... );

void textPutChar( int c );
void textSetCursor( int x, int y );
void textSetColor( int fg, int bg );
void textPuts( const char* text );

void textClearScreen( void );

void textSaveColors( void );
void textRestoreColors( void );
void textDrawLineH( int x, int y, int width, int ch );
void textDrawLineV( int x, int y, int height, int ch );
void textFill( int x, int y, int width, int height, int ch );
void textBox( int x, int y, int width, int height );

#endif
