#include <sys/iosupport.h>
#include <gba_console.h>
#include <gba_video.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <vector>
#include <tuple>

#include <mgba.h>

#include "text_and_debug.h"

#include "font_img_bin.h"

#define TilePalette( n ) ( ( n ) << 12 )

#define FontStartTile 224
#define FontStartChar ' '
#define FontEndChar '~'
#define FontBGChar ( FontEndChar + 1 )

#define VRAMEnd ( VRAM + 65536 )
#define MapSize ( 32 * 32 * 2 )

static void textPlaceCharAt( int x, int y, int c );
static void textInit( void );

static const devoptab_t* mgbaStdout = nullptr;

static volatile uint16_t* bg1Map = ( volatile uint16_t* ) MAP_BASE_ADR( 30 );
static volatile uint16_t* bg2Map = ( volatile uint16_t* ) MAP_BASE_ADR( 31 );

std::vector< std::tuple< int, int > > colorSave;

static int fgColor = 7;
static int bgColor = 0;

static int consoleX = 0;
static int consoleY = 0;

static const uint16_t textPalette[ 16 ] = {
    RGB8( 0x00, 0x00, 0x00 ),   // Black
    RGB8( 0x00, 0x00, 0x80 ),   // Dark blue
    RGB8( 0x00, 0x80, 0x00 ),   // Dark green
    RGB8( 0x00, 0x80, 0x80 ),   // Teal
    RGB8( 0x80, 0x00, 0x00 ),   // Dark red
    RGB8( 0x80, 0x00, 0x80 ),   // Purple
    RGB8( 0x80, 0x80, 0x00 ),   // Brown
    RGB8( 0x80, 0x80, 0x80 ),   // Light grey
    
    RGB8( 0x40, 0x40, 0x40 ),   // Dark grey
    RGB8( 0x00, 0x00, 0xFF ),   // Blue
    RGB8( 0x00, 0xFF, 0x00 ),   // Green
    RGB8( 0x00, 0xFF, 0xFF ),   // Cyan
    RGB8( 0xFF, 0x00, 0x00 ),   // Red
    RGB8( 0xFF, 0x00, 0xFF ),   // Magenta
    RGB8( 0xFF, 0xFF, 0x00 ),   // Yellow
    RGB8( 0xFF, 0xFF, 0xFF ),   // White
};

void textAndDebugInit( void ) {
    mgba_console_open( );
    mgbaStdout = devoptab_list[ STD_OUT ];

    textInit( );
}

void mgbaPrintf( const char* format, ... ) {
    char buf[ 256 ];
    va_list argp;

    va_start( argp, format );
        vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    mgba_printf( MGBA_LOG_ERROR, buf );
}

void textPrintf( const char* format, ... ) {
    char buf[ 256 ];
    va_list argp;

    va_start( argp, format );
        vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    textPuts( buf );
}

void textPrintfCenter( int y, const char* format, ... ) {
    char buf[ 256 ];
    int len = 0;
    va_list argp;

    va_start( argp, format );
        len = vsnprintf( buf, sizeof( buf ), format, argp );
    va_end( argp );

    consoleX = ( ( TextConsoleWidth / 2 ) - ( len / 2 ) ) - 1;
    consoleY = y;

    textPuts( buf );
}

static void textInit( void ) {
    int i = 0;

    dmaCopy( font_img_bin, ( void* ) ( ( VRAMEnd - ( MapSize * 3 ) ) - font_img_bin_size ), font_img_bin_size );

    REG_DISPCNT |= BG1_ENABLE | BG2_ENABLE;

    REG_BG1CNT = BG_16_COLOR | BG_SIZE_0 | CHAR_BASE( 3 ) | MAP_BASE( 30 ) | BG_PRIORITY( 0 );
    REG_BG2CNT = BG_16_COLOR | BG_SIZE_0 | CHAR_BASE( 3 ) | MAP_BASE( 31 ) | BG_PRIORITY( 1 );

    // Initialize maps to transparent tiles
    for ( i = 0; i < 32 * 32; i++ ) {
        bg1Map[ i ] = FontStartTile;
        bg2Map[ i ] = FontStartTile;
    }

    // The palette should be contained in the renderer's palette, but initialize it here anyway
    // in case a renderer has not been started yet.
    for ( i = 0; i < 16; i++ )
        BG_PALETTE[ ( i * 16 ) + 15 ] = textPalette[ i ];
}

