#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_dma.h>
#include <string.h>
#include <assert.h>

#include "IWorldRenderer.h"
#include "w_micropolis.h"
#include "game_sprite.h"
#include "text_and_debug.h"
#include "cursor.h"

const OBJATTR oamClearSpritesDisabled[ 128 ] = {
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 },
	{ OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }, { OBJ_DISABLE, 0, 0 }
};

const Sprite trainSprite[ ] = {
    makeSprite16( 0, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Vertical
    makeSprite16( 2, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Horizontal
    makeSprite16( 4, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Northwest
    makeSprite16( 4, Sprite_16x16, SQUARE, 0, 1, 0 )    // Northeast
};

const Sprite heliSprite[ ] = {
    makeSprite16( 6, Sprite_32x32, SQUARE, 1, 0, 0 ),   // North
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 0, 0 ),  // Northeast
    makeSprite16( 14, Sprite_32x32, SQUARE, 1, 0, 0 ),  // East
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 0, 1 ),  // Southeast
    makeSprite16( 6, Sprite_32x32, SQUARE, 1, 0, 1 ),   // South
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 1, 1 ),  // Southwest
    makeSprite16( 14, Sprite_32x32, SQUARE, 1, 1, 0 ),  // West
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 1, 0 ),  // Northwest
};

const Sprite planeSprite[ ] = {
    makeSprite16( 18, Sprite_32x32, SQUARE, 2, 0, 0 ),  // North
    makeSprite16( 22, Sprite_32x32, SQUARE, 2, 0, 0 ),  // Northeast
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 0, 0 ),  // East
    makeSprite16( 22, Sprite_32x32, SQUARE, 2, 0, 1 ),  // Southeast
    makeSprite16( 18, Sprite_32x32, SQUARE, 2, 0, 1 ),  // South
    makeSprite16( 22, Sprite_32x32, SQUARE, 2, 1, 1 ),  // Southwest
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 1, 0 ),  // West
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 1, 0 ),  // Northwest

    // ???? Takeoff sprites??
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 1, 0 ),  // Northwest
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 0, 0 ),  // East
    makeSprite16( 26, Sprite_32x32, SQUARE, 2, 0, 0 ),  // East
};

const Sprite shipSprite[ ] = {
    makeSprite16( 128, Sprite_32x32, SQUARE, 3, 0, 0 ), // North
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 0, 0 ), // Northeast
    makeSprite16( 136, Sprite_32x32, SQUARE, 3, 0, 0 ), // East
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 0, 1 ), // Southeast
    makeSprite16( 128, Sprite_32x32, SQUARE, 3, 0, 1 ), // South
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 1, 1 ), // Southwest
    makeSprite16( 136, Sprite_32x32, SQUARE, 3, 1, 0 ), // West
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 1, 0 ), // Northwest
};

const Sprite monsterSprite[ ] = {
    makeSprite16( 140, Sprite_32x32, SQUARE, 4, 0, 0 ), // Northeast walk 0
    makeSprite16( 144, Sprite_32x32, SQUARE, 4, 0, 0 ), // Northeast walk 1
    makeSprite16( 148, Sprite_32x32, SQUARE, 4, 0, 0 ), // Northeast walk 2

    makeSprite16( 140, Sprite_32x32, SQUARE, 4, 0, 1 ), // Southeast walk 0
    makeSprite16( 144, Sprite_32x32, SQUARE, 4, 0, 1 ), // Southeast walk 1
    makeSprite16( 148, Sprite_32x32, SQUARE, 4, 0, 1 ), // Southeast walk 2

    makeSprite16( 140, Sprite_32x32, SQUARE, 4, 1, 1 ), // Southwest walk 0
    makeSprite16( 144, Sprite_32x32, SQUARE, 4, 1, 1 ), // Southwest walk 1
    makeSprite16( 148, Sprite_32x32, SQUARE, 4, 1, 1 ), // Southwest walk 2

    makeSprite16( 140, Sprite_32x32, SQUARE, 4, 1, 0 ), // Northwest walk 0
    makeSprite16( 144, Sprite_32x32, SQUARE, 4, 1, 0 ), // Northwest walk 1
    makeSprite16( 148, Sprite_32x32, SQUARE, 4, 1, 0 ), // Northwest walk 2

    makeSprite16( 152, Sprite_32x32, SQUARE, 4, 0, 0 ), // North
    makeSprite16( 156, Sprite_32x32, SQUARE, 4, 0, 0 ), // East
    makeSprite16( 152, Sprite_32x32, SQUARE, 4, 0, 1 ), // South
    makeSprite16( 156, Sprite_32x32, SQUARE, 4, 1, 0 ), // West
};

const Sprite tornadoSprite[ ] = {
    makeSprite16( 256, Sprite_32x64, TALL, 5, 0, 0 ),
    makeSprite16( 260, Sprite_32x64, TALL, 5, 0, 0 ),
    makeSprite16( 264, Sprite_32x64, TALL, 5, 0, 0 ),
};

const Sprite explosionSprite[ ] = {
    makeSprite16( 268, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 272, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 276, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 280, Sprite_32x32, SQUARE, 6, 0, 0 )
};

IWRAM_CODE Sprite setupSprite( const Sprite base, int x, int y ) {
    Sprite res = base;

    res.Priority = 2;
    res.Mosaic = 1;
    res.X = x;
    res.Y = y;

    return res;
}

IWRAM_CODE Sprite setupSprite( const Sprite base, int x, int y, int palette ) {
    Sprite res = base;

    res.Palette = palette;
    res.Priority = 2;
    res.Mosaic = 1;
    res.X = x;
    res.Y = y;

    return res;
}
