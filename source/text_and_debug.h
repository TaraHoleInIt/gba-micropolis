#ifndef _TEXT_AND_DEBUG_H_
#define _TEXT_AND_DEBUG_H_

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

void textPrintf( const char* format, ... );
void textPrintfCenter( int y, const char* format, ... );

void textClearScreen( void );

#endif
