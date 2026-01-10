#ifndef _TANDYWORLDRENDERER_H_
#define _TANDYWORLDRENDERER_H_

#include "IWorldRenderer.h"

class TandyWorldRenderer : public IWorldRenderer {
private:
    static volatile uint16_t mapShadow[ 32 * 32 ];
    Micropolis* sim;

    volatile int scrollX;
    volatile int scrollY;

    volatile int scrollXTile;
    volatile int scrollYTile;

public:
    void init( Micropolis* _sim );
    void deinit( void );
    void update( void );

    void getViewport( int& left, int& right, int& top, int& bottom );

    void scroll( int dx, int dy );

    std::vector< Sprite > getSprites( void );
};

#endif