static void textPlaceCharAt( int x, int y, int c ) {
    int mapOffset = x + ( y * 32 );

    c+= FontStartTile - FontStartChar;

    if ( fgColor == Color_Transparent )
        c = FontStartTile;

    bg1Map[ mapOffset ] = c | TilePalette( fgColor );

    if ( bgColor != Color_Transparent )
        bg2Map[ mapOffset ] = ( FontStartTile + FontBGChar - FontStartChar ) | TilePalette( bgColor );
    else
        bg2Map[ mapOffset ] = ( FontStartTile );
}

void textPutChar( int c ) {
    switch ( c ) {
        case '\r': {
            consoleX = 0;
            break;
        }
        case '\n': {
            consoleX = 0;
            consoleY++;

            break;
        }
        default: {
            textPlaceCharAt( consoleX, consoleY, c );

            consoleX++;
            break;
        }
    };

    if ( consoleX >= TextConsoleWidth ) {
        consoleX = 0;
        consoleY++;
    }

    consoleY = ( consoleY >= TextConsoleHeight ) ? TextConsoleHeight - 1 : consoleY;
}

void textSetCursor( int x, int y ) {
    consoleX = x;
    consoleY = y;

    consoleX = ( consoleX < 0 ) ? 0 : consoleX;
    consoleY = ( consoleY < 0 ) ? 0 : consoleY;

    consoleX = ( consoleX >= TextConsoleWidth ) ? TextConsoleWidth - 1 : consoleX;
    consoleY = ( consoleY >= TextConsoleHeight ) ? TextConsoleHeight - 1 : consoleY;
}

void textSetColor( int fg, int bg ) {
    fgColor = fg;
    bgColor = bg;
}

void textPuts( const char* text ) {
    while ( *text )
        textPutChar( *text++ );
}

void textClearScreen( void ) {
    textSaveColors( );
        fgColor = Color_Transparent;
        bgColor = Color_Transparent;

        for ( int y = 0; y < TextConsoleHeight; y++ ) {
            for ( int x = 0; x < TextConsoleWidth; x++ )
                textPlaceCharAt( x, y, 0 );
        }
    textRestoreColors( );

    consoleX = 0;
    consoleY = 0;
}

void textSaveColors( void ) {
    colorSave.push_back( { fgColor, bgColor } );
}

void textRestoreColors( void ) {
    std::tuple< int, int > sv;

    if ( colorSave.empty( ) )
        return;

    sv = colorSave.back( );
    colorSave.pop_back( );

    fgColor = std::get< 0 >( sv );
    bgColor = std::get< 1 >( sv );
}

void textDrawLineH( int x, int y, int width, int ch ) {
    for ( int i = 0; i < width; i++ )
        textPlaceCharAt( x + i, y, ch );
}

void textDrawLineV( int x, int y, int height, int ch ) {
    for ( int i = 0; i < height; i++ )
        textPlaceCharAt( x, y + i, ch );
}

void textFill( int x, int y, int width, int height, int ch ) {
    for ( int i = 0; i < height; i++ )
        textDrawLineH( x, y + i, width, ch );
}

void textBox( int x, int y, int width, int height ) {
    // Fill with blank space
    textFill( x, y, width, height, ' ' );

    // Top
    textPlaceCharAt( x, y, '+' );
    textDrawLineH( x + 1, y, width - 2, '-' );
    textPlaceCharAt( x + width - 1, y, '+' );

    // Bottom
    textPlaceCharAt( x, y + height - 1, '+' );
    textDrawLineH( x + 1, y + height - 1, width - 2, '-' );
    textPlaceCharAt( x + width - 1, y + height - 1, '+' );

    // Left
    textDrawLineV( x, y + 1, height - 2, '|' );

    // Right
    textDrawLineV( x + width - 1, y + 1, height - 2, '|' );
}
