#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_dma.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "timer.h"
#include "text_and_debug.h"
#include "sprite_engine.h"
#include "tile_engine.h"
#include "config.h"

#include "sprites_pal_bin.h"

#include "obj1_img_bin.h"
#include "obj2_img_bin.h"
#include "obj3_img_bin.h"
#include "obj4_img_bin.h"
#include "obj5_img_bin.h"
#include "obj6_img_bin.h"
#include "obj7_img_bin.h"
#include "obj8_img_bin.h"

#define ArraySize( a ) ( ( int ) ( sizeof( a ) / sizeof( a[ 0 ] ) ) )

#define SpriteFrameSize ( 64 * 64 / 2 )
#define SpriteVRAMSize 32768
#define SpriteEngineVRAMSize ( SpriteVRAMSize / 2 )
#define SpriteEngineStartTile 512
#define SpriteEngineSpriteTiles ( 8 * 8 )
#define SpriteEngineSpriteIndex( n ) ( ( SpriteEngineStartTile ) + ( ( n ) * SpriteEngineSpriteTiles ) )

static IWRAM_CODE void copySpriteData( void* dest, const void* spriteBase, int frame );
static IWRAM_CODE SpriteVRAMSlot* spriteCacheLookup( const void* spriteFramePtr );
static IWRAM_CODE SpriteVRAMSlot* spriteCacheAllocate( void );
static IWRAM_CODE void spriteCacheEvictOld( void );
static IWRAM_CODE void spriteCacheAssign( SpriteVRAMSlot* slot, const void* spriteData, int frame );
static const void* spriteFramePtr( int objNo, int frame );
static IWRAM_CODE SpriteVRAMSlot* spriteCacheEvictOldest( void );

static IWRAM_DATA const void* spriteObjs[ SPRITE_COUNT ] = {
    nullptr,
    obj1_img_bin,
    obj2_img_bin,
    obj3_img_bin,
    obj4_img_bin,
    obj5_img_bin,
    obj6_img_bin,
    obj7_img_bin,
    obj8_img_bin
};

static IWRAM_DATA SpriteVRAMSlot spriteCache[ ] = {
    { SpriteEngineSpriteIndex( 0 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 1 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 2 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 3 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 4 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 5 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 6 ), nullptr, 0 },
    { SpriteEngineSpriteIndex( 7 ), nullptr, 0 }
};

static IWRAM_DATA Sprite oamShadow[ 128 ];

static inline __attribute__( ( always_inline ) ) const void* spriteFramePtr( int objNo, int frame ) {
    assert( objNo > 0 );

    return ( const void* ) ( ( ( const uint8_t* ) spriteObjs[ objNo ] ) + ( frame * SpriteFrameSize ) );
}

static IWRAM_CODE void copySpriteData( void* dest, const void* spriteBase, int frame ) {
    spriteBase = ( const void* ) ( ( ( uint8_t* ) spriteBase ) + ( frame * ( 64 * 64 / 2 ) ) );
    dmaCopy( spriteBase, dest, SpriteFrameSize );
}

static IWRAM_CODE SpriteVRAMSlot* spriteCacheLookup( const void* spriteFramePtr ) {
    for ( int i = 0; i < ArraySize( spriteCache ); i++ ) {
        if ( spriteCache[ i ].spriteFramePtr == spriteFramePtr )
            return &spriteCache[ i ];
    }

    // Sprite not cached
    return nullptr;
}

static IWRAM_CODE SpriteVRAMSlot* spriteCacheAllocate( void ) {
    for ( int i = 0; i < ArraySize( spriteCache ); i++ ) {
        if ( spriteCache[ i ].spriteFramePtr == nullptr )
            return &spriteCache[ i ];
    }

    // No free sprite slots
    return nullptr;
}

static IWRAM_CODE void spriteCacheEvictOld( void ) {
    uint32_t tickNow = timerMillis( );
    uint32_t timeDiff = 0;
    
    for ( int i = 0; i < ArraySize( spriteCache ); i++ ) {
        if ( spriteCache[ i ].lastSeenTick > 0 ) {
            timeDiff = tickNow - spriteCache[ i ].lastSeenTick;

            // Mark sprite slot as available if we haven't seen it in a while
            if ( timeDiff >= 500 ) {
                spriteCache[ i ].spriteFramePtr = nullptr;
                spriteCache[ i ].lastSeenTick = 0;
            }
        }
    }
}

