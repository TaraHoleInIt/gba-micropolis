#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>
#include <vector>

#include <gba_sprites.h>

void spriteInit( void );
void spriteUpdate( IWorldRenderer* renderer, Micropolis* sim );

#endif
