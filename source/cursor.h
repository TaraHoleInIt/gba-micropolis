#ifndef _CURSOR_H_
#define _CURSOR_H_

#include <vector>
#include "w_micropolis.h"
#include "game_sprite.h"

class Cursor {
private:
    EditingTool tool;
    int toolSize;
    int curToolIndex;

    int x;
    int y;

    bool placeOk;

    bool canPlaceTool( void );

    void getToolIconPosition( int cursorScreenX, int cursorScreenY, int& xOffset, int& yOffset );

    EditingTool toolFromIndex( int tooli );
public:
    Cursor( void );
    
    void init( Micropolis* _sim );

    void moveTo( int newX, int newY );
    void moveBy( int dx, int dy );

    std::vector< Sprite > getSprites( void );

    int getX( void );
    int getY( void );

    bool worldToScreen( int& x, int& y );

    EditingTool getTool( void );
    void setTool( EditingTool newTool );
    void nextTool( void );
    void prevTool( void );

    void doTool( void );
};

extern Cursor cursor;

#endif
