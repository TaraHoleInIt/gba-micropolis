#ifndef _MCGAWORLDRENDERER_H_
#define _MCGAWORLDRENDERER_H_

#include "IWorldRenderer.h"
#include "game_sprite.h"

class MCGAWorldRenderer : public IWorldRenderer {
private:
    static volatile uint16_t mapShadow[ 32 * 32 ];
    Micropolis* sim;

    volatile int scrollX;
    volatile int scrollY;

    volatile int scrollXTile;
    volatile int scrollYTile;

public:
    void init( void );
    void deinit( void );
    void update( unsigned short* simMap[ WORLD_W ] );
    void vblank( void );

    void getViewport( int& left, int& right, int& top, int& bottom );

    void scroll( int dx, int dy );
    void scrollTo( int x, int y );
};

#endif
