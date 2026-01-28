#ifndef _GAME_H_
#define _GAME_H_

#include <vector>

#include "Window.h"
#include "IWorldRenderer.h"
#include "w_micropolis.h"
#include "cursor.h"
#include "MCGAWorldRenderer.h"
#include "TandyWorldRenderer.h"

class Game : public Window {
private:
    std::vector< Window* > windows;

    Micropolis sim;
    IWorldRenderer* renderer;
    Cursor cursor;

    MCGAWorldRenderer rendererMCGA;
    TandyWorldRenderer rendererTandy;

    uint32_t nextSimTick;
    uint32_t nextTileAnimateTick;
    uint32_t nextScrollTick;

    bool needsRedraw;
    bool gameReady;
    bool gameRunning;

    void setRenderer( IWorldRenderer* newRenderer );

    static Sprite oamShadow[ 128 ];

    std::vector< Sprite > getGameSprites( void );
    void spriteInit( void );
    void spriteClearAll( void );
    void spriteUpdate( void );

    void handleScrolling( int keysHeld );

public:
    Game( void );

    void tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp );
    void runFrame( void );

    void vblank( void );

    void show( void ) override;
    void hide( void ) override;

    bool wmPopLastWindow( void );
    void wmShowWindow( Window* win );

    Window* wmGetActiveWindow( void );

    friend class Cursor;
};

extern Game game;

#endif
