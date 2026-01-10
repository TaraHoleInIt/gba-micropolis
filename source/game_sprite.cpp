#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_dma.h>
#include <string.h>
#include <assert.h>

#include "game_sprite.h"

int GameSprite::init( int startTileId, int paletteIndex, int _spriteSize, int _spriteShape, const void* spriteImage, int spriteImageLen, const void* spritePalette, int spritePaletteLen, int frames ) {
    void* vramPtr = SPR_VRAM( startTileId );

    assert( startTileId <= 1023 );

    switch ( _spriteSize ) {
        case Sprite_8x8:
            spriteTileCount = 1;
            break;
        case Sprite_16x16:
            spriteTileCount = 4;
            break;
        case Sprite_16x32:
            spriteTileCount = 8;
            break;
        case Sprite_32x32:
            spriteTileCount = 16;
            break;
        case Sprite_32x64:
            spriteTileCount = 32;
            break;
        case Sprite_64x64:
            spriteTileCount = 64;
            break;
        default:
            assert( 0 == 1 );
            break;
    };

    dmaCopy( spriteImage, vramPtr, spriteImageLen );
    dmaCopy( spritePalette, &SPRITE_PALETTE[ paletteIndex * 16 ], spritePaletteLen );

    spriteTileIndex = startTileId;
    spritePaletteIndex = paletteIndex;
    frameCount = frames;
    spriteSize = _spriteSize;
    spriteShape = _spriteShape;

    return ( spriteImageLen / 32 );
}

Sprite GameSprite::getSprite( int x, int y, int frame ) {
    Sprite res;

    memset( &res, 0, sizeof( res ) );

    res.X = x;
    res.Y = y;
    res.Character = spriteTileIndex + ( frame * spriteTileCount );
    res.Size = spriteSize;
    res.Palette = spritePaletteIndex;
    res.ColorMode = OBJ_16_COLOR;
    res.Shape = spriteShape;

    return res;
}
