#ifndef _TANDYWORLDRENDERER_H_
#define _TANDYWORLDRENDERER_H_

#include "IWorldRenderer.h"
#include "game_sprite.h"

class TandyWorldRenderer : public IWorldRenderer {
private:
    Micropolis* sim;

    volatile int scrollX;
    volatile int scrollY;

    volatile int scrollXTile;
    volatile int scrollYTile;

public:
    void init( void );
    void deinit( void );
    void update( unsigned short* simMap[ WORLD_W ] );

    void getViewport( int& left, int& right, int& top, int& bottom );

    void scroll( int dx, int dy );
    void scrollTo( int x, int y );
};

#endif
