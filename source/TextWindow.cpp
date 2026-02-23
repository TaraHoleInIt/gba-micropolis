#include <gba_input.h>
#include "TextWindow.h"
#include "Game.h"

#define InfoStr "B: BACK"

TextWindow::TextWindow( int _x, int _y, int _width, int _height, int _fgColor, int _bgColor, const char* _title ) {
    title = _title;

    width = _width;
    width = ( width < ( int ) strlen( InfoStr ) + 4 ) ? ( int ) strlen( InfoStr ) + 4 : width;
    height = _height;

    bgColor = _bgColor;
    fgColor = _fgColor;

    x = _x;
    y = _y;

    selectedIndex = -1;
}

TextWindow::~TextWindow( void ) {
    for ( TextWindowItem* item : items ) {
        assert( item != nullptr );
        delete item;
    }
}

void TextWindow::show( void ) {
    int titleLen = strlen( title );
    int dirLen = strlen( InfoStr );

    textSaveColors( );
        textSetColor( fgColor, bgColor );
        textBox( x, y, width, height );

        textSetColor( Color_Yellow, bgColor );
        textSetCursor( x + ( ( width - titleLen ) / 2 ), y );
        textPuts( title );

        textSetCursor( x + ( ( width - dirLen ) / 2 ), y + height - 1 );
        textPuts( InfoStr );
    
        textSetColor( fgColor, bgColor );

        // for ( TextWindowItem* item : items ) {
        //     textSetCursor( x + item->x, y + item->y );
        //     textPuts( item->getText( ) );
        // }

        for ( int i = 0; i < countItems( ); i++ ) {
            textSetCursor( x + items[ i ]->x, y + items[ i ]->y );

            if ( i == selectedIndex ) {
                textSaveColors( );
                    textSetColor( bgColor, fgColor );
                    textPuts( items[ i ]->getText( ) );
                textRestoreColors( );
            } else {
                textPuts( items[ i ]->getText( ) );
            }
        }

    textRestoreColors( );
}

void TextWindow::hide( void ) {
    textSaveColors( );
        textSetColor( Color_Transparent, Color_Transparent );
        textFill( x, y, width, height, ' ' );
    textRestoreColors( );
}

void TextWindow::tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp ) {
    if ( keysDown & KEY_DOWN ) {
        selectNextItem( );
        show( );
    }

    if ( keysDown & KEY_UP ) {
        selectPrevItem( );
        show( );
    }

    if ( keysDown & KEY_A ) {
        if ( selectedIndex >= 0 && selectedIndex < countItems( ) )
            items[ selectedIndex ]->onClick( );
    }
}

void TextWindow::selectNextItem( void ) {
    bool foundInteractable = false;
    int count = countItems( );
    int i = 0;

    for ( i = selectedIndex + 1; i < count; i++ ) {
        foundInteractable = items[ i ]->isInteractive( );

        if ( foundInteractable == true )
            break;
    }

    if ( foundInteractable == true )
        selectedIndex = i;
}

void TextWindow::selectPrevItem( void ) {
    bool foundInteractable = false;
    int i = 0;

    for ( i = selectedIndex - 1; i >= 0; i++ ) {
        foundInteractable = items[ i ]->isInteractive( );

        if ( foundInteractable == true )
            break;
    }

    if( foundInteractable == true )
        selectedIndex = i;
}

void TextWindow::addItem( TextWindowItem* item ) {
    assert( item != nullptr );
    items.push_back( item );

    selectedIndex = 0;
}

TextWindowItem::TextWindowItem( int _x, int _y, int _width, int _height, const char* _text ) :
    x( _x ),
    y( _y ),
    width( _width ),
    height( _height ) {
    
    strncpy( text, _text, sizeof( text ) );
}

TextWindowItem::TextWindowItem( int _x, int _y, const char* _text, ... ) {
    va_list argp;
    int len = 0;

    va_start( argp, _text );
        len = vsnprintf( text, sizeof( text ), _text, argp );
    va_end( argp );

    x = _x;
    y = _y;
    width = len;
    height = 1;
}

TextWindowButton::TextWindowButton( int _x, int _y, int _width, int _height, const char* _text, ActionProc actionToDo ) :
    TextWindowItem( _x, _y, _width, _height, _text ),
    action( actionToDo ) {
}
