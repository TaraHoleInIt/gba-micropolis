#ifndef _GAME_H_
#define _GAME_H_

#include <vector>

#include "Window.h"
#include "IWorldRenderer.h"
#include "w_micropolis.h"
#include "cursor.h"
#include "MCGAWorldRenderer.h"
#include "TandyWorldRenderer.h"
#include "text_and_debug.h"
#include "text.h"

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
    uint32_t nextStatusBarTick;
    uint32_t eraseMessageTick;

    bool needsRedraw;
    bool gameReady;
    bool gameRunning;
    bool statusBarShown;

    void setRenderer( IWorldRenderer* newRenderer );

    static Sprite oamShadow[ 128 ];

    std::vector< Sprite > getGameSprites( void );
    void spriteInit( void );
    void spriteClearAll( void );
    void spriteUpdate( void );

    void handleScrolling( int keysHeld );

    void drawStatusBars( void );
    void clearStatusBars( void );

    static const char* messageTable[ MESSAGE_LAST + 1 ];
    static const char* monthNames[ 12 ];

    static void simCallback( Micropolis* sim, void* data, const char* name, const char* params, va_list args );

    char gameMessage[ TextConsoleWidth + 1 ];

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

    void showMessage( int messageId );
    void showMessage( const char* message );
    void clearMessage( void );

    void scrollTo( int x, int y );

    Micropolis& getSim( void ) { return sim; }

    void setTandyRenderer( void );
    void setMCGARenderer( void );

    friend class Cursor;
};

extern Game game;

#endif
