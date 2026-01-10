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

    GameSprite trainSprite;
    GameSprite heliSprite;
    GameSprite planeSprite;
    GameSprite boatSprite;
    GameSprite tornadoSprite;
    GameSprite monsterSprite;
    GameSprite explosionSprite;

public:
    void init( Micropolis* _sim );
    void deinit( void );
    void update( void );

    void getViewport( int& left, int& right, int& top, int& bottom );

    void scroll( int dx, int dy );

    std::vector< Sprite > getSprites( void );
};

#endif
