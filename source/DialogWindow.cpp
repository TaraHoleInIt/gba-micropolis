#include "DialogWindow.h"

TextWindow* makeDialog( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, const char* text ) {
    TextWindow* tw = new TextWindow( x, y, width, height, fgColor, bgColor, title );

    tw->addItem( new TextWindowItem( 2, 2, width - 2, height - 2, text ) );

    return tw;
}

TextWindow* makeMenu( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, std::vector< MenuEntry > menuEntries ) {
    TextWindow* tw = new TextWindow( x, y, width, height, fgColor, bgColor, title );
    const char* text = nullptr;
    ActionProc action = nullptr;
    int yPos = 1;

    for ( MenuEntry i : menuEntries ) {
        text = std::get< 0 >( i );
        action = std::get< 1 >( i );

        tw->addItem(
            new TextWindowButton(
                1,
                yPos++,
                strlen( text ),
                1,
                text,
                action
            )
        );
    }

    return tw;
}
