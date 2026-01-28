#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "w_micropolis.h"

#include "vram_queue.h"
#include "timer.h"

#include "text_and_debug.h"

#include "TandyWorldRenderer.h"
#include "tandy_palette_bin.h"
#include "tandy_tiles_bin.h"
#include "tandy_map_bin.h"

#include "tandysheet_img_bin.h"
#include "tandysheet_pal_bin.h"

static const int tilesWide = SCREEN_WIDTH / 8;
static const int tilesTall = SCREEN_HEIGHT / 8;

static const int maxScrollX = ( WORLD_W * 8 ) - SCREEN_WIDTH;
static const int maxScrollY = ( WORLD_H * 8 ) - SCREEN_HEIGHT;

static const uint16_t* tileMap = ( const uint16_t* ) tandy_map_bin;

IWRAM_DATA volatile uint16_t TandyWorldRenderer::mapShadow[ 32 * 32 ];

void TandyWorldRenderer::init( void) {
    dmaCopy( tandy_palette_bin, BG_PALETTE, tandy_palette_bin_size );
    dmaCopy( tandy_tiles_bin, CHAR_BASE_ADR( 0 ), tandy_tiles_bin_size );

    memset( ( void* ) mapShadow, 0, sizeof( mapShadow ) );

    REG_DISPCNT |= BG0_ON | OBJ_ENABLE;
    REG_BG0CNT = BG_SIZE_0 | BG_16_COLOR | CHAR_BASE( 0 ) | MAP_BASE( 29 ) | BG_PRIORITY( 2 );

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    scrollX = 0;
    scrollY = 0;

    scrollXTile = 0;
    scrollYTile = 0;

    dmaCopy( tandysheet_img_bin, SPR_VRAM( 0 ), tandysheet_img_bin_size );
    dmaCopy( tandysheet_pal_bin, SPRITE_PALETTE, tandysheet_pal_bin_size );
}

void TandyWorldRenderer::deinit( void ) {
}

IWRAM_CODE void TandyWorldRenderer::update( unsigned short* simMap[ WORLD_W ] ) {
    volatile uint16_t* row = nullptr;
    int x = 0;
    int y = 0;
    int sx = 0;
    int sy = 0;

    sx = scrollXTile;
    sy = scrollYTile;

    for ( y = 0; y < tilesTall + 1; y++ ) {
        row = ( volatile uint16_t* ) &mapShadow[ y * 32 ];

        for ( x = 0; x < tilesWide + 1; x++ )
            *row++ = tileMap[ simMap[ x + sx ][ y + sy ] & 0x03FF ];
    }

    dmaCopy( ( void* ) mapShadow, MAP_BASE_ADR( 29 ), sizeof( mapShadow ) );

    REG_BG0HOFS = scrollX & 0x07;
    REG_BG0VOFS = scrollY & 0x07;
}

IWRAM_CODE void TandyWorldRenderer::getViewport( int& left, int& right, int& top, int& bottom ) {
    left = scrollX;
    right = left + SCREEN_WIDTH;

    top = scrollY;
    bottom = top + SCREEN_HEIGHT;
}

IWRAM_CODE void TandyWorldRenderer::scroll( int dx, int dy ) {
    scrollX+= dx;
    scrollY+= dy;

    scrollX = ( scrollX < 0 ) ? 0 : scrollX;
    scrollY = ( scrollY < 0 ) ? 0 : scrollY;

    scrollX = ( scrollX >= maxScrollX ) ? maxScrollX - 1 : scrollX;
    scrollY = ( scrollY >= maxScrollY ) ? maxScrollY - 1 : scrollY;

    scrollXTile = scrollX / 8;
    scrollYTile = scrollY / 8;
}

IWRAM_CODE void TandyWorldRenderer::scrollTo( int x, int y ) {
    scrollX = x;
    scrollY = y;

    scrollX = ( scrollX < 0 ) ? 0 : scrollX;
    scrollY = ( scrollY < 0 ) ? 0 : scrollY;

    scrollX = ( scrollX >= maxScrollX ) ? maxScrollX - 1 : scrollX;
    scrollY = ( scrollY >= maxScrollY ) ? maxScrollY - 1 : scrollY;

    scrollXTile = scrollX / 8;
    scrollYTile = scrollY / 8;
}
