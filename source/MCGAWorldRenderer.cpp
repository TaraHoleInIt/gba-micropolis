#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "w_micropolis.h"

#include "timer.h"
#include "input.h"

#include "text_and_debug.h"

#include "MCGAWorldRenderer.h"

#include "mcga_palette_bin.h"
#include "mcga_tiles_bin.h"
#include "mcga_map_bin.h"

// Sprites
#include "train_mcga_img_bin.h"
#include "train_mcga_pal_bin.h"

#include "heli_mcga_img_bin.h"
#include "heli_mcga_pal_bin.h"

#include "plane_mcga_img_bin.h"
#include "plane_mcga_pal_bin.h"

#include "boat_mcga_img_bin.h"
#include "boat_mcga_pal_bin.h"

#include "tornado_mcga_img_bin.h"
#include "tornado_mcga_pal_bin.h"

#include "monster_mcga_img_bin.h"
#include "monster_mcga_pal_bin.h"

#include "explosion_mcga_img_bin.h"
#include "explosion_mcga_pal_bin.h"

static const int tilesWide = SCREEN_WIDTH / 8;
static const int tilesTall = SCREEN_HEIGHT / 8;

static const int maxScrollX = ( WORLD_W * 8 ) - SCREEN_WIDTH;
static const int maxScrollY = ( WORLD_H * 8 ) - SCREEN_HEIGHT;

static const uint16_t* tileMap = ( const uint16_t* ) mcga_map_bin;

IWRAM_DATA volatile uint16_t MCGAWorldRenderer::mapShadow[ 32 * 32 ];

void MCGAWorldRenderer::init( Micropolis* _sim ) {
    int tid = 1;

    assert( _sim != nullptr );
    sim = _sim;

    dmaCopy( mcga_tiles_bin, CHAR_BASE_ADR( 0 ), mcga_tiles_bin_size );
    dmaCopy( mcga_palette_bin, BG_PALETTE, mcga_palette_bin_size );

    memset( SPR_VRAM( 0 ), 0, 8 * 8 / 2 );

    REG_DISPCNT |= BG0_ON | OBJ_1D_MAP | OBJ_ENABLE;
    REG_BG0CNT = BG_SIZE_0 | BG_256_COLOR | CHAR_BASE( 0 ) | MAP_BASE( 29 ) | BG_PRIORITY( 2 );

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    scrollX = 0;
    scrollY = 0;

    scrollXTile = 0;
    scrollYTile = 0;

    tid+= trainSprite.init( tid, 0, Sprite_16x16, SQUARE, train_mcga_img_bin, train_mcga_img_bin_size, train_mcga_pal_bin, train_mcga_pal_bin_size, 4 );
    tid+= heliSprite.init( tid, 1, Sprite_32x32, SQUARE, heli_mcga_img_bin, heli_mcga_img_bin_size, heli_mcga_pal_bin, heli_mcga_pal_bin_size, 8 );
    tid+= planeSprite.init( tid, 2, Sprite_32x32, SQUARE, plane_mcga_img_bin, plane_mcga_img_bin_size, plane_mcga_pal_bin, plane_mcga_pal_bin_size, 11 );
    tid+= boatSprite.init( tid, 3, Sprite_32x32, SQUARE, boat_mcga_img_bin, boat_mcga_img_bin_size, boat_mcga_pal_bin, boat_mcga_pal_bin_size, 8 );
    tid+= tornadoSprite.init( tid, 4, Sprite_32x64, TALL, tornado_mcga_img_bin, tornado_mcga_img_bin_size, tornado_mcga_pal_bin, tornado_mcga_pal_bin_size, 3 );
    tid+= monsterSprite.init( tid, 5, Sprite_32x32, SQUARE, monster_mcga_img_bin, monster_mcga_img_bin_size, monster_mcga_pal_bin, monster_mcga_pal_bin_size, 16 );
    tid+= explosionSprite.init( tid, 6, Sprite_32x32, SQUARE, explosion_mcga_img_bin, explosion_mcga_img_bin_size, explosion_mcga_pal_bin, explosion_mcga_pal_bin_size, 5 );
}

void MCGAWorldRenderer::deinit( void ) {
}

void MCGAWorldRenderer::update( void ) {
    volatile uint16_t* row = nullptr;
    int timeNow = 0;
    int x = 0;
    int y = 0;
    int i = 0;
    int sx = 0;
    int sy = 0;

    timeNow = timerMillis( );

    sx = scrollXTile;
    sy = scrollYTile;

    for ( y = 0; y < 32; y++ ) {
        row = ( volatile uint16_t* ) &mapShadow[ y * 32 ];

        for ( x = 0; x < 32; x++ )
            *row++ = tileMap[ sim->map[ x + sx ][ y + sy ] & 0x03FF ];
    }

    dmaCopy( ( void* ) mapShadow, MAP_BASE_ADR( 29 ), sizeof( mapShadow ) );

    REG_BG0HOFS = scrollX & 0x07;
    REG_BG0VOFS = scrollY & 0x07;
}

void MCGAWorldRenderer::getViewport( int& left, int& right, int& top, int& bottom ) {
    left = scrollX;
    right = left + SCREEN_WIDTH;

    top = scrollY;
    bottom = top + SCREEN_HEIGHT;
}

void MCGAWorldRenderer::scroll( int dx, int dy ) {
    scrollX+= dx;
    scrollY+= dy;

    scrollX = ( scrollX < 0 ) ? 0 : scrollX;
    scrollY = ( scrollY < 0 ) ? 0 : scrollY;

    scrollX = ( scrollX >= maxScrollX ) ? maxScrollX - 1 : scrollX;
    scrollY = ( scrollY >= maxScrollY ) ? maxScrollY - 1 : scrollY;

    scrollXTile = scrollX / 8;
    scrollYTile = scrollY / 8;
}

std::vector< Sprite > MCGAWorldRenderer::getSprites( void ) {
    std::vector< Sprite > sprites;
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    int sprX0 = 0;
    int sprY0 = 0;
    int sprX1 = 0;
    int sprY1 = 0;

    getViewport( left, right, top, bottom );
    sprites.clear( );

    for ( SimSprite* s = sim->spriteList; s != nullptr; s = s->next ) {
        sprX0 = ( ( s->x + s->xOffset ) / 2 ) - left;
        sprY0 = ( ( s->y + s->yOffset ) / 2 ) - top;

        sprX1 = sprX0 + ( s->width / 2 );
        sprY1 = sprY0 + ( s->height / 2 );

        if ( sprX1 >= 0 && sprX0 < SCREEN_WIDTH && sprY1 >= 0 && sprY0 < SCREEN_HEIGHT && s->frame > 0 ) {
            switch ( s->type ) {
                case SPRITE_TRAIN:
                    sprites.push_back( trainSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_HELICOPTER:
                    sprites.push_back( heliSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_AIRPLANE:
                    sprites.push_back( planeSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_SHIP:
                    sprites.push_back( boatSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_MONSTER:
                    sprites.push_back( monsterSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_TORNADO:
                    sprites.push_back( tornadoSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                case SPRITE_EXPLOSION:
                    sprites.push_back( explosionSprite.getSprite( sprX0, sprY0, s->frame - 1 ) );
                    break;
                default:
                    break;
            };
        }
    }

    return sprites;
}
