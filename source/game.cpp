#include <gba.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_video.h>
#include <gba_dma.h>
#include "text_and_debug.h"
#include "Game.h"
#include "timer.h"

#include "TextWindow.h"
#include "DialogWindow.h"

#define MosaicBGSizeH( size ) ( ( size ) & 0x0F )
#define MosaicBGSizeV( size ) ( ( ( size ) & 0x0F ) << 4 )
#define MosaicOBJSizeH( size ) ( ( ( size ) & 0x0F ) << 8 )
#define MosaicOBJSizeV( size ) ( ( ( size ) & 0x0F ) << 12 )

#define Config_SimTick_Time 100
#define Config_AnimationTick_Time 200
#define Config_ScrollTick_Time 100

IWRAM_DATA Sprite Game::oamShadow[ 128 ];
Game* game;

Game::Game( void ) {
    nextTileAnimateTick = 0;
    nextSimTick = 0;
    nextScrollTick = 0;

    gameReady = false;
    needsRedraw = true;

    sim.generateSomeCity( 0x10203040 );
    sim.setSpeed( 1 );
    sim.setPasses( 1 );
    sim.simTick( );

    setRenderer( &rendererTandy );
    spriteInit( );

    gameRunning = true;
    gameReady = true;

    windows.push_back( this );
}

void Game::tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp ) {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    if ( gameRunning ) {
        if ( tickNow >= nextScrollTick ) {
            handleScrolling( keysHeld );
            nextScrollTick = tickNow + Config_ScrollTick_Time;
        }

        if ( tickNow >= nextSimTick ) {
            nextSimTick = tickNow + Config_SimTick_Time;
            sim.simTick( );

            needsRedraw = true;
        }

        if ( tickNow >= nextTileAnimateTick ) {
            nextTileAnimateTick = tickNow + Config_AnimationTick_Time;

            renderer->getViewport( left, right, top, bottom );

            left>>= 3;
            right>>= 3;
            top>>= 3;
            bottom >>= 3;

            sim.animateTiles( left, top, left + ( SCREEN_WIDTH / 8 ), top + ( SCREEN_HEIGHT / 8 ) );
            needsRedraw = true;
        }
    }
}

void Game::vblank( void ) {
    if ( gameReady ) {
        if ( needsRedraw ) {
            renderer->update( sim.map );
            spriteUpdate( );
        }
    }
}

void Game::setRenderer( IWorldRenderer* newRenderer ) {
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;

	// Wait until in vblank
	while ( REG_VCOUNT >= SCREEN_HEIGHT );
	while ( REG_VCOUNT < SCREEN_HEIGHT );

	irqDisable( IRQ_VBLANK );
		if ( renderer != nullptr ) {
			// Save scrolling parameters
			renderer->getViewport( left, right, top, bottom );
			renderer->deinit( );
		}

		renderer = newRenderer;
		
		REG_DISPCNT |= LCDC_OFF;
			renderer->init( );
			renderer->scrollTo( left, top );
			textAndDebugInit( );
		REG_DISPCNT &= ~LCDC_OFF;
	irqEnable( IRQ_VBLANK );
}

