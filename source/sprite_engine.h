#ifndef _SPRITE_ENGINE_H_
#define _SPRITE_ENGINE_H_

#include <gba_video.h>
#include <gba_sprites.h>

#include "w_micropolis.h"

typedef struct {
    uint16_t tileIndex;
    uint16_t reserved;
    uint32_t lastSeenTick;
} SpriteSlot;

typedef struct SpriteHandle_S {
    Sprite* oamShadowEntry;
    uint32_t lastSeenTick;
    uint16_t tileIndex;
    uint16_t isShown;
} SpriteHandle;

typedef struct {
    int tileBase;;
    const void* spriteFramePtr;

    uint32_t lastSeenTick;
} SpriteVRAMSlot;

void spriteEngineUpdate( Micropolis& sim );
void spriteEngineInit( void );

#endif
