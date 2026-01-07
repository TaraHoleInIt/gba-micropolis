#include <sys/iosupport.h>
#include <gba_console.h>
#include <gba_video.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <mgba.h>

#include "config.h"

#define ConsoleWidth ( SCREEN_WIDTH / 8 )
#define ConsoleHeight ( SCREEN_HEIGHT / 8 )

static void textInit( void );

static const devoptab_t* mgbaStdout = nullptr;
static const devoptab_t* consoleStdout = nullptr;

void textAndDebugInit( void ) {
    mgba_console_open( );
    mgbaStdout = devoptab_list[ STD_OUT ];

    //textInit( );
    //consoleStdout = devoptab_list[ STD_OUT ];
}

void mgbaPrintf( const char* format, ... ) {
    char buf[ 256 ];
    int len = 0;
    va_list argp;

    va_start( argp, format );
        len = vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    if ( mgbaStdout != nullptr )
        mgbaStdout->write_r( _REENT, nullptr, buf, len + 1 );
}

void textPrintf( const char* format, ... ) {
    char buf[ 256 ];
    int len = 0;
    va_list argp;

    va_start( argp, format );
        len = vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    if ( consoleStdout != nullptr )
        consoleStdout->write_r( _REENT, nullptr, buf, len + 1 );
}

void textPrintfCenter( int y, const char* format, ... ) {
    char buf[ 256 ];
    int len = 0;
    va_list argp;
    int x = 0;

    va_start( argp, format );
        len = vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    if ( consoleStdout != nullptr ) {
        x = ( ConsoleWidth / 2 ) - ( len / 2 );

        textPrintf( "\x1b[%d;%dH", y, x );
        consoleStdout->write_r( _REENT, nullptr, buf, len + 1 );
    }
}

static void textInit( void ) {
    consoleInit(
        ConfigConsoleCharBase,
        ConfigConsoleMapBase,
        ConfigConsoleBG,
        nullptr,
        0,
        15
    );

    REG_DISPCNT |= MODE_0 | ConsoleBG_Enable;

    BG_PALETTE[ ( 15 * 16 ) + 0 ] = 0;
    BG_PALETTE[ ( 15 * 16 ) + 1 ] = RGB5( 0, 31, 31 );

    BGCTRL[ ConfigConsoleBG ] |= BG_PRIORITY( 1 );
}
