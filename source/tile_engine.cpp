#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "micropolis.h"
#include "tile_engine.h"
#include "vram_queue.h"
#include "timer.h"
#include "input.h"
#include "gfx.h"
#include "config.h"
#include "text_and_debug.h"

class MetaTile;

class MetaTile {
private:
    static MetaTile* metaTileTable[ 1024 ];
    static MetaTile tileCache[ 256 ];
    static int hwTileIndex;

    MetaTile** entry;

    void* tileVRAM;

public:
    uint32_t lastSeenTick;
    uint16_t baseTileID;

    MetaTile( void ) {
        assert( ( MetaTile::hwTileIndex + 4 ) <= 1024 );

        baseTileID = MetaTile::hwTileIndex;
        MetaTile::hwTileIndex+= 4;

        tileVRAM = ( ( ( uint8_t* ) CHAR_BASE_BLOCK( 0 ) ) + baseTileID * ( TileSize ) );
        
        lastSeenTick = 0xFFFFFFFF;
        entry = nullptr;
    }

    IWRAM_CODE void associate( int micropolisTileNo, MetaTile** tableEntry ) {
        micropolisTileNo &= 0x03FF;
        entry = tableEntry;

        *tableEntry = this;

        //dmaCopy( &__tiles_bin[ micropolisTileNo * TileSize * 4 ], tileVRAM, TileSize * 4 );
        //addTileUpdateToQueue( &__tiles_bin[ micropolisTileNo * TileSize * 4 ], tileVRAM );
    }

    IWRAM_CODE void disassociate( void ) {
        *entry = nullptr;
        lastSeenTick = 0xFFFFFFFF;
    }

    IWRAM_CODE static MetaTile* findFreeTile( void ) {
        uint32_t tickNow = timerMillis( );
        uint32_t ticksSince = 0;
        uint32_t oldestTime = 0;
        int oldestIndex = -1;
        MetaTile* oldest = nullptr;
        MetaTile* res = nullptr;

        for ( int i = 0; i < 256; i++ ) {
            // Tile is free (just created)
            if ( tileCache[ i ].lastSeenTick == 0xFFFFFFFF )
                return &tileCache[ i ];
            
            ticksSince = tickNow - tileCache[ i ].lastSeenTick;

            if ( ticksSince > oldestTime ) {
                oldestTime = ticksSince;
                oldestIndex = i;
            }
        }

        assert( oldestIndex >= 0 );

        tileCache[ oldestIndex ].disassociate( );

        return &tileCache[ oldestIndex ];
    }

    IWRAM_CODE static void metaTilePlace( int micropolisTileID, int x, int y ) {
        volatile uint16_t* mapUpper = ( volatile uint16_t* ) SCREEN_BASE_BLOCK( 31 );
        volatile uint16_t* mapLower = ( volatile uint16_t* ) SCREEN_BASE_BLOCK( 31 );
        uint32_t tileWordUpper = 0;
        uint32_t tileWordLower = 0;
        MetaTile* mt = nullptr;

        // If tile is not in the cache
        if ( metaTileTable[ micropolisTileID ] == nullptr ) {
            mt = MetaTile::findFreeTile( );
            assert( mt != nullptr );

            mt->associate( micropolisTileID, &metaTileTable[ micropolisTileID ] );
        }

        assert( metaTileTable[ micropolisTileID ] != nullptr );

        mapUpper+= ( x + ( y * 32 ) );
        mapLower+= ( x + ( ( y + 1 ) * 32 ) );

        tileWordUpper = ( metaTileTable[ micropolisTileID ]->baseTileID + 1 ) << 16;
        tileWordUpper |= ( metaTileTable[ micropolisTileID ]->baseTileID + 0 );

        tileWordLower = ( metaTileTable[ micropolisTileID ]->baseTileID + 3 ) << 16;
        tileWordLower |= ( metaTileTable[ micropolisTileID ]->baseTileID + 2 );

        *( ( uint32_t* ) mapUpper ) = tileWordUpper;
        *( ( uint32_t* ) mapLower ) = tileWordLower;

        metaTileTable[ micropolisTileID ]->lastSeenTick = timerMillis( );
    }
};

int MetaTile::hwTileIndex = 0;

IWRAM_DATA MetaTile* MetaTile::metaTileTable[ 1024 ];
IWRAM_DATA MetaTile MetaTile::tileCache[ 256 ];

static volatile int scrollTileX = 0;
static volatile int scrollTileY = 0;
static volatile int scrollX = 0;
static volatile int scrollY = 0;

static volatile int tileEngineReady = 0;

void tileEngineInit( void ) {
	REG_DISPCNT |= GameBG_Enable;
	BGCTRL[ 0 ] = BG_16_COLOR | BG_SIZE_0 | BG_MAP_BASE( 31 ) | BG_TILE_BASE( 0 ) | BG_PRIORITY( 3 );

	dmaCopy( __tiles_pal_bin, BG_PALETTE, __tiles_pal_bin_len );

    // Make a copy of the palette and invert it (used for highlighting tiles)
    for ( int i = 0; i < __tiles_pal_bin_len / 2; i++ )
        BG_PALETTE[ 16 + i ] = ~BG_PALETTE[ i ];

    tileEngineReady = 1;
}

void tileEngineVBlank( void ) {
    int dx = 0;
    int dy = 0;

    if ( ! tileEngineReady )
        return;

    if ( inputIsHeld( KEY_RIGHT ) )
        dx = 1;

    if ( inputIsHeld( KEY_LEFT ) )
        dx = -1;

    if ( inputIsHeld( KEY_DOWN ) )
        dy = 1;

    if ( inputIsHeld( KEY_UP ) )
        dy = -1;

    if ( inputIsHeld( KEY_R ) ) {
        dx*= 2;
        dy*= 2;
    }

    scrollX+= dx;
    scrollY+= dy;

    scrollX = scrollX < 0 ? 0 : scrollX;
    scrollY = scrollY < 0 ? 0 : scrollY;

    scrollX = ( scrollX >= MaxScrollX ) ? MaxScrollX - 1 : scrollX;
    scrollY = ( scrollY >= MaxScrollY ) ? MaxScrollY - 1 : scrollY;

    scrollTileX = scrollX / 16;
    scrollTileY = scrollY / 16;

    REG_BG0HOFS = scrollX & 0x0F;
    REG_BG0VOFS = scrollY & 0x0F;
}

void tileEngineUpdate( Micropolis& sim ) {
    uint32_t a = 0;
    uint32_t b = 0;
    int x = 0;
    int y = 0;
    int tileId = 0;

    if ( ! tileEngineReady )
        return;

    for ( y = 0; y < 11; y++ ) {
        for ( x = 0; x < 16; x++ ) {
            tileId = sim.map[ x + scrollTileX ][ y + scrollTileY ] & 0x03FF;

            MetaTile::metaTilePlace( tileId, x * 2, y * 2 );
        }
    }
}

void tileEngineGetViewport( int& left, int& right, int& top, int& bottom ) {
    left = scrollX;
    right = scrollX + SCREEN_WIDTH;

    top = scrollY;
    bottom = scrollY + SCREEN_HEIGHT;
}