IWRAM_CODE std::vector< Sprite > Game::getGameSprites( void ) {
    std::vector< Sprite > res;
    Sprite newSprite;
    int left = 0;
    int top = 0;
    int bottom = 0;
    int right = 0;
    int sprX0 = 0;
    int sprY0 = 0;
    int sprX1 = 0;
    int sprY1 = 0;

    res.reserve( 128 );
    renderer->getViewport( left, right, top, bottom );
    
    for ( SimSprite* s = sim.spriteList; s != nullptr; s = s->next ) {
        sprX0 = ( ( s->x + s->xOffset ) / 2 ) - left;
        sprY0 = ( ( s->y + s->yOffset ) / 2 ) - top;

        sprX1 = sprX0 + ( s->width / 2 );
        sprY1 = sprY0 + ( s->height / 2 );

        if ( sprX1 >= 0 && sprX0 < SCREEN_WIDTH && sprY1 >= 0 && sprY0 < SCREEN_HEIGHT && s->frame > 0 ) {
            memcpy( &newSprite, &oamClearSpritesDisabled[ 0 ], sizeof( Sprite ) );

            switch ( s->type ) {
                case SPRITE_TRAIN:
                    newSprite = setupSprite( trainSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_HELICOPTER:
                    newSprite = setupSprite( heliSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_AIRPLANE:
                    newSprite = setupSprite( planeSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_SHIP:
                    newSprite = setupSprite( shipSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_MONSTER:
                    newSprite = setupSprite( monsterSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_TORNADO:
                    newSprite = setupSprite( tornadoSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_EXPLOSION:
                    newSprite = setupSprite( explosionSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                default:
                    break;
            };

            res.push_back( newSprite );
        }
    }

    return res;
}

void Game::spriteInit( void ) {
    spriteClearAll( );
}

void Game::spriteClearAll( void ) {
    dmaCopy( oamClearSpritesDisabled, oamShadow, sizeof( oamClearSpritesDisabled ) );
}

IWRAM_CODE void Game::spriteUpdate( void ) {
    int spriteNo = 0;

    spriteClearAll( );

    for ( Sprite spr : cursor.getSprites( ) )
        oamShadow[ spriteNo++ ] = spr;

    for ( Sprite spr : getGameSprites( ) )
        oamShadow[ spriteNo++ ] = spr;

    dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );
}

void Game::handleScrolling( int keysHeld ) {
    int cursorX = 0;
    int cursorY = 0;
    int scrollX = 0;
    int scrollY = 0;
    int dx = 0;
    int dy = 0;

    dx = ( keysHeld & KEY_LEFT ) ? -1 : 0;
    dx = ( keysHeld & KEY_RIGHT ) ? 1 : dx;

    dy = ( keysHeld & KEY_UP ) ? -1 : 0;
    dy = ( keysHeld & KEY_DOWN ) ? 1 : dy;

    if ( dx != 0 || dy != 0 ) {
		cursorX = ( cursor.getX( ) * 8 ) + 4;
		cursorY = ( cursor.getY( ) * 8 ) + 4;

		scrollX = cursorX - ( SCREEN_WIDTH / 2 );
		scrollY = cursorY - ( SCREEN_HEIGHT / 2 );

        cursor.moveBy( dx, dy );
        renderer->scrollTo( scrollX, scrollY );

        needsRedraw = true;
    }
}

void Game::show( void ) {
    REG_BG0CNT &= ~BG_MOSAIC;
    REG_MOSAIC = 0;

    REG_BLDCNT = 0;
    REG_BLDY = 0;

    gameRunning = true;
}

void Game::hide( void ) {
    REG_MOSAIC = MosaicBGSizeH( 1 ) | MosaicBGSizeV( 1 ) | MosaicOBJSizeH( 1 ) | MosaicOBJSizeV( 1 );
    REG_BG0CNT |= BG_MOSAIC;

    REG_BLDCNT = 1 | ( 1 << 8 ) | ( 3 << 6 );
    REG_BLDY = 8;

    gameRunning = false;
}

void Game::runFrame( void ) {
    Window* activeWin = nullptr;
    int down = 0;
    int held = 0;
    int up = 0;

    scanKeys( );

    down = keysDown( );
    held = keysHeld( );
    up = keysUp( );

    if ( ( down & KEY_START ) && ! ( ( held | down ) & KEY_SELECT ) ) {
        if ( wmGetActiveWindow( ) == this ) {
            wmShowWindow(
                makeMenu(
                    1,
                    1,
                    20,
                    10,
                    Color_White,
                    Color_Blue,
                    "Disasters",
                    { 
                        { "Earthquake", [ ] { game->sim.makeEarthquake( ); } },
                        { "Fire", [ ] { game->sim.makeFire( ); } },
                        { "Fire bombs", [ ] { game->sim.makeFireBombs( ); } },
                        { "Flood", [ ] { game->sim.makeFlood( ); } },
                        { "Monster", [ ] { game->sim.makeMonster( ); } },
                        { "Tornado", [ ] { game->sim.makeTornado( ); } },
                        { "Meltdown", [ ] { game->sim.makeMeltdown( ); } }
                    }
                )
            );
        }
    }

    if ( down & KEY_L )
        cursor.prevTool( );

    if ( down & KEY_R )
        cursor.nextTool( );

    if ( held & KEY_A )
        cursor.doTool( );

    if ( down & KEY_B )
        wmPopLastWindow( );

    activeWin = wmGetActiveWindow( );

    activeWin->tick( timerMillis( ), down, held, up );
}

bool Game::wmPopLastWindow( void ) {
    Window* win = nullptr;
    bool result = false;

    if ( windows.size( ) > 1UL ) {
        win = windows.back( );

        assert( win != nullptr );
        assert( win != this );

        win->hide( );
        delete win;

        windows.pop_back( );
        windows.back( )->show( );

        result = true;
    }

    return result;
}

void Game::wmShowWindow( Window* win ) {
    assert( win != nullptr );

    if ( windows.size( ) >= 1UL )
        windows.back( )->hide( );

    windows.push_back( win );
    win->show( );
}

Window* Game::wmGetActiveWindow( void ) {
    assert( windows.size( ) >= 1UL );

    return windows.back( );
}
