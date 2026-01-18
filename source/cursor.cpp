#include <gba.h>
#include <gba_sprites.h>

#include "game_sprite.h"
#include "w_micropolis.h"
#include "cursor.h"
#include "text_and_debug.h"
#include "IWorldRenderer.h"

static const Sprite cursor8x8 = makeSprite16( 544, Sprite_8x8, SQUARE, 0, 0, 0 );

static const Sprite cursorCornerNW = makeSprite16( 512, Sprite_8x8, SQUARE, 0, 0, 0 );
static const Sprite cursorCornerNE = makeSprite16( 512, Sprite_8x8, SQUARE, 0, 1, 0 );
static const Sprite cursorCornerSW = makeSprite16( 512, Sprite_8x8, SQUARE, 0, 0, 1 );
static const Sprite cursorCornerSE = makeSprite16( 512, Sprite_8x8, SQUARE, 0, 1, 1 );

static const Sprite toolIconBulldozer = makeSprite16( 513, Sprite_32x16, WIDE, 7, 0, 0 );
static const Sprite toolIconRoad = makeSprite16( 517, Sprite_32x16, WIDE, 0, 0, 0 );
static const Sprite toolIconRail = makeSprite16( 521, Sprite_32x16, WIDE, 4, 0, 0 );
static const Sprite toolIconPark = makeSprite16( 525, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconResidential = makeSprite16( 529, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconCommercial = makeSprite16( 533, Sprite_32x16, WIDE, 9, 0, 0 );
static const Sprite toolIconIndustrial = makeSprite16( 537, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconPolice = makeSprite16( 577, Sprite_32x16, WIDE, 9, 0, 0 );
static const Sprite toolIconFirestation = makeSprite16( 581, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconStadium = makeSprite16( 585, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconPowerPlant = makeSprite16( 589, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconSeaport = makeSprite16( 593, Sprite_32x16, WIDE, 9, 0, 0 );
static const Sprite toolIconAirport = makeSprite16( 597, Sprite_32x16, WIDE, 8, 0, 0 );
static const Sprite toolIconWire = makeSprite16( 601, Sprite_32x16, WIDE, 8, 0, 0 );

static const Sprite* toolIcons[ ] = {
    &toolIconResidential,
    &toolIconCommercial,
    &toolIconIndustrial,
    &toolIconFirestation,
    &toolIconPolice,
    nullptr,    // Query has no icon
    &toolIconWire,
    &toolIconBulldozer,
    &toolIconRail,
    &toolIconRoad,
    &toolIconStadium,
    &toolIconPark,
    &toolIconSeaport,
    &toolIconPowerPlant,
    &toolIconPowerPlant,
    &toolIconAirport,
    nullptr,    // Network has no icon
    nullptr,    // Water has no icon
    nullptr,    // Land has no icon
    nullptr,    // Forest has no icon
};

extern IWorldRenderer* renderer;
Cursor cursor;

Cursor::Cursor( void ) {
    tool = EditingTool::TOOL_ROAD;
    sim = nullptr;
    x = 0;
    y = 0;
    placeOk = false;
    toolSize = 0;
}

void Cursor::init( Micropolis* _sim ) {
    assert( _sim != nullptr );

    sim = _sim;
    x = 0;
    y = 0;
    placeOk = false;
    toolSize = 0;

    setTool( TOOL_RESIDENTIAL );
}

void Cursor::moveTo( int newX, int newY ) {
    int toolOffset = ( toolSize > 1 ) ? 1 : 0;
    int maxX = ( toolSize > 1 ) ? ( WORLD_W - toolSize + 1 ) : ( WORLD_W - 1 );
    int maxY = ( toolSize > 1 ) ? ( WORLD_H - toolSize + 1 ) : ( WORLD_H - 1 );

    x = newX;
    y = newY;

    x = clamp( x, toolOffset, maxX );
    y = clamp( y, toolOffset, maxY );

    placeOk = canPlaceTool( );
}

void Cursor::moveBy( int dx, int dy ) {
    moveTo( x + dx, y + dy );
}

int Cursor::getX( void ) {
    return x;
}

int Cursor::getY( void ) {
    return y;
}

bool Cursor::worldToScreen( int& _x, int& _y ) {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    int xOffset = 0;
    int yOffset = 0;

    renderer->getViewport( left, right, top, bottom );

    _x = ( x * 8 ) - left;
    _y = ( y * 8 ) - top;

    return ( _x >= 0 ) && ( _y >= 0 ) && ( _x < 512 ) && ( _y < 256 );
}

bool Cursor::canPlaceTool( void ) {
    return sim->doTool( tool, x, y, false ) == TOOLRESULT_OK;
}

EditingTool Cursor::getTool( void ) {
    return tool;
}

void Cursor::setTool( EditingTool newTool ) {
    tool = newTool;
    toolSize = sim->getToolSize( tool );

    placeOk = canPlaceTool( );
}

void Cursor::doTool( void ) {
    sim->doTool( tool, x, y, true );
}

void testUISprite( Sprite spr, int x, int y, std::vector< Sprite >& res ) {
    Sprite newSprite = spr;

    newSprite.X = x;
    newSprite.Y = y;

    res.push_back( newSprite );
}

std::vector< Sprite > Cursor::getSprites( void ) {
    const Sprite* toolSprite = nullptr;
    std::vector< Sprite > res;
    int cursorColor = 0;
    int toolWidthTiles = 0;
    int toolHeightTiles = 0;
    int screenX = 0;
    int screenY = 0;
    int xOffset = 0;
    int yOffset = 0;

    if ( worldToScreen( screenX, screenY ) ) {
        cursorColor = placeOk ? Color_Green : Color_Red;

        if ( toolSize > 1 ) {
            screenX-= 8;
            screenY-= 8;

            toolWidthTiles = toolSize;
            toolHeightTiles = toolSize;

            res.push_back( setupSprite( cursorCornerNW, screenX, screenY, cursorColor ) );
            res.push_back( setupSprite( cursorCornerNE, screenX + ( toolWidthTiles * 8 ) - 8 - 1, screenY, cursorColor ) );
            res.push_back( setupSprite( cursorCornerSW, screenX, screenY + ( toolHeightTiles * 8 ) - 8 - 1, cursorColor ) );
            res.push_back( setupSprite( cursorCornerSE, screenX + ( toolWidthTiles * 8 ) - 8 - 1, screenY + ( toolHeightTiles * 8 ) - 8 - 1, cursorColor ) );
        } else {
            res.push_back( setupSprite( cursor8x8, screenX, screenY, cursorColor ) );
        }

        if ( ( toolSprite = toolIcons[ tool ] ) != nullptr ) {
            getToolIconPosition( screenX, screenY, xOffset, yOffset );
            res.push_back( setupSprite( *toolSprite,  xOffset, yOffset ) );
        }
    }

    return res;
}

void Cursor::getToolIconPosition( int cursorScreenX, int cursorScreenY, int& xOffset, int& yOffset ) {
    int toolWidth = ( ( ( toolSize > 1 ) ? toolSize : 1 ) * 8 );
    int toolHeight = ( ( ( toolSize > 1 ) ? toolSize : 1 ) * 8 );
    int x0 = cursorScreenX + toolWidth;
    int y0 = cursorScreenY + toolHeight;
    const int iconWidth = 24;
    const int iconHeight = 16;
    int x1 = x0 + iconWidth;
    int y1 = y0 + iconHeight;

    if ( x1 >= SCREEN_WIDTH )
        x0-= ( toolWidth + iconWidth );

    if ( y1 >= SCREEN_HEIGHT )
        y0-= ( toolHeight + iconHeight );

    xOffset = x0;
    yOffset = y0;
}

void Cursor::nextTool( void ) {
    tool = ( EditingTool ) ( ( ( int ) tool ) + 1 );

    if ( tool > TOOL_AIRPORT )
        tool = TOOL_RESIDENTIAL;

    setTool( tool );
}

void Cursor::prevTool( void ) {

}
