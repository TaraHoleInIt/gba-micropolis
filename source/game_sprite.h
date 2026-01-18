#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>
#include <vector>

#include <gba_sprites.h>
#include "w_micropolis.h"
#include "IWorldRenderer.h"

void spriteInit( void );
void spriteUpdate( IWorldRenderer* renderer, Micropolis* sim );

constexpr Sprite makeSprite16( uint16_t tileId, uint16_t size, uint16_t shape, uint16_t palette, uint16_t hFlip, uint16_t vFlip ) {
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

Sprite setupSprite( const Sprite base, int x, int y );
Sprite setupSprite( const Sprite base, int x, int y, int palette );

#endif
