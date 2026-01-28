#ifndef _IWORLDRENDERER_H_
#define _IWORLDRENDERER_H_

#include <gba_sprites.h>
#include <vector>

#include "w_micropolis.h"

class IWorldRenderer {
public:
    virtual void init( void )               = 0;
    virtual void deinit( void )             = 0;
    virtual void update( unsigned short* simMap[ WORLD_W ] ) = 0;

    virtual void getViewport( int& left, int& right, int& top, int& bottom ) = 0;

    virtual void scroll( int dx, int dy )   = 0;
    virtual void scrollTo( int x, int y )   = 0;
};

#endif
