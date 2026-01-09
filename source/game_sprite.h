#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>

#include <gba_sprites.h>

class GameSprite {
private:
    uint16_t spriteTileIndex;
    uint16_t spritePaletteIndex;
    uint16_t spriteTileCount;

    uint8_t spriteSize;
    uint8_t spriteShape;
    uint8_t frameCount;

public:
    int init( int startTileId, int palletIndex, int _spriteSize, int spriteShape, const void* spriteImage, int spriteImageLen, const void* spritePalette, int spritePaletteLen, int frames );

    Sprite getSprite( int x, int y, int frame );
};

#endif
