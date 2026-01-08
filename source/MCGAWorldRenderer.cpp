#include <gba_video.h>
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

typedef struct MCGATileCacheEntry_S {
    struct MCGATileCacheEntry_S** simTilePtr;

    uint32_t lastSeenTick;
    uint16_t hwTileId;
    uint16_t padding;
} MCGATileCacheEntry;

static const int tilesWide = SCREEN_WIDTH / 8;
static const int tilesTall = SCREEN_HEIGHT / 8;

static const int maxScrollX = ( WORLD_W * 8 ) - SCREEN_WIDTH;
static const int maxScrollY = ( WORLD_H * 8 ) - SCREEN_HEIGHT;

static const uint16_t* tileMap = ( const uint16_t* ) mcga_map_bin;

IWRAM_DATA volatile uint16_t MCGAWorldRenderer::mapShadow[ 32 * 32 ];

void MCGAWorldRenderer::init( Micropolis* _sim ) {
    assert( _sim != nullptr );
    sim = _sim;

    dmaCopy( mcga_tiles_bin, CHAR_BASE_ADR( 0 ), mcga_tiles_bin_size );
    dmaCopy( mcga_palette_bin, BG_PALETTE, mcga_palette_bin_size );

    REG_DISPCNT |= BG0_ON;
    REG_BG0CNT = BG_SIZE_0 | BG_256_COLOR | CHAR_BASE( 0 ) | MAP_BASE( 29 ) | BG_PRIORITY( 2 );

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    scrollX = 0;
    scrollY = 0;

    scrollXTile = 0;
    scrollYTile = 0;
}

void MCGAWorldRenderer::deinit( void ) {
}

void MCGAWorldRenderer::update( void ) {
    volatile uint16_t* row = nullptr;
    MCGATileCacheEntry* ptr = nullptr;
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
