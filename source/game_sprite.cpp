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

static constexpr inline Sprite makeSprite16( uint16_t tileId, uint16_t size, uint16_t shape, uint16_t palette, uint16_t hFlip, uint16_t vFlip );
static std::vector< Sprite > getGameSprites( IWorldRenderer* renderer, Micropolis* sim );
static Sprite setupSprite( const Sprite base, int x, int y );
static void spriteClearAll( void );

static const OBJATTR oamClearSpritesDisabled[ 128 ] = {
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

static const Sprite trainSprite[ ] = {
    makeSprite16( 0, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Vertical
    makeSprite16( 2, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Horizontal
    makeSprite16( 4, Sprite_16x16, SQUARE, 0, 0, 0 ),   // Northwest
    makeSprite16( 4, Sprite_16x16, SQUARE, 0, 1, 0 )    // Northeast
};

static const Sprite heliSprite[ ] = {
    makeSprite16( 0, Sprite_32x32, SQUARE, 1, 0, 0 ),   // North
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 0, 0 ),  // Northeast
    makeSprite16( 14, Sprite_32x32, SQUARE, 1, 0, 0 ),  // East
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 0, 1 ),  // Southeast
    makeSprite16( 6, Sprite_32x32, SQUARE, 1, 0, 1 ),   // South
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 1, 1 ),  // Southwest
    makeSprite16( 14, Sprite_32x32, SQUARE, 1, 1, 0 ),  // West
    makeSprite16( 10, Sprite_32x32, SQUARE, 1, 1, 0 ),  // Northwest
};

static const Sprite planeSprite[ ] = {
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

static const Sprite shipSprite[ ] = {
    makeSprite16( 128, Sprite_32x32, SQUARE, 3, 0, 0 ), // North
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 0, 0 ), // Northeast
    makeSprite16( 136, Sprite_32x32, SQUARE, 3, 0, 0 ), // East
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 0, 1 ), // Southeast
    makeSprite16( 128, Sprite_32x32, SQUARE, 3, 0, 1 ), // South
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 1, 1 ), // Southwest
    makeSprite16( 136, Sprite_32x32, SQUARE, 3, 1, 0 ), // West
    makeSprite16( 132, Sprite_32x32, SQUARE, 3, 1, 0 ), // Northwest
};

static const Sprite monsterSprite[ ] = {
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

static const Sprite tornadoSprite[ ] = {
    makeSprite16( 256, Sprite_32x64, TALL, 5, 0, 0 ),
    makeSprite16( 260, Sprite_32x64, TALL, 5, 0, 0 ),
    makeSprite16( 264, Sprite_32x64, TALL, 5, 0, 0 ),
};

static const Sprite explosionSprite[ ] = {
    makeSprite16( 268, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 272, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 276, Sprite_32x32, SQUARE, 6, 0, 0 ),
    makeSprite16( 280, Sprite_32x32, SQUARE, 6, 0, 0 )
};

static Sprite oamShadow[ 128 ];

static constexpr inline Sprite makeSprite16( uint16_t tileId, uint16_t size, uint16_t shape, uint16_t palette, uint16_t hFlip, uint16_t vFlip ) {
    Sprite res = {
        // attr 0
        .Y = 0,
        .RotationScaling = 0,
        .Mode = 0,
        .Mosaic = 0,
        .ColorMode = OBJ_16_COLOR,
        .Shape = shape,

        // attr 1
        .X = 0,
        .NotUsed = 0,
        .HFlip = hFlip,
        .VFlip = vFlip,
        .Size = size,

        // attr 2
        .Character = tileId,
        .Priority = 0,
        .Palette = palette,

        // dummy
        .dummy = 0
    };

    return res;
}

IWRAM_CODE static Sprite setupSprite( const Sprite base, int x, int y ) {
    Sprite res = base;

    res.X = x;
    res.Y = y;

    return res;
}

IWRAM_CODE static std::vector< Sprite > getGameSprites( IWorldRenderer* renderer, Micropolis* sim ) {
    std::vector< Sprite > res;
    Sprite newSprite;
    int left = 0;
    int top = 0;
    int bottom = 0;
    int right = 0;
    int sprX0 = 0;
    int sprY0 = 0;
    int sprX1 = 0;
    int sprY1 = 0;

    renderer->getViewport( left, right, top, bottom );

    for ( SimSprite* s = sim->spriteList; s != nullptr; s = s->next ) {
        sprX0 = ( ( s->x + s->xOffset ) / 2 ) - left;
        sprY0 = ( ( s->y + s->yOffset ) / 2 ) - top;

        sprX1 = sprX0 + ( s->width / 2 );
        sprY1 = sprY0 + ( s->height / 2 );

        if ( sprX1 >= 0 && sprX0 < SCREEN_WIDTH && sprY1 >= 0 && sprY0 < SCREEN_HEIGHT && s->frame > 0 ) {
            memcpy( &newSprite, &oamClearSpritesDisabled[ 0 ], sizeof( Sprite ) );

            switch ( s->type ) {
                case SPRITE_TRAIN:
                    newSprite = setupSprite( trainSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_HELICOPTER:
                    newSprite = setupSprite( heliSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_AIRPLANE:
                    newSprite = setupSprite( planeSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_SHIP:
                    newSprite = setupSprite( shipSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_MONSTER:
                    newSprite = setupSprite( monsterSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_TORNADO:
                    newSprite = setupSprite( tornadoSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_EXPLOSION:
                    newSprite = setupSprite( explosionSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                default:
                    break;
            };

            res.push_back( newSprite );
        }
    }

    return res;
}

IWRAM_CODE static void spriteClearAll( void ) {
    dmaCopy( oamClearSpritesDisabled, oamShadow, sizeof( oamClearSpritesDisabled ) );
}

void spriteInit( void ) {
    spriteClearAll( );
    dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );
}

IWRAM_CODE void spriteUpdate( IWorldRenderer* renderer, Micropolis* sim ) {
    int spriteNo = 0;

    assert( renderer != nullptr );
    assert( sim != nullptr );

    spriteClearAll( );

    for ( Sprite spr : getGameSprites( renderer, sim ) )
        oamShadow[ spriteNo++ ] = spr;

    dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );
}
