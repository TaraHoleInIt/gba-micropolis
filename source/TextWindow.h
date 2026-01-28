#ifndef _TEXTWINDOW_H_
#define _TEXTWINDOW_H_

#include <vector>

#include "Window.h"
#include "text_and_debug.h"

typedef void ( *ActionProc ) ( void );

class TextWindowItem;

class TextWindow : public Window {
protected:
    const char* title;

    int width;
    int height;

    int x;
    int y;

    int fgColor;
    int bgColor;

    int selectedIndex;

    std::vector< TextWindowItem* > items;

    int countItems( void ) { return ( int ) items.size( ); }

    void selectNextItem( void );
    void selectPrevItem( void );

public:
    TextWindow( int _x, int _y, int _width, int _height, int _fgColor, int _bgColor, const char* _title );
    ~TextWindow( void ) override;

    void show( void ) override;
    void hide( void ) override;
    void tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp );

    void addItem( TextWindowItem* item );
};

class TextWindowItem {
protected:
    const char* text;

    int x;
    int y;

    int width;
    int height;

public:
    TextWindowItem( int _x, int _y, int _width, int _height, const char* _text );
    virtual ~TextWindowItem( void ) = default;

    virtual const char* getText( void ) { return text; }
    virtual bool isInteractive( void ) { return false; }
    virtual void onClick( void ) { }

    friend class TextWindow;
};

class TextWindowButton : public TextWindowItem {
private:
    ActionProc action;

public:
    TextWindowButton( int _x, int _y, int _width, int _height, const char* _text, ActionProc actionToDo );
    ~TextWindowButton( void ) override = default;

    virtual bool isInteractive( void ) override { return true; }
    virtual void onClick( void ) override { if ( action != nullptr ) action( ); }
};

#endif