static IWRAM_CODE SpriteVRAMSlot* spriteCacheEvictOldest( void ) {
    uint32_t tickNow = timerMillis( );
    uint32_t timeDiff = 0;
    uint32_t biggestDiff = 0;
    int biggestIndex = -1;;
    
    for ( int i = 0; i < ArraySize( spriteCache ); i++ ) {
        if ( spriteCache->lastSeenTick > 0 ) {
            timeDiff = tickNow - spriteCache[ i ].lastSeenTick;

            if ( timeDiff > biggestDiff ) {
                biggestDiff = timeDiff;
                biggestIndex = i;
            }
        }
    }

    assert( biggestIndex >= 0 );

    spriteCache[ biggestIndex ].spriteFramePtr = nullptr;
    spriteCache[ biggestIndex ].lastSeenTick = 0;

    return &spriteCache[ biggestIndex ];
}

static IWRAM_CODE void spriteCacheAssign( SpriteVRAMSlot* slot, const void* spriteData, int frame ) {
    if ( slot != nullptr ) {
        slot->lastSeenTick = timerMillis( );
        slot->spriteFramePtr = spriteData;

        copySpriteData( SPR_VRAM( slot->tileBase ), spriteData, frame );
    }
}

IWRAM_CODE void spriteEngineUpdate( Micropolis& sim ) {
    int viewportLeft = 0;
    int viewportRight = 0;
    int viewportTop = 0;
    int viewportBottom = 0;
    int sprX0 = 0;
    int sprX1 = 0;
    int sprY0 = 0;
    int sprY1 = 0;
    int frame = 0;
    SpriteHandle* ptr = nullptr;
    SpriteVRAMSlot* slot = nullptr;
    uint32_t tickNow = 0;
    int id = 64;

    tickNow = timerMillis( );
    spriteCacheEvictOld( );

    tileEngineGetViewport( viewportLeft, viewportRight, viewportTop, viewportBottom );
    memset( oamShadow, 0, sizeof( oamShadow ) );

    for ( SimSprite* spr = sim.spriteList; spr != nullptr; spr = spr->next ) {
        if ( spr->frame > 0 ) {
            frame = spr->frame - 1;

            sprX0 = ( spr->x + spr->xOffset ) - viewportLeft;
            sprY0 = ( spr->y + spr->yOffset ) - viewportTop;

            sprX1 = sprX0 + spr->width;
            sprY1 = sprY0 + spr->height;

            // If sprite is even partially visible...
            if ( sprX1 >= 0 && sprX0 < SCREEN_WIDTH && sprY1 >= 0 && sprY0 < SCREEN_HEIGHT ) {
                // Try to find existing sprite in cache
                slot = spriteCacheLookup( spriteFramePtr( spr->type, frame ) );

                // If not in the cache, try allocating a new slot
                if ( slot == nullptr )
                    slot = spriteCacheAllocate( );

                // Uh oh, try to evict the oldest sprite
                if ( slot == nullptr )
                    slot = spriteCacheEvictOldest( );

                // testtesttest
                // If it gets here, then there are bigger problems...
                assert( slot != nullptr );

                // Assign sprite into the cache
                spriteCacheAssign( slot, spriteObjs[ spr->type ], frame );
                slot->lastSeenTick = tickNow;

                oamShadow[ id ].Character = slot->tileBase;
                oamShadow[ id ].X = sprX0;
                oamShadow[ id ].Y = sprY0;
                oamShadow[ id ].ColorMode = OBJ_16_COLOR;
                oamShadow[ id ].Shape = SQUARE;
                oamShadow[ id ].Size = Sprite_64x64;
                
                id++;

                //mgbaPrintf( "Sprite shown\n" );
            }
        }
    }

	dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );
}

void spriteEngineInit( void ) {
    REG_DISPCNT |= OBJ_16_COLOR | OBJ_ENABLE | OBJ_1D_MAP;

    dmaCopy( sprites_pal_bin, SPRITE_PALETTE, sprites_pal_bin_size );
}
